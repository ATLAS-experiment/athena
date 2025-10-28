/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MsTrackFindingAlg.h"

#include "Acts/Surfaces/PerigeeSurface.hpp"

#include "MuonTrackFindingTools/MsTrackSeeder.h"
#include "MuonPatternHelpers/MatrixUtils.h"
#include "ActsCalibBase/CalibrationContext.h"
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"

#include "GaudiKernel/PhysicalConstants.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "MuonTrackEvent/TrackingHelpers.h"

#include "ActsInterop/UnitConverters.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "TruthUtils/AtlasPID.h"

using namespace Acts::UnitLiterals;

namespace MuonR4{
    StatusCode MsTrackFindingAlg::initialize() {
        ATH_CHECK(m_segmentKey.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_segSelector.retrieve());
        ATH_CHECK(m_msTrkSeedKey.initialize());

        ATH_CHECK(m_visualizationTool.retrieve(EnableTool{!m_visualizationTool.empty()}));

        ATH_CHECK(m_trackingGeometryTool.retrieve());
        ATH_CHECK(m_extrapolationTool.retrieve());
        ATH_CHECK(m_trackFitTool.retrieve());
        ATH_CHECK(m_calibTool.retrieve());
        ATH_CHECK(m_writeKey.initialize());


        MsTrackSeeder::Config seederCfg{};
        seederCfg.seedHalfLength = m_seedHalfLength;
        seederCfg.selector = m_segSelector.get();
        seederCfg.detMgr = m_detMgr;

        m_seeder = std::make_unique<MsTrackSeeder>(name(), std::move(seederCfg));
        return StatusCode::SUCCESS;
    }

    MsTrackFindingAlg::~MsTrackFindingAlg() = default;

    StatusCode MsTrackFindingAlg::execute(const EventContext& ctx) const {
        ATH_MSG_VERBOSE("Run track finding in event "<<ctx.eventID().event_number());
        
        const xAOD::MuonSegmentContainer* allEventSegs{nullptr};
        ATH_CHECK(SG::get(allEventSegs, m_segmentKey, ctx));

        auto seedContainer = findTrackSeeds(ctx, *allEventSegs);

        const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
        const Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
        const Acts::CalibrationContext calContext{ActsTrk::getCalibrationContext(ctx)};
        
        
        Acts::VectorTrackContainer trackBackend{};
        Acts::VectorMultiTrajectory trackStateBackend{};
        ActsTrk::MutableTrackContainer cacheTrkContainer{std::move(trackBackend), 
                                                         std::move(trackStateBackend)};


        for (const MsTrackSeed& seed :*seedContainer) {
            fitSeedCandidate(tgContext, mfContext, calContext, seed, cacheTrkContainer);
        }

         
        SG::WriteHandle writeHandleSeed{m_msTrkSeedKey, ctx};
        ATH_CHECK(writeHandleSeed.record(std::move(seedContainer)));
    
        // Constant declination
        Acts::ConstVectorTrackContainer ctrackBackend{std::move(cacheTrkContainer.container())};
        Acts::ConstVectorMultiTrajectory ctrackStateBackend{std::move(cacheTrkContainer.trackStateContainer())};
        auto ctc = std::make_unique<ActsTrk::TrackContainer>(std::move(ctrackBackend),
                                                             std::move(ctrackStateBackend));
  
        SG::WriteHandle writeHandle{m_writeKey, ctx};
        ATH_CHECK(writeHandle.record(std::move(ctc)));
        return StatusCode::SUCCESS;
    }

    std::unique_ptr<MsTrackSeedContainer>  
        MsTrackFindingAlg::findTrackSeeds(const EventContext& ctx,
                                          const xAOD::MuonSegmentContainer& segments) const {

        auto seedContainer = m_seeder->findTrackSeeds(ctx, m_trackingGeometryTool->getGeometryContext(ctx), segments);

        if (!m_visualizationTool.empty()) {
            m_visualizationTool->displaySeeds(ctx, *m_seeder, segments, *seedContainer, "all seeds");
        }
        return seedContainer;
    }
    void MsTrackFindingAlg::fitSeedCandidate(const Acts::GeometryContext& tgContext,
                                             const Acts::MagneticFieldContext& mfContext,
                                             const Acts::CalibrationContext& calContext,
                                             const MsTrackSeed& seed,
                                              ActsTrk::MutableTrackContainer& outContainer) const {
        const EventContext& ctx{*calContext.get<const EventContext*>()};
        ATH_MSG_DEBUG("Attempt to fit a new track seed \n"<<seed);
        std::vector<const xAOD::UncalibratedMeasurement*> measurements{};
        const auto assocDetailedSegs = seed.detailedSegments();
        unsigned int nMeas = std::accumulate(assocDetailedSegs.begin(),
                                             assocDetailedSegs.end(), 0, 
                                             [](const unsigned n, const Segment* segment) {
                                                return n + segment->measurements().size();
                                            });
        measurements.reserve(nMeas);
        const xAOD::MuonSegment* refSeg{nullptr};
        for (const xAOD::MuonSegment* segment : seed.segments()) {
            m_calibTool->stampSignsOnMeasurements(*segment);
            auto segMeasurements = MuonR4::collectMeasurements(*segment, /*skipOutlier:*/ true);
            if (msgLvl(MSG::VERBOSE)) {
                std::stringstream sstr{};
                for (const xAOD::UncalibratedMeasurement* m : segMeasurements) {
                    const auto& surf{xAOD::muonSurface(m)};
                    sstr<<" ***  "<<m_idHelperSvc->toString(xAOD::identify(m))<<"  "<<
                          surf.geometryId()<<" @ "<<Amg::toString(surf.transform(tgContext))<<std::endl;
                }
                ATH_MSG_VERBOSE("Fetch measurements from segment: "<<Amg::toString(segment->position())
                         <<", direction: "<<Amg::toString(segment->direction())<<"\n"<<sstr.str());
            }
            measurements.insert(measurements.end(), 
                                std::make_move_iterator(segMeasurements.begin()),
                                std::make_move_iterator(segMeasurements.end()));

            if (!refSeg && m_segSelector->passSeedingQuality(ctx, *detailedSegment(*segment))) {
                refSeg = segment;
            }
        }
        /// The first segment on the seed is the inner most one
        const Segment* innerSeg{detailedSegment(*refSeg)};
        /// Create a surface which is shortly before the first measurement
        const double propDistance = (xAOD::muonSurface(measurements[0]).center(tgContext) - 
                                         innerSeg->position()).dot(innerSeg->direction()) - 1.*Gaudi::Units::cm;
        const Amg::Vector3D refPos  = innerSeg->position() + propDistance * innerSeg->direction();
        auto target = Acts::Surface::makeShared<Acts::PerigeeSurface>(refPos);
        
        auto fourPos = ActsTrk::convertPosToActs(refPos, refPos.mag() / Gaudi::Units::c_light);
        const double qOverP = 1./ m_seeder->estimateQtimesP(*tgContext.get<const ActsTrk::GeometryContext*>(),
                                                            *mfContext.get<const AtlasFieldCacheCondObj*>(), seed);
        auto initialPars = Acts::BoundTrackParameters::create(tgContext, target, fourPos, 
                                                              innerSeg->direction(),
                                                              ActsTrk::energyToActs(qOverP),
                                                              Acts::BoundSquareMatrix::Identity(), 
                                                              Acts::ParticleHypothesis::muon());
        if (!initialPars.ok()) {
            ATH_MSG_WARNING("Initial estimate of the parameters failed");
            return;
        }
        auto fitTraject = m_trackFitTool->fit(measurements, *initialPars, 
                                              tgContext, mfContext, calContext, target.get());
        if (!fitTraject || fitTraject->size() == 0) {
            return;
        }
        outContainer.ensureDynamicColumns(*fitTraject);
        auto destProxy = outContainer.getTrack(outContainer.addTrack());
        destProxy.copyFrom(fitTraject->getTrack(0));
        ATH_MSG_DEBUG("Good track fit...");
        for (const auto state : destProxy.trackStates()) {
            if (!state.hasUncalibratedSourceLink()){
                continue;
            }
            auto meas = ActsTrk::detail::xAODUncalibMeasCalibrator::unpack(state.getUncalibratedSourceLink());
            ATH_MSG_DEBUG("Accepted measurement "<<m_idHelperSvc->toString(xAOD::identify(meas))
                              <<", "<<xAOD::muonSurface(meas).geometryId()); 
        }
    }

}