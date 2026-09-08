/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MsTrackFindingAlg.h"

#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/detail/PlanarHelper.hpp"


#include "MuonReadoutGeometryR4/MuonDetectorDefs.h"
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"
#include "xAODMuonPrepData/UtilFunctions.h"

#include "GaudiKernel/PhysicalConstants.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonSpacePoint/SpacePointHelpers.h"

#include "MuonTrackEvent/HitSummary.h"

#include "ActsInterop/UnitConverters.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "TruthUtils/HepMCHelpers.h"
#include "MuonVisualizationHelpersR4/ObjVisualizationHelpers.h"

#include <system_error>

using namespace Acts::UnitLiterals;
using namespace Acts::PlanarHelper;

namespace MuonR4{
    StatusCode MsTrackFindingAlg::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_msTrkSeedKey.initialize(SG::AllowEmpty));

        ATH_CHECK(m_visualizationTool.retrieve(EnableTool{!m_visualizationTool.empty()}));
        ATH_CHECK(m_trackFitTool.retrieve());
        ATH_CHECK(m_calibTool.retrieve());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_summaryTool.retrieve());
        ATH_CHECK(m_seedingTool.retrieve());
        ATH_CHECK(m_extrapolationTool.retrieve(EnableTool{m_expressAtMsEntrance}));
        ATH_CHECK(m_trackingGeometrySvc.retrieve());

        ATH_CHECK(m_ctxProvider.initialize());
        return StatusCode::SUCCESS;
    }

    MsTrackFindingAlg::~MsTrackFindingAlg() = default;

    StatusCode MsTrackFindingAlg::execute(const EventContext& ctx) const {
        ATH_MSG_VERBOSE("Run track finding in event "<<ctx.eventID().event_number());
        

        auto seedContainer = std::make_unique<MsTrackSeedContainer>();

        ATH_CHECK(m_seedingTool->findTrackSeeds(ctx, *seedContainer));

        if (!m_visualizationTool.empty()) {
            m_visualizationTool->displaySeeds(ctx, *seedContainer);
        }
        const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);
        const Acts::MagneticFieldContext mfContext = m_ctxProvider.getMagneticFieldContext(ctx);
        const Acts::CalibrationContext calContext{m_ctxProvider.getCalibrationContext(ctx)};
        
        
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
        if (!m_msTrkSeedKey.empty()) {
            SG::WriteHandle writeHandleSeed{m_msTrkSeedKey, ctx};
            ATH_CHECK(writeHandleSeed.record(std::move(seedContainer)));
        }
    
        // Constant declination
        Acts::ConstVectorTrackContainer ctrackBackend{std::move(cacheTrkContainer.container())};
        Acts::ConstVectorMultiTrajectory ctrackStateBackend{std::move(cacheTrkContainer.trackStateContainer())};
        auto ctc = std::make_unique<ActsTrk::TrackContainer>(std::move(ctrackBackend),
                                                             std::move(ctrackStateBackend));
  
        SG::WriteHandle writeHandle{m_writeKey, ctx};
        ATH_CHECK(writeHandle.record(std::move(ctc)));
        return StatusCode::SUCCESS;
    }

    std::pair<MsTrackFindingAlg::OptBoundPars_t, 
          MsTrackFindingAlg::MeasVec_t>
        MsTrackFindingAlg::prepareFit(const Acts::GeometryContext& tgContext,
                                      const Acts::CalibrationContext& calContext,
                                      const MsTrackSeed& seed) const {
        const EventContext& ctx{*calContext.get<const EventContext*>()};        
        
        auto initialPars = m_seedingTool->estimateStartParameters(ctx, seed);
        if (!initialPars.ok()) {          
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__
                            <<" - Start parameter creation failed. ");
            return std::make_pair(OptBoundPars_t::failure(std::make_error_code(std::errc::invalid_argument)),
                                  std::vector<const xAOD::UncalibratedMeasurement_v1*>{});
        }
        
        MeasVec_t measurements{};
        measurements.reserve(100);
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
                        <<", "<<surf.geometryId()<<" @ "<<Amg::toString(surf.localToGlobalTransform(tgContext))<<std::endl;
                }
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Fetch measurements from segment: "<<Amg::toString(segment->position())
                         <<", direction: "<<Amg::toString(segment->direction()) << " eta " << segment->direction().eta() << " phi " << segment->direction().phi() <<"\n"<<sstr.str());
            }
            measurements.insert(measurements.end(), 
                                std::make_move_iterator(segMeasurements.begin()),
                                std::make_move_iterator(segMeasurements.end()));
        }

        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - "<<measurements.size()<<" measurements");
        if (measurements.empty()) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - No measurements were associated with seed "<<seed);
            return std::make_pair(OptBoundPars_t::failure(std::make_error_code(std::errc::invalid_argument)),
                                  std::vector<const xAOD::UncalibratedMeasurement_v1*>{});
        }
        return std::make_pair(std::move(initialPars),  std::move(measurements));

    }
    bool MsTrackFindingAlg::fitSeedCandidate(const Acts::GeometryContext& tgContext,
                                             const Acts::MagneticFieldContext& mfContext,
                                             const Acts::CalibrationContext& calContext,
                                             const MsTrackSeed& seed,
                                             ActsTrk::MutableTrackContainer& outContainer) const {
        
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Attempt to fit a new track seed \n"<<seed);
        const EventContext& ctx{*calContext.get<const EventContext*>()};
        const auto [initialPars, measurements] = prepareFit(tgContext, calContext, seed);
        
        if (!initialPars.ok()) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Failed to construct valid parameters for seed \n"<<seed);
            if (m_visualizationTool.isEnabled()) {
                m_visualizationTool->displayTrackSeedObj(ctx, seed, initialPars, "FailedStartPars");
            }
            return false;
        }
        auto fitTraject = m_trackFitTool->fit(measurements, *initialPars, 
                                              tgContext, mfContext, calContext, 
                                              &(*initialPars).referenceSurface());
        if (!fitTraject || fitTraject->size() == 0) {
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Fit failed. Seed was \n"<<seed);
            if (m_visualizationTool.isEnabled()) {
                m_visualizationTool->displayTrackSeedObj(ctx, seed, initialPars, "FailedFit");
            }
            return false;
        }
 
        ActsTrk::MutableTrackContainer::TrackProxy track = fitTraject->getTrack(0);
        // Check the hit counts on track post fit and if we only have one station on track discard track
        // Eventually we should implement some recovery mechanism for track where we loose too many stations
        MuonR4::HitSummary summary = m_summaryTool->makeSummary(ctx, track);
        ATH_MSG_DEBUG("Track has " << static_cast<std::uint32_t>(summary.nPrecisionStations()) << " precision layers with summary "<< summary);
        if(summary.nPrecisionStations()<2) {
            ATH_MSG_DEBUG("rejecting single station track");
            return false;
        }
        if (!expressAtCaloExit(ctx, track)) {

            return false;
        }
        
        /** Add the links to the segments making up this track as an extra
         *  column. Use the indices of the segment objects which can later
         *  be transformed into a full ElementLink as there is only one
         *  SegmentContainer from which the seeds are built */
        {
            fitTraject->addColumn<std::vector<const xAOD::MuonSegment*>>("muonSegLinks");
            auto appendMe = seed.segments();
            auto& toAppend = track.component<std::vector<const xAOD::MuonSegment*>>("muonSegLinks");
            toAppend.insert(toAppend.end(), appendMe.begin(), appendMe.end());
        }
        outContainer.ensureDynamicColumns(*fitTraject);
        auto destProxy = outContainer.getTrack(outContainer.addTrack());
        destProxy.copyFrom(track);
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Good track fit...");
        if (m_visualizationTool.isEnabled()) {
            m_visualizationTool->displayTrackSeedObj(ctx, seed, 
                destProxy.createParametersAtReference(), "GoodFit");
        }
        return true;
    }
    bool MsTrackFindingAlg::expressAtCaloExit(const EventContext& ctx,
                                              ActsTrk::MutableTrackContainer::TrackProxy track) const {
        if (!m_expressAtMsEntrance) {
            return true;
        }
        const Acts::BoundTrackParameters startPars = track.createParametersAtReference();
        const Acts::TrackingVolume* msEntrance = m_trackingGeometrySvc->getEnvelope(ActsTrk::SystemEnvelope::CaloExit);
        const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Extrapolate "<<startPars<<", "
            <<startPars.referenceSurface().toString(tgContext)<<" to MS entrance: "<<msEntrance->volumeBounds());
       
        auto parsAtEntrance = m_extrapolationTool->propagate(ctx, startPars, *msEntrance, 
                                                             ActsTrk::IExtrapolationTool::VolumeAbort::atEntrance,
                                                             Acts::Direction::Backward());
        
        if (!parsAtEntrance.ok()) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Failed to extrapolate "<<startPars<<" to MS entrance");
            return m_ignoreFailedMsEntrance;
        }
        if (parsAtEntrance->referenceSurface().geometryId().withBoundary(0) != msEntrance->geometryId()) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Parameter extrapolation to "<<(*parsAtEntrance)<<", @"
                            << Amg::toString(parsAtEntrance->referenceSurface().localToGlobalTransform(tgContext))
                            <<" did not end up at "<<msEntrance->geometryId()<<".");
            return m_ignoreFailedMsEntrance;
        }
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Extrapolated start parameters to "<<(*parsAtEntrance)
                      <<", "<<parsAtEntrance->referenceSurface().toString(tgContext));
        track.setReferenceSurface(parsAtEntrance->referenceSurface().getSharedPtr());
        track.parameters() = parsAtEntrance->parameters();
        track.covariance() = (*parsAtEntrance->covariance());
        return true;
    }
    
}
