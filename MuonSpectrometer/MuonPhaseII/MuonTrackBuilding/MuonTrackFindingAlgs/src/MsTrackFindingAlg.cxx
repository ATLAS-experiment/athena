/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MsTrackFindingAlg.h"

#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/detail/PlanarHelper.hpp"


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
#include "MuonVisualizationHelpersR4/ObjVisualizationHelpers.h"

using namespace Acts::UnitLiterals;
using namespace Acts::PlanarHelper;

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
        /// Attach the number of the parent seed to the output container
        cacheTrkContainer.addColumn<std::size_t>("parentSeed");
        unsigned seedIdx{0};
        for (const MsTrackSeed& seed : *seedContainer) {
            if (!fitSeedCandidate(tgContext, mfContext, calContext, seed, 
                                  cacheTrkContainer)) {
                ++seedIdx;
                continue;
            }
            auto lastTrack = cacheTrkContainer.getTrack(cacheTrkContainer.size() -1);
            lastTrack.component<std::size_t, Acts::hashString("parentSeed")>() = seedIdx;
            ++seedIdx;
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
std::pair<MsTrackFindingAlg::OptBoundPars_t, 
          MsTrackFindingAlg::MeasVec_t>
        MsTrackFindingAlg::prepareFit(const Acts::GeometryContext& tgContext,
                                      const Acts::MagneticFieldContext& mfContext,
                                      const Acts::CalibrationContext& calContext,
                                      const MsTrackSeed& seed) const {
        const EventContext& ctx{*calContext.get<const EventContext*>()};        
        MeasVec_t measurements{};
        measurements.reserve(100);
        ///Fetch the measurements from the segment & find the first segment which has phi hits
        const xAOD::MuonSegment* refSeg{nullptr};
        for (const xAOD::MuonSegment* segment : seed.segments()) {
            /// Ensure that the drift signs from the fit are stamped onto the Uncalibrated measurements
            m_calibTool->stampSignsOnMeasurements(*segment);
            MeasVec_t segMeasurements = collectMeasurements(*segment, /*skipOutlier:*/ true);
            if (msgLvl(MSG::VERBOSE)) {
                std::stringstream sstr{};
                for (const xAOD::UncalibratedMeasurement* m : segMeasurements) {
                    const Acts::Surface& surf{xAOD::muonSurface(m)};
                    sstr<<" ***  "<<m_idHelperSvc->toString(xAOD::identify(m))
                        <<", "<<m->numDimensions()<<", "
                        <<", "<<surf.geometryId()<<" @ "<<Amg::toString(surf.transform(tgContext))<<std::endl;
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
        Amg::Vector3D seedPos{refSeg->position()};
        Amg::Vector3D seedDir{refSeg->direction()};
        /// The middle or outer segment provide the phi information. Not so easy becasue we want to
        /// Take the y0 & precision direction from the inner segment but the phi & x0 from a straight
        /// line extrapolation onto the plane
        if (false && refSeg != seed.segments().front()) {
            const MuonGMR4::SpectrometerSector* innerPlane = m_seeder->envelope(*seed.segments().front());
            const Acts::PlaneSurface& surf = innerPlane->surface();
            const Amg::Transform3D toInnerPlane = surf.transform(tgContext).inverse();
            const Amg::Vector3D locSeedPos = toInnerPlane * seedPos;
            const Amg::Vector3D locSeedDir = toInnerPlane.linear() * seedDir;

            auto seedOnInner = Acts::PlanarHelper::intersectPlane(locSeedPos, locSeedDir,
                                                                  Amg::Vector3D::UnitZ(), 0.);

            using enum SegmentFit::ParamDefs;
            SegmentFit::Parameters innerPars = SegmentFit::localSegmentPars(*seed.segments().front());
            innerPars[Acts::toUnderlying(x0)] = seedOnInner.position().x();
            const Amg::Vector3D innerSegDir = 
                    Acts::makeDirectionFromPhiTheta(innerPars[Acts::toUnderlying(phi)],
                                                    innerPars[Acts::toUnderlying(theta)]);
            const Amg::Vector3D combSegDir = 
                    Acts::makeDirectionFromAxisTangents(houghTanAlpha(locSeedDir),
                                                        houghTanBeta(innerSegDir));
            seedPos = surf.transform(tgContext) * Amg::Vector3D{innerPars[Acts::toUnderlying(x0)],
                                                                innerPars[Acts::toUnderlying(y0)], 0};
            seedDir = surf.transform(tgContext).linear() * combSegDir;
        }
        /// Create a surface which is shortly before the first measurement
        const double propDistance = (xAOD::muonSurface(measurements[0]).center(tgContext) - 
                                         seedPos).dot(seedDir) - 1.*Gaudi::Units::cm;
        const Amg::Vector3D refPos  = seedPos + propDistance * seedDir;
        auto target = Acts::Surface::makeShared<Acts::PerigeeSurface>(refPos);
        
        auto fourPos = ActsTrk::convertPosToActs(refPos, refPos.mag() / Gaudi::Units::c_light);
        const double qOverP = 1./ m_seeder->estimateQtimesP(*tgContext.get<const ActsTrk::GeometryContext*>(),
                                                            *mfContext.get<const AtlasFieldCacheCondObj*>(), seed);
        auto initialPars = Acts::BoundTrackParameters::create(tgContext, target, fourPos, 
                                                              seedDir,
                                                              ActsTrk::energyToActs(qOverP),
                                                              Acts::BoundSquareMatrix::Identity(), 
                                                              Acts::ParticleHypothesis::muon());
        return std::make_pair(std::move(initialPars),  std::move(measurements));

    }
    void MsTrackFindingAlg::visualizeObj(const Acts::GeometryContext& tgContext,
                                         const Acts::CalibrationContext& calContext,
                                         const MsTrackSeed& seed,
                                         const OptBoundPars_t& parsToExt) const {
        if (!m_drawEvent) {
            return;
        }
        const EventContext& ctx {*calContext.get<const EventContext*>()};
        const ActsTrk::GeometryContext& gctx{*tgContext.get<const ActsTrk::GeometryContext*>()};
        Acts::ObjVisualization3D visualHelper{};
        if (parsToExt.ok()) {
            MuonValR4::drawPropagation(m_extrapolationTool->propagationSteps(ctx, *parsToExt).first,
                                       visualHelper);
        }
        std::string saveStr = std::format("MsTrackFinding_{:}", ctx.eventID().event_number());
        for (const xAOD::MuonSegment* seg : seed.segments()) {
            MuonValR4::drawSegmentMeasurements(gctx,* seg, visualHelper);
            MuonValR4::drawSegmentLine(gctx,*seg, visualHelper);
            saveStr += std::format("_{:}_{:}", printID(*seg), seg->index());
        }
        saveStr+=".obj";
        visualHelper.write(saveStr);
    }


    bool MsTrackFindingAlg::fitSeedCandidate(const Acts::GeometryContext& tgContext,
                                             const Acts::MagneticFieldContext& mfContext,
                                             const Acts::CalibrationContext& calContext,
                                             const MsTrackSeed& seed,
                                             ActsTrk::MutableTrackContainer& outContainer) const {
        
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Attempt to fit a new track seed \n"<<seed);

        const auto [initialPars, measurements] = prepareFit(tgContext, mfContext, calContext, seed);
        
        if (!initialPars.ok()) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Failed to construct valid parameters for seed \n"<<seed);
            visualizeObj(tgContext, calContext, seed, initialPars);
            return false;
        }
        auto fitTraject = m_trackFitTool->fit(measurements, *initialPars, 
                                              tgContext, mfContext, calContext, 
                                              &(*initialPars).referenceSurface());
        if (!fitTraject || fitTraject->size() == 0) {
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Fit failed ");
            visualizeObj(tgContext, calContext, seed, initialPars);
            return false;
        }
        outContainer.ensureDynamicColumns(*fitTraject);
        auto destProxy = outContainer.getTrack(outContainer.addTrack());
        destProxy.copyFrom(fitTraject->getTrack(0));
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Good track fit...");
        for (const auto state : destProxy.trackStates()) {
            if (!state.hasUncalibratedSourceLink()){
                continue;
            }
            auto meas = ActsTrk::detail::xAODUncalibMeasCalibrator::unpack(state.getUncalibratedSourceLink());
            ATH_MSG_DEBUG("Accepted measurement "<<m_idHelperSvc->toString(xAOD::identify(meas))
                              <<", "<<xAOD::muonSurface(meas).geometryId()); 
        }
        return true;
    }
    
}