/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MSTrackFindingAlg.h"

#include "Acts/Surfaces/PerigeeSurface.hpp"

#include "AthContainers/ConstDataVector.h"
#include "MuonTrackFindingTools/MsTrackSeeder.h"
#include "ActsCalibBase/CalibrationContext.h"
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"

#include "GaudiKernel/PhysicalConstants.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "TruthUtils/AtlasPID.h"

namespace MuonR4{
    StatusCode MSTrackFindingAlg::initialize() {
        ATH_CHECK(m_segmentKeys.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_segSelector.retrieve());
        ATH_CHECK(m_msTrkSeedKey.initialize());

        ATH_CHECK(m_visualizationTool.retrieve(EnableTool{!m_visualizationTool.empty()}));

        ATH_CHECK(m_trackingGeometryTool.retrieve());
        ATH_CHECK(m_extrapolationTool.retrieve());
        ATH_CHECK(m_trackFitTool.retrieve());
        ATH_CHECK(m_trackContKeys.initialize(m_writePrefix));
        return StatusCode::SUCCESS;
    }

    MSTrackFindingAlg::~MSTrackFindingAlg() = default;

    StatusCode MSTrackFindingAlg::execute(const EventContext& ctx) const {
        ATH_MSG_VERBOSE("Run track finding in event "<<ctx.eventID().event_number());
        
        ConstDataVector<xAOD::MuonSegmentContainer> allEventSegs{SG::VIEW_ELEMENTS};
        for (const SG::ReadHandleKey<xAOD::MuonSegmentContainer>& key : m_segmentKeys) {
            const xAOD::MuonSegmentContainer* partSegments{nullptr};
            ATH_CHECK(SG::get(partSegments, key, ctx));
            allEventSegs.insert(allEventSegs.end(), partSegments->begin(), partSegments->end());
        }
        auto seedContainer = findTrackSeeds(ctx, *allEventSegs.asDataVector());

        
        const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
        const Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
        const Acts::CalibrationContext calContext{ActsTrk::getCalibrationContext(ctx)};
        
        
        Acts::VectorTrackContainer trackBackend;
        Acts::VectorMultiTrajectory trackStateBackend;
        ActsTrk::MutableTrackContainer cacheTrkContainer{
                                     std::move(trackBackend),
                                     std::move(trackStateBackend)};


        for (const MsTrackSeed& seed :*seedContainer) {
            fitSeedCandidate(tgContext, mfContext, calContext, seed, cacheTrkContainer);
        }

        SG::WriteHandle writeHandle{m_msTrkSeedKey, ctx};
        ATH_CHECK(writeHandle.record(std::move(seedContainer)));
        return StatusCode::SUCCESS;
    }

    std::unique_ptr<MsTrackSeedContainer>  
        MSTrackFindingAlg::findTrackSeeds(const EventContext& ctx,
                                          const xAOD::MuonSegmentContainer& segments) const {

        MsTrackSeeder::Config seederCfg{};
        seederCfg.seedHalfLength = m_seedHalfLength;
        seederCfg.selector = m_segSelector.get();

        MsTrackSeeder seeder{name(), std::move(seederCfg)};

        auto seedContainer = seeder.findTrackSeeds(ctx, segments);

        if (!m_visualizationTool.empty()) {
            m_visualizationTool->displaySeeds(ctx, seeder, segments, *seedContainer, "all seeds");
        }
        return seedContainer;
    }
    void MSTrackFindingAlg::fitSeedCandidate(const Acts::GeometryContext& tgContext,
                                             const Acts::MagneticFieldContext& mfContext,
                                             const Acts::CalibrationContext& calContext,
                                             const MsTrackSeed& seed,
                                              ActsTrk::MutableTrackContainer& outContainer) const {
        ATH_MSG_DEBUG("Attempt to fit a new track seed");

        std::vector<const xAOD::UncalibratedMeasurement*> measurements{};
        unsigned int nMeas = 2* std::accumulate(seed.detailedSegments().begin(),
                                                seed.detailedSegments().end(), 0, 
                                                [](const unsigned n, const Segment* segment){
                                                    return n + segment->measurements().size();
                                                });
        measurements.reserve(nMeas);
        
        for (const Segment* segment : seed.detailedSegments()) {
            ATH_MSG_DEBUG("Fetch measurements from segment: "<<Amg::toString(segment->position())
                         <<", direction: "<<Amg::toString(segment->direction()));
            auto segMeasuremnts = MuonR4::collectMeasurements(*segment, /*skipOutlier:*/ true);
            measurements.insert(measurements.end(), 
                                std::make_move_iterator(segMeasuremnts.begin()),
                                std::make_move_iterator(segMeasuremnts.end()));
        }
        /// The first segment on the seed is the inner most one
        const Segment* innerSeg{seed.detailedSegments().front()};
        /// Create a surface which is shortly before the first measurement
        constexpr double propDistance = 5.*Gaudi::Units::cm;
        const Amg::Vector3D refPos  = innerSeg->position() - propDistance * innerSeg->direction();
        auto target = Acts::Surface::makeShared<Acts::PerigeeSurface>(refPos);
        
        Acts::ActsVector<4> fourPos{};
        fourPos.block<3,1>(Acts::ePos0, 0) = refPos;
        fourPos[Acts::eTime] = refPos.mag() / Gaudi::Units::c_light;
        auto initialPars = Acts::BoundTrackParameters::create(tgContext, target, fourPos, 
                                innerSeg->direction(),
                                1. / (10.*Gaudi::Units::GeV),
                                Acts::BoundSquareMatrix::Identity(), Acts::ParticleHypothesis::muon());
        if (!initialPars.ok()) {
            ATH_MSG_WARNING("Initial estimate of the parameters failed");
            return;
        }
        auto fitTraject = m_trackFitTool->fit(measurements, *initialPars, tgContext, mfContext,calContext, target.get());
        if (!fitTraject) {
            return;
        }
        auto destProxy = outContainer.getTrack(outContainer.addTrack());
        destProxy.copyFrom(fitTraject->getTrack(0), true); // make sure we copy track states!
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