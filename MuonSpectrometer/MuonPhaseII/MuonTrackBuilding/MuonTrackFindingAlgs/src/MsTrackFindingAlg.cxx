/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MsTrackFindingAlg.h"

#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/detail/PlanarHelper.hpp"
#include "Acts/Utilities/detail/CombinatoricIndices.hpp"

#include "MuonReadoutGeometryR4/MuonDetectorDefs.h"
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"
#include "xAODMuonPrepData/UtilFunctions.h"

#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonSpacePoint/SpacePointHelpers.h"

#include "MuonTrackEvent/HitSummary.h"

#include "ActsInterop/UnitConverters.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "TruthUtils/HepMCHelpers.h"
#include "MuonVisualizationHelpersR4/ObjVisualizationHelpers.h"

#include <limits>
#include <system_error>

using namespace Acts::UnitLiterals;
using namespace Acts::PlanarHelper;
using namespace Muon::MuonStationIndex;

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
                ++seedIdx;
            }
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
                                               PseudoMeasHandle_t& auxMeasContainer) const {
        MeasVec_t pickedMeas{};
        pickedMeas.reserve(seed.nMeasurements());

        std::uint8_t nPhiMeasurements{0};
        AmgSymMatrix(1) phiCov{Acts::square(1._cm)};
        /// Count the number of sations
        for (const xAOD::MuonSegment* segment : seed.segments()) {
            m_calibTool->stampSignsOnMeasurements(tgContext, *segment);
            /** Check if the station has not yet been processed and also  
             *  check whether there were already enough stations processed */
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
            const Acts::Transform3& startTrf{startPars.referenceSurface().
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
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Create new auxilliary surface @ "
                        <<Amg::toString(firstSurface->localToGlobalTransform(tgContext)));
            auto lastSurface = Acts::Surface::makeShared<Acts::PlaneSurface>(startTrf * 
                                    Amg::getTranslate3D(locPos + (8._mm + (lastM - startPos).dot(startDir))* locDir), bounds);
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Create new auxilliary surface @ "
                        <<Amg::toString(lastSurface->localToGlobalTransform(tgContext)));
            constexpr auto calibIdx = ActsTrk::detail::MeasurementCalibratorBase::ProjectorType::e1DimRotNoTime;
            pickedMeas.push_back(auxMeasContainer.newMeasurement<1>(firstSurface, calibIdx, phiCov));
            pickedMeas.push_back(auxMeasContainer.newMeasurement<1>(lastSurface, calibIdx, phiCov));
        }
        if (msgLvl(MSG::VERBOSE)) {
            std::stringstream sstr{};
            for (const xAOD::UncalibratedMeasurement* m : pickedMeas) {
                /// Auxiliary measurements are no muon measurements and have no identifier
                if (!xAOD::isMuon(m->type())) {
                    sstr<<"  --- auxiliary measurement, nDim: "<<m->numDimensions()<<std::endl;
                    continue;
                }
                sstr<<"  --- "<<m_idHelperSvc->toString(xAOD::identify(m))<<", nDim: "<<m->numDimensions()<<", "
                    <<xAOD::muonSurface(m).geometryId()<<std::endl;
            }
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Collected measurements.\n"<<sstr.str());
        }
        return pickedMeas;
    }

    template <std::size_t N> 
        std::unique_ptr<ActsTrk::MutableTrackContainer> 
        MsTrackFindingAlg::fitBestCombination(const Acts::GeometryContext& tgContext,
                                              const Acts::MagneticFieldContext& mfContext,
                                              const Acts::CalibrationContext& calContext,
                                              const MsTrackSeed& seed,
                                              std::optional<Acts::BoundTrackParameters> startPars,
                                              PseudoMeasHandle_t& auxMeasContainer) const {
        std::unique_ptr<ActsTrk::MutableTrackContainer> bestFit{};
        const EventContext& ctx{*calContext.get<const EventContext*>()};
        const auto stations = seed.stations();
        std::array<StIndex, N> pickedStations{};
            
        double bestRedChi2{std::numeric_limits<double>::max()};
        Acts::detail::CombinatoricIndices<N> stationPicker{stations.size()};
        for (const auto& combo : stationPicker) {
            for (std::size_t i =0 ; i < N; ++i){
                pickedStations[i] = stations[combo[i]];
            }
            if constexpr(N == 2) {
                if (Acts::rangeContainsValue(pickedStations, StIndex::BE) ||
                    Acts::rangeContainsValue(pickedStations, StIndex::EE) ||
                    toLayerIndex(pickedStations.front()) == toLayerIndex(pickedStations.back()) ||
                    (Acts::rangeContainsValue(pickedStations, StIndex::EM) &&
                     Acts::rangeContainsValue(pickedStations, StIndex::EO))){
                    continue;
                }
            }
            
            MsTrackSeed partialSeed{seed.location(), seed.sector()};
            for (const xAOD::MuonSegment* segment : seed.segments()) {
                const StIndex stIdx = toStationIndex(segment->chamberIndex());
                if (!Acts::rangeContainsValue(pickedStations, stIdx)) {
                    continue;
                }
                partialSeed.addSegment(segment);
            }
            ATH_MSG_DEBUG(__func__<<"<"<<N<<">() "<<__LINE__<<" - Try station combination "<<pickedStations);
            
            if (startPars) {
                const Amg::Vector3D startPos = startPars->position(tgContext);
                const Acts::GeometryIdentifier volId = volumeId(xAOD::muonSurface(getMeasurement(*partialSeed.segments()[0], 0)));
                const Acts::TrackingVolume* volume{MuonGMR4::highestAlignable(m_trackingGeometrySvc->trackingGeometry()->findVolume(volId))};
                auto boundSurf = MuonGMR4::bottomBoundary(*volume);
                if (!boundSurf) {
                    ATH_MSG_WARNING(__func__<<"<"<<N<<">() "<<__LINE__<<" - Failed to find boundary surface for tracking volume");
                    continue;
                }
                
                ATH_MSG_VERBOSE(__func__<<"<"<<N<<">() "<<__LINE__<<" - Start position: "<<Amg::toString(startPos)
                                <<", perp: "<<startPos.perp()<<", "<<startPars->referenceSurface().center(tgContext).perp()
                                <<", surf center: "<<Amg::toString(boundSurf->center(tgContext))
                                <<", perp: "<<boundSurf->center(tgContext).perp()
                                <<", "<<(boundSurf->center(tgContext)-startPos).dot(startPars->direction()));
                /// The start parameters of the partial seed are behind the first segment to fit
                if (boundSurf->geometryId() != startPars->referenceSurface().geometryId() && 
                    (boundSurf->center(tgContext)-startPos).dot(startPars->direction()) < 0.) {

                    auto firstSurface = Acts::Surface::makeShared<Acts::PlaneSurface>(boundSurf->localToGlobalTransform(tgContext) *
                                                                                      Amg::getTranslateZ3D(-10._cm));
                    ATH_MSG_DEBUG(__func__<<"<"<<N<<">() "<<__LINE__<<" - Extrapolate back onto surface "
                                    <<firstSurface->center(tgContext).perp()
                                    <<", "<<boundSurf->center(tgContext).perp());
                    
                    auto extpPars = m_extrapolationTool->propagate(ctx, *startPars, *firstSurface,
                                                                   Acts::Direction::Backward());
                    if(extpPars.ok()) {
                        startPars = std::move(*extpPars);
                    }
                }
            } else {
                auto seedPars = m_seedingTool->estimateStartParameters(ctx, partialSeed);
                if (seedPars.ok()) {
                    startPars = std::move(*seedPars);
                }
            }
            if (!startPars) {
                ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Start parameter estimation failed.");
                continue;
            }
            const MeasVec_t measurements = collectMeasurements(tgContext, *startPars, partialSeed, auxMeasContainer);
            auto fit = m_trackFitTool->fit(measurements, *startPars, tgContext, mfContext, calContext, nullptr);
            if (!fit) {
                ATH_MSG_DEBUG(__func__<<"<"<<N<<">() "<<__LINE__<<" - Fit failed.");
                continue;
            }
            ActsTrk::MutableTrackContainer::TrackProxy track = fit->getTrack(0);
            const std::size_t nFitStations = m_summaryTool->countMuonStations(ctx, track);
            const double redChi2 = track.chi2() / std::max<double>(1., track.nDoF());
            ATH_MSG_DEBUG(__func__<<"<"<<N<<">() "<<__LINE__<<" -  Combination "<<pickedStations
                            <<" gives chi2/nDoF: "<<redChi2<<", nStations: "<<nFitStations);
            if (nFitStations >= N && redChi2 < bestRedChi2) {
                bestRedChi2 = redChi2;
                bestFit = std::move(fit);
            }
        }
        return bestFit;
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
        MeasVec_t measurements = collectMeasurements(tgContext, *initialPars, seed, auxMeasContainer);
  
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
                auto startPars = fitAttempts.size() >  0ul ?
                            std::make_optional<Acts::BoundTrackParameters>(fitAttempts.getTrack(fitAttempts.size() - 1ul).createParametersAtReference()) 
                            : std::nullopt; 
                std::unique_ptr<ActsTrk::MutableTrackContainer> fitTraject{};
                switch (nStation) {
                    case 2ul:
                        fitTraject = fitBestCombination<2>(tgContext, mfContext, calContext, seed, startPars, auxMeasContainer);
                        break;
                    case 3ul:
                        fitTraject = fitBestCombination<3>(tgContext, mfContext, calContext, seed, startPars, auxMeasContainer);
                        break;
                    case 4ul:
                        fitTraject = fitBestCombination<4>(tgContext, mfContext, calContext, seed, startPars, auxMeasContainer);
                        break;
                    case 5ul:
                        fitTraject = fitBestCombination<5>(tgContext, mfContext, calContext, seed, startPars, auxMeasContainer);
                        break;
                    case 6ul:
                        fitTraject = fitBestCombination<6>(tgContext, mfContext, calContext, seed, startPars, auxMeasContainer);
                        break;
                    default:
                        ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - A seven station track is extraordinary\n"<<seed);
                }
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
            auto& toAppend = track.component<std::vector<const xAOD::MuonSegment*>>("muonSegLinks");
            toAppend.insert_range(toAppend.end(), seed.segments());
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
