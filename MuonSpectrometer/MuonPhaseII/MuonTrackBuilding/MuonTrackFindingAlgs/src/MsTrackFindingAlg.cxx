/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MsTrackFindingAlg.h"

#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
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
        ATH_CHECK(m_pseudoMeasuremntCreator.initialize(m_writeKey.key(), !m_writeKey.empty()));
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
        const Acts::GeometryContext tgContext{m_ctxProvider.getGeometryContext(ctx)};
        const Acts::MagneticFieldContext mfContext{m_ctxProvider.getMagneticFieldContext(ctx)};
        const Acts::CalibrationContext calContext{m_ctxProvider.getCalibrationContext(ctx)};
        
        ActsTrk::MutableTrackContainer cacheTrkContainer{Acts::VectorTrackContainer{},
                                                         Acts::VectorMultiTrajectory{}};
        
        auto auxMeasHandler = m_pseudoMeasuremntCreator.makeHandle(ctx, tgContext);
        ATH_CHECK(auxMeasHandler.ok());
        /// Attach the number of the parent seed to the output container
        cacheTrkContainer.addColumn<std::size_t>("parentSeed");
        unsigned seedIdx{0};
        for (MsTrackSeed& seed : *seedContainer) {
            if (seed.hasOverlap()) {
                ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Overlap marker from a previous fit is triggered.");
                continue;
            }
            if (fitSeedCandidate(tgContext, mfContext, calContext, seed, 
                                  cacheTrkContainer, *auxMeasHandler)) {
                auto lastTrack = cacheTrkContainer.getTrack(cacheTrkContainer.size() -1);
                lastTrack.component<std::size_t, Acts::hashString("parentSeed")>() = seedIdx;
                seed.triggerOverlapMarker();
            }
            ++seedIdx;
        }
        auto [rBegin, rEnd] = std::ranges::remove_if(*seedContainer, &MsTrackSeed::hasOverlap);
        seedContainer->erase(rBegin, rEnd);
        if (!m_msTrkSeedKey.empty()) {
            SG::WriteHandle writeHandleSeed{m_msTrkSeedKey, ctx};
            ATH_CHECK(writeHandleSeed.record(std::move(seedContainer)));
        }
    
        // Constant declination
        auto ctc = std::make_unique<ActsTrk::TrackContainer>(Acts::ConstVectorTrackContainer{std::move(cacheTrkContainer.container())},
                                                             Acts::ConstVectorMultiTrajectory{std::move(cacheTrkContainer.trackStateContainer())});
  
        SG::WriteHandle writeHandle{m_writeKey, ctx};
        ATH_CHECK(writeHandle.record(std::move(ctc)));
        return StatusCode::SUCCESS;
    }

    MsTrackFindingAlg::MeasVec_t 
        MsTrackFindingAlg::collectMeasurements(const Acts::GeometryContext& tgContext,
                                               const Acts::BoundTrackParameters& startPars, 
                                               const MsTrackSeed& seed,
                                               const std::size_t reqStations,
                                               PseudoMeasHandle_t& auxMeasContainer) const {
        MeasVec_t pickedMeas{};
        pickedMeas.reserve(seed.nMeasurements());

        std::uint8_t nPhiMeasurements{0};
        AmgSymMatrix(1) phiCov{Acts::square(1._cm)};
        /// Count the number of sations
        std::vector<Muon::MuonStationIndex::StIndex> stations{};
        stations.reserve(seed.segments().size());
        for (const xAOD::MuonSegment* segment : seed.segments()) {
            /** Check if the station has not yet been processed and also  
             *  check whether there were already enough stations processed */
            const auto stIdx = Muon::MuonStationIndex::toStationIndex(segment->chamberIndex());
            if (!Acts::rangeContainsValue(stations, stIdx)) {
                if (stations.size() == reqStations) {
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Requirement on "<<reqStations<<" stations satisfied.");
                    break;
                }
                stations.push_back(stIdx);
            }
            for (std::size_t m = 0; m < nMeasurements(*segment); ++m) {
                const xAOD::UncalibratedMeasurement* meas = getMeasurement(*segment, m);
                if (meas->type() == xAOD::UncalibMeasType::Other || 
                    isOutlierMeasurement(*segment, m)) {
                    continue;
                }
                pickedMeas.push_back(meas);
                /// Count the phi degree of freedom
                switch (meas->numDimensions()) {
                    case 1: {
                        const auto* muonMeas{dynamic_cast<const xAOD::MuonMeasurement*>(meas)};
                        assert(muonMeas != nullptr);
                        if (muonMeas->measuresPhi() || (muonMeas->type() == xAOD::UncalibMeasType::MMClusterType &&
                                                        m_idHelperSvc->mmIdHelper().isStereo(muonMeas->identify()))) {
                            ++nPhiMeasurements;
                            phiCov(0,0) = std::max(phiCov(0,0), 1.* meas->localCovariance<1>()(0, 0));    
                        }
                        break;
                    }
                    /// Combined or 2D measurements
                    case 0:
                    case 2:
                        ++nPhiMeasurements;
                        phiCov(0,0) = std::max(phiCov(0,0), 1.*meas->localCovariance<2>()(Acts::eBoundLoc1, Acts::eBoundLoc1));
                        break;
                    default:
                        break;
                }
            }
        }

        if (nPhiMeasurements < 2) {
            /** Construct the start position at that surface */
            const Amg::Vector3D startPos = startPars.position(tgContext);
            const Amg::Vector3D startDir = startPars.direction();

            /** Fetch the transform from the start parameter surface */
            const Amg::Transform3D& startTrf{startPars.referenceSurface().
                                             localToGlobalTransform(tgContext)};
            /** Calculate the local position and direction */
            const Amg::Vector3D locDir = startTrf.inverse().linear() * startDir;
            const Amg::Vector3D locPos = startTrf.inverse()* startPos;
            /** Position of the first and last surface */
            const Amg::Vector3D firstM = xAOD::muonSurface(pickedMeas.front()).
                                         intersect(tgContext,  startPos, startDir).closest().position();
            const Amg::Vector3D lastM = xAOD::muonSurface(pickedMeas.back()).
                                        intersect(tgContext,  startPos, startDir).closest().position();
            
            auto bounds = std::make_shared<Acts::RectangleBounds>(10._m, 10. * std::sqrt(phiCov(0,0)));
            /* Place the first one between the first measurement and the start*/
            auto firstSurface = Acts::Surface::makeShared<Acts::PlaneSurface>(startTrf * 
                                    Amg::getTranslate3D(locPos + 0.5*(firstM - startPos).dot(startDir) * locDir), bounds);
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Create new auxilliary surface @ "
                        <<Amg::toString(firstSurface->localToGlobalTransform(tgContext)));
            auto lastSurface = Acts::Surface::makeShared<Acts::PlaneSurface>(startTrf * 
                                    Amg::getTranslate3D(locPos + (8._mm + (lastM - startPos).dot(startDir))* locDir), bounds);
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Create new auxilliary surface @ "
                        <<Amg::toString(lastSurface->localToGlobalTransform(tgContext)));
            constexpr auto calibIdx = ActsTrk::detail::MeasurementCalibratorBase::ProjectorType::e1DimRotNoTime;
            pickedMeas.push_back(auxMeasContainer.newMeasurement<1>(firstSurface, calibIdx, phiCov));
            pickedMeas.push_back(auxMeasContainer.newMeasurement<1>(lastSurface, calibIdx, phiCov));
        }
        if (msgLvl(MSG::VERBOSE)) {
            std::stringstream sstr{};
            for (const xAOD::UncalibratedMeasurement* m : pickedMeas) {
                sstr<<"  --- "<<m_idHelperSvc->toString(xAOD::identify(m))<<", nDim: "<<m->numDimensions()<<", "
                    <<xAOD::muonSurface(m).geometryId()<<std::endl;
            }
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Collected measurements.\n"<<sstr.str());
        }
        return pickedMeas;
    }

    bool MsTrackFindingAlg::fitSeedCandidate(const Acts::GeometryContext& tgContext,
                                             const Acts::MagneticFieldContext& mfContext,
                                             const Acts::CalibrationContext& calContext,
                                             const MsTrackSeed& seed,
                                             ActsTrk::MutableTrackContainer& outContainer,
                                             PseudoMeasHandle_t& auxMeasContainer) const {
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Attempt to fit a new track seed \n"<<seed);
        const EventContext& ctx{*calContext.get<const EventContext*>()};
        const auto initialPars = m_seedingTool->estimateStartParameters(ctx, seed);
        if (!initialPars.ok()) {          
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__
                            <<" - Start parameter creation failed. ");
            if (m_visualizationTool.isEnabled()) {
                m_visualizationTool->displayTrackSeedObj(ctx, seed, initialPars, "FailedStartPars");
            }
            return false;
        }
        MeasVec_t measurements = collectMeasurements(tgContext, *initialPars, seed,
                                                     seed.nStations(), auxMeasContainer);
  
        ActsTrk::MutableTrackContainer fitAttempts{Acts::VectorTrackContainer{}, 
                                                  Acts::VectorMultiTrajectory{}};

        auto copyFit = [&] (std::unique_ptr<ActsTrk::MutableTrackContainer> && result,
                             const std::size_t reqStations) {
            if (!result || result->size() == 0ul) {
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Track fit failed");
                if (m_visualizationTool.isEnabled()) {
                    auto fitStartPars = fitAttempts.size() > 0 ? fitAttempts.getTrack(
                                        fitAttempts.size() -1ul).createParametersAtReference() :
                                        *initialPars;
                   m_visualizationTool->displayTrackSeedObj(ctx, seed, fitStartPars, 
                                                            "FailedFit");
                }
                return false;
            }
            fitAttempts.ensureDynamicColumns(*result);
            ActsTrk::MutableTrackContainer::TrackProxy track = result->getTrack(0);
            
            auto destProxy = fitAttempts.getTrack(fitAttempts.addTrack());
            destProxy.copyFrom(track);
            
            const std::size_t nStations = m_summaryTool->countMuonStations(ctx, track);
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Created new track "<<Amg::toString(track.parameters())<<
                         " with chi2: "<<(track.chi2() / track.nDoF())<<", nDoF: "<<track.nDoF()<<", nStations: "<<nStations
                            <<", requested: "<<reqStations);
            if (m_visualizationTool.isEnabled() && nStations < reqStations) {
               m_visualizationTool->displayTrackSeedObj(ctx, seed, track.createParametersAtReference(), 
               std::format("InsufficentStations_{:}_vs_{:}", nStations, reqStations));
            }
            return nStations >= reqStations;
        };
        /// We first combine two muon
        const Acts::Surface* target = initialPars->referenceSurface().getSharedPtr().get();
        
        if (!copyFit(m_trackFitTool->fit(measurements, *initialPars, 
                     tgContext, mfContext, calContext, target), seed.nStations()) && seed.nStations() > 2) {
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - First shot failed. Attempt iteratively over the stations. ");
            for (std::size_t nStation = 2ul; nStation <= seed.nStations(); ++nStation) {
                const Acts::BoundTrackParameters startPars = fitAttempts.size() > 0ul ?
                        fitAttempts.getTrack(fitAttempts.size() - 1ul).createParametersAtReference() : *initialPars;
                        
                measurements = collectMeasurements(tgContext, startPars, seed, nStation, auxMeasContainer);
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Include "<<nStation<<" stations.");
                auto fitTraject = m_trackFitTool->fit(measurements, startPars, 
                                                      tgContext, mfContext, calContext, target);

                if (!copyFit(std::move(fitTraject), nStation)) {
                    break;
                }
                if (m_visualizationTool.isEnabled()) {
                    m_visualizationTool->displayTrackSeedObj(ctx, seed, 
                        fitAttempts.getTrack(fitAttempts.size() - 1ul).createParametersAtReference(), 
                        std::format("IntermediateGooFit_{:}", nStation));
                }
            }
        }
            
        /// No tracks have succceeded.
        if (fitAttempts.size() == 0ul) {
            return false;
        }
        ActsTrk::MutableTrackContainer::TrackProxy track = fitAttempts.getTrack(fitAttempts.size() - 1);
        // Check the hit counts on track post fit and if we only have one station on track discard track
        // Eventually we should implement some recovery mechanism for track where we loose too many stations
        const std::uint32_t nStations = m_summaryTool->countMuonStations(ctx, track);
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Track has " << nStations << " precision layers.\n"
                      <<m_summaryTool->makeSummary(ctx, track));
        if(nStations<2ul) {
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
            fitAttempts.addColumn<std::vector<const xAOD::MuonSegment*>>("muonSegLinks");
            auto appendMe = seed.segments();
            auto& toAppend = track.component<std::vector<const xAOD::MuonSegment*>>("muonSegLinks");
            toAppend.insert(toAppend.end(), appendMe.begin(), appendMe.end());
        }
        outContainer.ensureDynamicColumns(fitAttempts);
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
