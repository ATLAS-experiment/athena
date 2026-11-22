/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "src/TrackFindingAlg.h"

// Athena
#include "AthenaMonitoringKernel/Monitored.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "InDetRIO_OnTrack/PixelClusterOnTrack.h"
#include "InDetRIO_OnTrack/SCT_ClusterOnTrack.h"

// ACTS
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Utilities/TrackHelpers.hpp"
#include "Acts/TrackFitting/MbfSmoother.hpp"
#include "Acts/TrackFitting/GsfMixtureReduction.hpp"
#include "Acts/TrackFitting/BetheHeitlerApprox.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "ActsInterop/Logger.h"
#include "ActsCalibrators/SourceLinkHash.h"

// ActsTrk

#include "ActsEvent/TrackContainer.h"
#include "ActsGeometry/ActsDetectorElement.h"

#include "ActsEvent/ExpectedHitUtils.h"
#include "ActsEvent/TrackContainerUtils.h"
#include "detail/Definitions.h"
#include "src/detail/TrackFindingMeasurements.h"
#include "src/detail/SharedHitCounter.h"

// STL
#include <Acts/Propagator/StandardAborters.hpp>
#include <optional>
#include <sstream>
#include <functional>
#include <stdexcept>
#include <utility>
#include <algorithm>
#include <iterator>
#include <ranges>
#include <variant>
#include <span>
#include <array>

namespace {

  static std::optional<ActsTrk::detail::RecoTrackStateContainerProxy> getFirstMeasurementFromTrack(typename ActsTrk::detail::RecoTrackContainer::TrackProxy trackProxy) {
    std::optional<ActsTrk::detail::RecoTrackStateContainerProxy> firstMeasurement {std::nullopt};
    for (auto st : trackProxy.trackStatesReversed()) {
      // We are excluding non measurement states and outlier here. Those can
      // decrease resolution because only the smoothing corrected the very
      // first prediction as filtering is not possible.
      if (not st.typeFlags().hasMeasurement()) continue;
      if (st.typeFlags().isOutlier()) continue;
      firstMeasurement = st;
    }
    return firstMeasurement;
  }

  // Collect the measurement keys of the seed duplicate measurements
  // (ICkfSeedSource::duplicateMeasurements), as the seed keys of the Acts::SeedDeduplicator.
  const std::vector<Acts::SeedDeduplicator::MeasurementKey>&
  collectSeedKeys(const ActsTrk::detail::ICkfSeedSource& seeds,
                  std::size_t iseed,
                  const ActsTrk::detail::MeasurementIndex& measurementIndex,
                  ActsTrk::detail::SeedMeasurements& seedMeasurements,
                  std::vector<Acts::SeedDeduplicator::MeasurementKey>& seedKeys) {
    seedKeys.clear();
    seeds.duplicateMeasurements(iseed, seedMeasurements);
    for (const xAOD::UncalibratedMeasurement* meas : seedMeasurements) {
      const std::size_t hitIndex = measurementIndex.index(*meas);
      if (hitIndex < measurementIndex.size()) {
        seedKeys.push_back(static_cast<Acts::SeedDeduplicator::MeasurementKey>(hitIndex));
      }
    }
    return seedKeys;
  }

  // Links the unsmoothed two-way passes of the brem search, with the interface of Acts::TrackletJoiner
  struct BremLinker {
    using TrkProxy = ActsTrk::detail::RecoTrackContainer::TrackProxy;
    using StateProxy = ActsTrk::detail::RecoTrackStateContainerProxy;

    Acts::Result<void> joinInPlace(const Acts::GeometryContext& /*gctx*/, TrkProxy& first,
                                   StateProxy sharedState, TrkProxy& second) const {
      const auto tipIndex = first.tipIndex();
      if (second.tipIndex() != Acts::kTrackIndexInvalid) {
        second.reverseTrackStates(true);
        sharedState.previous() = second.outermostTrackState().index();
      }
      first.copyFromWithoutStates(second);
      first.tipIndex() = tipIndex;
      first.stemIndex() = second.stemIndex();
      return Acts::Result<void>::success();
    }

    Acts::Result<TrkProxy> joinCopy(const Acts::GeometryContext& gctx, ActsTrk::detail::RecoTrackContainer& out,
                                    const TrkProxy& first, const StateProxy& sharedState,
                                    const TrkProxy& second) const {
      // position of the shared state, counted from the tip
      std::size_t position = 0;
      for (auto ts : first.trackStatesReversed()) {
        if (ts.index() == sharedState.index()) break;
        ++position;
      }
      TrkProxy firstCopy = out.makeTrack();
      firstCopy.copyFrom(first);
      TrkProxy secondCopy = out.makeTrack();
      secondCopy.copyFrom(second);
      auto copyStates = firstCopy.trackStatesReversed();
      StateProxy sharedCopy = *std::ranges::next(copyStates.begin(), position);
      (void)joinInPlace(gctx, firstCopy, sharedCopy, secondCopy);  // a link cannot fail
      // the second copy is a shell now, its states are in the joined chain
      out.removeTrack(secondCopy.index());
      return Acts::Result<TrkProxy>::success(firstCopy);
    }
  };

}



namespace ActsTrk
{
  struct TrackFindingBaseAlg::CKF_pimpl : public detail::CKF_config {};
  struct TrackFindingBaseAlg::bremCKF_pimpl : public detail::bremCKF_config {};

  TrackFindingAlg::TrackFindingAlg(const std::string &name, ISvcLocator *pSvcLocator)
    : TrackFindingBaseAlg(name, pSvcLocator) {}

  // === initialize ==========================================================

  StatusCode TrackFindingAlg::initialize()
  {
    ATH_MSG_INFO("Initializing " << name() << " ... ");

    ATH_CHECK(TrackFindingBaseAlg::initialize());
    ATH_MSG_DEBUG("   " << m_skipDuplicateSeeds);
    ATH_MSG_DEBUG("   " << m_statEtaBins);
    ATH_MSG_DEBUG("   " << m_seedLabels);
    ATH_MSG_DEBUG("   " << m_dumpAllStatEtaBins);
    ATH_MSG_DEBUG("   " << m_useTopSpRZboundary);
    ATH_MSG_DEBUG("   " << m_seedMeasOffset);
    ATH_MSG_DEBUG("   " << m_ambiStrategy);
    ATH_MSG_DEBUG("   " << m_autoReverseSearch);
    ATH_MSG_DEBUG("   " << m_twoWaySmoothing);
    ATH_MSG_DEBUG("   " << m_countSharedHits);
    ATH_MSG_DEBUG("   " << m_forceTrackOnSeed);

    ATH_CHECK(m_paramEstimationTool.retrieve());
    ATH_CHECK(m_seedContainerKeys.initialize());
    ATH_CHECK(m_initialParametersKeys.initialize());
    ATH_CHECK(m_seedMeasurementsKeys.initialize());
    ATH_CHECK(m_uncalibratedMeasurementContainerKeys.initialize());
    ATH_CHECK(m_volumeIdToDetectorElementCollMapKey.initialize());
    ATH_CHECK(m_detElStatus.initialize());
    ATH_CHECK(m_beamSpotKey.initialize());
    ATH_CHECK(m_caloClusterROIKey.initialize(m_doBremRecovery.value()));

    m_storeDestinies = not m_seedDestiny.empty();
    ATH_CHECK(m_seedDestiny.initialize(m_storeDestinies));

    if (useInitialParameters()) {
      if (!m_seedContainerKeys.empty()) {
        ATH_MSG_FATAL("Seeds are given both by SeedContainerKeys and InitialParametersKeys, only one of them can be used");
        return StatusCode::FAILURE;
      }
      if (m_seedMeasurementsKeys.size() != m_initialParametersKeys.size()) {
        ATH_MSG_FATAL("There are " << m_initialParametersKeys.size() << " InitialParametersKeys, but " << m_seedMeasurementsKeys.size() << " SeedMeasurementsKeys");
        return StatusCode::FAILURE;
      }
      if (m_autoReverseSearch) {
        ATH_MSG_FATAL("autoReverseSearch needs the seed space points and cannot be used with InitialParametersKeys");
        return StatusCode::FAILURE;
      }
      if (!m_paramEstimationTool.empty()) {
        ATH_MSG_WARNING("The TrackParamsEstimationTool is not used, since the initial parameters are given by InitialParametersKeys");
      }
    } else if (!m_seedMeasurementsKeys.empty()) {
      ATH_MSG_FATAL("SeedMeasurementsKeys are only used together with InitialParametersKeys");
      return StatusCode::FAILURE;
    } else if (m_paramEstimationTool.size() != m_seedContainerKeys.size()) {
      ATH_MSG_FATAL("There are " << m_seedContainerKeys.size() << " SeedContainerKeys. Each needs its own TrackParamsEstimationTool, but there are " << m_paramEstimationTool.size());
    }

    if (m_useTopSpRZboundary.size() != 2) {
      ATH_MSG_FATAL("useTopSpRZboundary must have 2 elements, but has " << m_useTopSpRZboundary.size());
      return StatusCode::FAILURE;
    }

    if (m_autoReverseSearch && m_doTwoWay && !m_twoWaySmoothing) {
      ATH_MSG_WARNING("autoReverseSearch without twoWaySmoothing: the perigee of a reversed seed does not use the outward second pass");
    }
    if (m_twoWaySmoothing && m_inflateCovarianceTwoWay) {
      ATH_MSG_WARNING("twoWaySmoothing with inflateCovarianceTwoWay: the join underweights the second pass");
    }

    {
      Acts::TrackletJoiner::Config joinerCfg;
      joinerCfg.smoothSecond = m_twoWaySmoothing;
      joinerCfg.conditionFirst = m_twoWaySmoothing;
      m_trackletJoiner.emplace(joinerCfg, logger().cloneWithSuffix("TrackletJoiner"));
    }

    if (m_ambiStrategy != 0u /* OUTSIDE_TF */) m_showResolvedStats = true;
    if (m_ambiStrategy == 1u /* END_OF_TF */) {
      Acts::GreedyAmbiguityResolution::Config cfg;
      cfg.maximumSharedHits = m_maximumSharedHits;
      cfg.maximumIterations = m_maximumIterations;
      cfg.nMeasurementsMin = m_nMeasurementsMin;

      m_ambi.emplace(std::move(cfg), makeActsAthenaLogger(this, "Acts"));
    }

    if (m_storeDestinies) {
      if (m_seedDestiny.size() != nSeedInputs()) {
        ATH_MSG_ERROR("There are " << m_seedDestiny.size() << " seed destiny collections, but " << nSeedInputs() << " seed collections");
        return StatusCode::FAILURE;
      }
    }

    return StatusCode::SUCCESS;
  }

  // === finalize ============================================================

  StatusCode TrackFindingAlg::finalize() {
    ATH_CHECK(TrackFindingBaseAlg::finalize());
    return StatusCode::SUCCESS;
  }

  // === execute =============================================================

  StatusCode TrackFindingAlg::execute(const EventContext &ctx) const
  {
    ATH_MSG_DEBUG("Executing " << name() << " ... ");

    auto timer = Monitored::Timer<std::chrono::milliseconds>("TIME_execute");
    auto mon_nTracks = Monitored::Scalar<int>("nTracks");
    auto mon = Monitored::Group(m_monTool, timer, mon_nTracks);

    // ================================================== //
    // ===================== INPUTS ===================== //
    // ================================================== //
    //
    // CALO CLUSTER ROIS
    const ROIPhiRZContainer* caloClusterROIs = nullptr;
    if (m_doBremRecovery) {
      SG::ReadHandle<ROIPhiRZContainer> caloClusterROIsHandle{m_caloClusterROIKey, ctx};
      if (!caloClusterROIsHandle.isValid()) {
        ATH_MSG_FATAL("Failed to read ROIPhiRZContainer with key " << m_caloClusterROIKey.key());
        return StatusCode::FAILURE;
      }
      caloClusterROIs = caloClusterROIsHandle.cptr();
    }

    // SEED SOURCES: provide the initial parameters and the seed measurements to the CKF
    std::vector<std::unique_ptr<detail::ICkfSeedSource>> seedSources;
    std::size_t total_seeds = 0;
    if (useInitialParameters()) {
      // INITIAL PARAMETERS + SEED MEASUREMENTS
      std::vector<const ActsTrk::BoundTrackParametersContainer *> parameterContainers;
      std::vector<const ActsTrk::SeedMeasurementsContainer *> seedMeasurementsContainers;
      std::size_t total_seed_measurements = 0;
      ATH_CHECK(getContainersFromKeys(ctx, m_initialParametersKeys, parameterContainers, total_seeds));
      ATH_CHECK(getContainersFromKeys(ctx, m_seedMeasurementsKeys, seedMeasurementsContainers, total_seed_measurements));
      seedSources.reserve(parameterContainers.size());
      for (std::size_t icontainer = 0; icontainer < parameterContainers.size(); ++icontainer) {
        if (parameterContainers[icontainer]->size() != seedMeasurementsContainers[icontainer]->size()) {
          ATH_MSG_ERROR("There are " << parameterContainers[icontainer]->size() << " initial parameters in " << m_initialParametersKeys[icontainer].key()
                        << ", but " << seedMeasurementsContainers[icontainer]->size() << " seeds in " << m_seedMeasurementsKeys[icontainer].key());
          return StatusCode::FAILURE;
        }
        seedSources.push_back(std::make_unique<detail::ParameterSeedSource>(*parameterContainers[icontainer],
                                                                            *seedMeasurementsContainers[icontainer]));
      }
    } else {
      // SEED TRIPLETS
      std::vector<const ActsTrk::SeedContainer *> seedContainers;
      ATH_CHECK(getContainersFromKeys(ctx, m_seedContainerKeys, seedContainers, total_seeds));
      seedSources.reserve(seedContainers.size());
      for (std::size_t icontainer = 0; icontainer < seedContainers.size(); ++icontainer) {
        seedSources.push_back(std::make_unique<detail::SpacePointSeedSource>(*seedContainers[icontainer],
                                                                             *m_paramEstimationTool[icontainer],
                                                                             *m_trackingGeometrySvc->surfaceIdMap(),
                                                                             *m_trackingGeometrySvc->trackingGeometry(),
                                                                             [this](const ActsTrk::Seed &seed) -> std::size_t {
                                                                               return searchStartSp(seed);
                                                                             }));
      }
    }

    // DESTINIES
    std::vector< std::unique_ptr< std::vector<int> > > destinies {};
    if (m_storeDestinies) {
      destinies.reserve( seedSources.size() );
      for (std::size_t i(0); i<seedSources.size(); ++i) {
        destinies.push_back( std::make_unique< std::vector<int> >( seedSources.at(i)->size(), DestinyType::UNKNOWN) );
      }
    }

    // MEASUREMENTS
    std::vector<const xAOD::UncalibratedMeasurementContainer *> uncalibratedMeasurementContainers;
    std::size_t total_measurements = 0;
    ATH_CHECK(getContainersFromKeys(ctx, m_uncalibratedMeasurementContainerKeys, uncalibratedMeasurementContainers, total_measurements));

    // map detector element status to volume ids
    SG::ReadCondHandle<ActsTrk::ActsVolumeIdToDetectorElementCollectionMap>
      volumeIdToDetectorElementCollMap(m_volumeIdToDetectorElementCollMapKey,ctx);
    ATH_CHECK(volumeIdToDetectorElementCollMap.isValid());
    std::vector< const InDet::SiDetectorElementStatus *> detElStatusArr;
    const std::vector<const InDetDD::SiDetectorElementCollection*> &det_el_collections =volumeIdToDetectorElementCollMap->collections();
    detElStatusArr.resize( det_el_collections.size(), nullptr);
    for (const SG::ReadHandleKey<InDet::SiDetectorElementStatus> &det_el_status_key : m_detElStatus) {
      SG::ReadHandle<InDet::SiDetectorElementStatus> det_el_status(det_el_status_key,ctx);
      ATH_CHECK( det_el_status.isValid());
      const std::vector<const InDetDD::SiDetectorElementCollection*>::const_iterator
        det_el_col_iter = std::find(det_el_collections.begin(),
                                    det_el_collections.end(),
                                    &det_el_status->getDetectorElements());
      detElStatusArr.at(det_el_col_iter - det_el_collections.begin()) = det_el_status.cptr();
    }

    detail::MeasurementIndex measurementIndex(uncalibratedMeasurementContainers.size());
    for (std::size_t icontainer = 0; icontainer < uncalibratedMeasurementContainers.size(); ++icontainer) {
      measurementIndex.addMeasurements(*uncalibratedMeasurementContainers[icontainer]);
    }

    detail::TrackFindingMeasurements measurements(uncalibratedMeasurementContainers.size());
    for (std::size_t icontainer = 0; icontainer < uncalibratedMeasurementContainers.size(); ++icontainer) {
      ATH_MSG_DEBUG("Create " << uncalibratedMeasurementContainers[icontainer]->size() << " source links from measurements in " << m_uncalibratedMeasurementContainerKeys[icontainer].key());
      measurements.addMeasurements(icontainer,
                                   *uncalibratedMeasurementContainers[icontainer],
                                   *m_trackingGeometrySvc->surfaceIdMap(),
                                   m_forceTrackOnSeed ? &measurementIndex : nullptr);
    }

    ATH_MSG_DEBUG("measurement index size = " << measurementIndex.size());

    if (m_trackStatePrinter.isSet()) {
      m_trackStatePrinter->printMeasurements(ctx, uncalibratedMeasurementContainers, measurements.measurementOffsets());
    }

    // A seed is a duplicate if one accepted track holds all but
    // seedMeasOffset of its measurements. Each measurement counts 1.
    Acts::SeedDeduplicator seedDeduplicator({.minScore = 0, .maxMissingScore = m_seedMeasOffset.value()});
    if (m_skipDuplicateSeeds) {
      seedDeduplicator.reset(measurementIndex.size());
    }

    // Get Beam pos and make pSurface
    SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle = SG::makeHandle( m_beamSpotKey, ctx );
    ATH_CHECK( beamSpotHandle.isValid() );
    const InDet::BeamSpotData* beamSpotData = beamSpotHandle.cptr();

    // Beam Spot Position
    Acts::Vector3 beamPos( beamSpotData->beamPos().x() * Acts::UnitConstants::mm,
                           beamSpotData->beamPos().y() * Acts::UnitConstants::mm,
                           0 );

    // Construct a perigee surface as the target surface
    std::shared_ptr<Acts::PerigeeSurface> pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(beamPos);

    // ================================================== //
    // ===================== CONDS ====================== //
    // ================================================== //


    // ================================================== //
    // ===================== COMPUTATION ================ //
    // ================================================== //
    Acts::VectorTrackContainer actsTrackBackend;
    Acts::VectorMultiTrajectory actsTrackStateBackend;
    {
      std::lock_guard<std::mutex> lock( m_mutex );
      actsTrackBackend.reserve(m_nTrackReserve);
      actsTrackStateBackend.reserve(m_nTrackStateReserve);
    }
    detail::RecoTrackContainer actsTracksContainer(actsTrackBackend,
                                                   actsTrackStateBackend);

    addCountsAndProperties(actsTracksContainer, m_addCounts.value());

    detail::ExpectedLayerPatternHelper::add(actsTracksContainer);

    EventStats event_stat;
    event_stat.resize(m_stat.size());

    DetectorContextHolder detContext {
      .geometry = m_ctxProvider.getGeometryContext(ctx),
      .magField = m_ctxProvider.getMagneticFieldContext(ctx),
      // CalibrationContext converter not implemented yet.
      .calib = m_ctxProvider.getCalibrationContext(ctx)
    };

    detail::SharedHitCounter sharedHits;
    std::optional<std::vector<unsigned int>> trackCategories;
    if (m_ambi) trackCategories.emplace();  // only needed if m_ambiStrategy == END_OF_TF

    // Perform the track finding for all initial parameters.
    for (std::size_t icontainer = 0; icontainer < seedSources.size(); ++icontainer)
      {
        ATH_CHECK(findTracks(ctx,
                             detContext,
                             measurements,
                             measurementIndex,
                             detElStatusArr,
                             *(volumeIdToDetectorElementCollMap.cptr()),
                             sharedHits,
                             seedDeduplicator,
                             *seedSources.at(icontainer),
                             actsTracksContainer,
                             icontainer,
                             icontainer < m_seedLabels.size() ? m_seedLabels[icontainer].c_str() : seedInputKey(icontainer).c_str(),
                             event_stat,
                             m_storeDestinies ? destinies.at(icontainer).get() : nullptr,
                             *pSurface.get(),
                             trackCategories,
                             caloClusterROIs));
      }

    ATH_MSG_DEBUG("    \\__ Created " << actsTracksContainer.size() << " tracks");

    mon_nTracks = actsTracksContainer.size();


    // ================================================== //
    // ===================== OUTPUTS ==================== //
    // ================================================== //

    // Save the seed destinies
    if (m_storeDestinies) {
      for (std::size_t i(0); i<destinies.size(); ++i) {
        const SG::WriteHandleKey< std::vector<int> >& writeKey = m_seedDestiny.at(i);
        // make the handle and record
        SG::WriteHandle< std::vector<int> > destinyHandle = SG::makeHandle( writeKey, ctx );
        ATH_CHECK( destinyHandle.record( std::move(  destinies.at(i) )  ) );
      }
    }

    {
      std::lock_guard<std::mutex> lock( m_mutex );
      // update the reserve space
      if (actsTrackBackend.size() > m_nTrackReserve) {
        m_nTrackReserve = static_cast<std::size_t>( std::ceil(m_memorySafetyMargin * actsTrackBackend.size()) );
      }
      if (actsTrackStateBackend.size() > m_nTrackStateReserve) {
        m_nTrackStateReserve = static_cast<std::size_t>( std::ceil(m_memorySafetyMargin * actsTrackStateBackend.size()) );
      }
    }

    // handle the ambiguity
    // we potentially need to short list the track candidates and make some copies
    if (not m_ambi) {
      copyStats(event_stat);
      // no need to shortlist anything. just use the actsTracksContainer
      ATH_MSG_DEBUG("    \\__ Created " << actsTracksContainer.size() << " resolved tracks");
      ATH_CHECK( storeTrackCollectionToStoreGate( ctx,
                                                  std::move(actsTrackBackend),
                                                  std::move(actsTrackStateBackend) ) );
      return StatusCode::SUCCESS;
    }

    // we have asked for the ambi
    // we start by shortlisting the container
    Acts::VectorTrackContainer resolvedTrackBackend;
    Acts::VectorMultiTrajectory resolvedTrackStateBackend;
    resolvedTrackBackend.reserve( actsTrackBackend.size() );
    resolvedTrackStateBackend.reserve( actsTrackStateBackend.size() );
    detail::RecoTrackContainer resolvedTracksContainer(resolvedTrackBackend, resolvedTrackStateBackend);
    detail::ExpectedLayerPatternHelper::add(resolvedTracksContainer);

    addCountsAndProperties(resolvedTracksContainer, m_addCounts.value());

    // Start ambiguity resolution
    Acts::GreedyAmbiguityResolution::State state;
    m_ambi->computeInitialState(actsTracksContainer, state, &detail::sourceLinkHash,
                                &detail::sourceLinkEquality);
    m_ambi->resolve(state);

    // Copy the resolved tracks into the output container
    // We need a different sharedHits counter here because it saves the track index
    // and since I ran the resolving the track indices changed.
    detail::SharedHitCounter sharedHits_forFinalAmbi;

    // shotlist
    for (auto iTrack : state.selectedTracks) {
      int actsTrackIndex = state.trackTips.at(iTrack);
      auto destProxy = resolvedTracksContainer.makeTrack();
      destProxy.copyFrom(actsTracksContainer.getTrack(actsTrackIndex));

      unsigned int category_i = trackCategories->at(actsTrackIndex);
      ++event_stat[category_i][kNResolvedTracks];

      if (m_countSharedHits) {
        auto [nShared, nBadTrackMeasurements] = sharedHits_forFinalAmbi.computeSharedHits(destProxy, resolvedTracksContainer, measurementIndex);
        if (nBadTrackMeasurements > 0)
          ATH_MSG_ERROR("computeSharedHits: " << nBadTrackMeasurements << " track measurements not found in input track");
      }
    } // loop on tracks

    ATH_MSG_DEBUG("    \\__ Created " << resolvedTracksContainer.size() << " resolved tracks");
    copyStats(event_stat);

    ATH_CHECK( storeTrackCollectionToStoreGate( ctx,
                                                std::move(resolvedTrackBackend),
                                                std::move(resolvedTrackStateBackend)) );

    return StatusCode::SUCCESS;
  }

  StatusCode TrackFindingAlg::storeTrackCollectionToStoreGate(const EventContext& ctx,
                                                              Acts::VectorTrackContainer&& originalTrackBackend,
                                                              Acts::VectorMultiTrajectory&& originalTrackStateBackend) const
  {
    // convert to const
    Acts::ConstVectorTrackContainer constTrackBackend( std::move(originalTrackBackend) );
    Acts::ConstVectorMultiTrajectory constTrackStateBackend( std::move(originalTrackStateBackend) );
    std::unique_ptr< ActsTrk::TrackContainer> constTracksContainer = std::make_unique< ActsTrk::TrackContainer >( std::move(constTrackBackend),
                                                                                                                  std::move(constTrackStateBackend) );

    SG::WriteHandle<ActsTrk::TrackContainer> trackContainerHandle = SG::makeHandle(m_trackContainerKey, ctx);
    ATH_MSG_DEBUG("    \\__ Tracks Container `" << m_trackContainerKey.key() << "` created ...");
    ATH_CHECK(trackContainerHandle.record(std::move(constTracksContainer)));
    return StatusCode::SUCCESS;
  }

  std::size_t TrackFindingAlg::searchStartSp(const ActsTrk::Seed& seed) const {
    const xAOD::SpacePoint* bottom_sp = seed.sp().front();
    const bool reverse = m_autoReverseSearch && (bottom_sp->radius() > m_useTopSpRZboundary.value()[0] ||
                                                 std::abs(bottom_sp->z()) > m_useTopSpRZboundary.value()[1]);
    return reverse ? seed.sp().size() - 1 : 0;
  }

  // === findTracks ==========================================================

  StatusCode
  TrackFindingAlg::findTracks(const EventContext &ctx,
                              const DetectorContextHolder& detContext,
                              const detail::TrackFindingMeasurements &measurements,
                              const detail::MeasurementIndex &measurementIndex,
                              const std::vector< const InDet::SiDetectorElementStatus *> &detElStatusArr,
                              const ActsTrk::ActsVolumeIdToDetectorElementCollectionMap &volumeIdMap,
                              detail::SharedHitCounter &sharedHits,
                              Acts::SeedDeduplicator &seedDeduplicator,
                              const detail::ICkfSeedSource &seeds,
                              detail::RecoTrackContainer &actsTracksContainer,
                              std::size_t typeIndex,
                              const char *seedType,
                              EventStats &event_stat,
                              std::vector<int>* destiny,
                              const Acts::PerigeeSurface& pSurface,
                              std::optional<std::vector<unsigned int>>& trackCategories,
                              const ROIPhiRZContainer *caloClusterROIs) const
  {
    ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);

    auto [options, secondOptions, measurementSelector]
       = getDefaultOptions(ctx, detContext, measurements, detElStatusArr,volumeIdMap, &pSurface);
    std::optional<TrackFindingBremOptions> bremOptions;
    if (m_doBremRecovery){
        bremOptions = getBremOptions(ctx, detContext, measurements, detElStatusArr,volumeIdMap, &pSurface);
    }

    Acts::VectorTrackContainer trackBackend;
    Acts::VectorMultiTrajectory trackStateBackend;
    detail::RecoTrackContainer tracksContainerTemp(trackBackend, trackStateBackend);

    addCountsAndProperties(tracksContainerTemp, m_addCounts.value());

    detail::ExpectedLayerPatternHelper::add(tracksContainerTemp);

    std::size_t category_i = 0;
    const auto &trackSelectorCfg = trackFinder().trackSelector.config();

    auto stopBranchProxy = [&](const detail::RecoTrackContainer::TrackProxy &track,
                               const detail::RecoTrackContainer::TrackStateProxy &trackState) -> BranchStopperResult {
      return stopBranch(track, trackState, trackSelectorCfg, detContext.geometry, measurementIndex, typeIndex, event_stat[category_i]);
    };
    options.extensions.branchStopper.connect(stopBranchProxy);

    // Without a stopper the second pass runs past the last findable hit, and a
    // stray hit beyond the gap turns the layers it skipped into interior holes
    // that trimTrack cannot remove. Stop it there instead and let addTrack
    // extrapolate to the perigee. It must not drop: the first pass already
    // found this track.
    auto stopSecondBranchProxy = [&](const detail::RecoTrackContainer::TrackProxy &track,
                                     const detail::RecoTrackContainer::TrackStateProxy &trackState) -> BranchStopperResult {
      const BranchStopperResult result =
        stopBranch(track, trackState, trackSelectorCfg, detContext.geometry, measurementIndex, typeIndex, event_stat[category_i]);
      return result == BranchStopperResult::StopAndDrop ? BranchStopperResult::StopAndKeep : result;
    };
    if (m_doSecondPassBranchStopper) {
      secondOptions.extensions.branchStopper.connect(stopSecondBranchProxy);
    }

    if (m_doBremRecovery){
        bremOptions.value().options.extensions.branchStopper.connect(stopBranchProxy);
        if (m_doSecondPassBranchStopper) {
            bremOptions.value().secondOptions.extensions.branchStopper.connect(stopSecondBranchProxy);
        }
    }

    Acts::PropagatorOptions<detail::Stepper::Options, detail::Navigator::Options,
                            Acts::ActorList<Acts::MaterialInteractor>>
      extrapolationOptions(detContext.geometry, detContext.magField);

    Acts::TrackExtrapolationStrategy extrapolationStrategy =
      Acts::TrackExtrapolationStrategy::first;

    // Perform the track finding for all initial parameters
    ATH_MSG_DEBUG("Invoke track finding with " << seeds.size() << ' ' << seedType << " seeds.");


    std::size_t nPrinted = 0;
    detail::SeedMeasurements seedMeasurements;

    // The seed keys of the duplicate check: the seed duplicate measurements
    std::vector<Acts::SeedDeduplicator::MeasurementKey> seedKeys;

    // Loop over the track finding results for all initial parameters
    for (unsigned int iseed = 0; iseed < seeds.size(); ++iseed)
      {
        
        category_i = typeIndex * (m_statEtaBins.size() + 1);
        tracksContainerTemp.clear();

        // Check if the seed is a duplicate seed
        const bool isDupSeed = m_skipDuplicateSeeds &&
          seedDeduplicator.isDuplicate(collectSeedKeys(seeds, iseed, measurementIndex, seedMeasurements, seedKeys));
        if (isDupSeed) {
          ATH_MSG_DEBUG("skip " << seedType << " seed " << iseed << " - already found");
          category_i = getStatCategory(typeIndex, seeds.statEta(iseed));
          ++event_stat[category_i][kNTotalSeeds];
          ++event_stat[category_i][kNDuplicateSeeds];
          if (m_storeDestinies) destiny->at(iseed) = DestinyType::DUPLICATE;
          if (!m_trackStatePrinter.isSet()) continue;  // delay continue to estimate track parms for TrackStatePrinter?
        }

        // Get first estimate of parameters from the seed
        const auto& [optTrackParams, estimationStatus] =
          seeds.initialParameters(iseed, detContext.geometry, detContext.magField, detContext.calib);

        if (!optTrackParams) {
          ATH_MSG_DEBUG("Failed to estimate track parameters for seed " << iseed);
          if (!isDupSeed) {
            category_i = getStatCategory(typeIndex, seeds.statEta(iseed));
            ++event_stat[category_i][kNTotalSeeds];
            ++event_stat[category_i][kNNoEstimatedParams];
            if (m_storeDestinies) destiny->at(iseed) = DestinyType::FAILURE;
          }
          continue;
        }

        printSeed(iseed, detContext, seeds, *optTrackParams, measurementIndex, nPrinted, seedType);
        if (isDupSeed) continue;  // skip now if not done before

        double etaInitial = -std::log(std::tan(0.5 * optTrackParams->theta()));
        category_i = getStatCategory(typeIndex, etaInitial);
        ++event_stat[category_i][kNTotalSeeds];  // also updated for duplicate seeds
        ++event_stat[category_i][kNUsedSeeds];

        if (estimationStatus == ITrackParamsEstimationTool::kSeedRefitFailed) {
          ++event_stat[category_i][kNSeedRefitFailure];
        } else if (m_seedPtMinFactor > 0) {
          // Check pTmin requirement on the parameters that start the CKF (refit or estimate)
          const auto &cutSet = getCuts(etaInitial);
          if (optTrackParams->transverseMomentum() < cutSet.ptMin * m_seedPtMinFactor) {
            ATH_MSG_VERBOSE("min pt requirement not satisfied by the seed parameters: pt min is " << cutSet.ptMin << " but the seed parameters have pt of " << optTrackParams->transverseMomentum());
            ++event_stat[category_i][kNRejectedSeedParams];
            if (m_storeDestinies) destiny->at(iseed) = DestinyType::FAILURE;
            continue;
          }
        }

        // Above the bottom SP: first pass inward, then outward; a failed refit starts at the bottom SP
        const bool searchInward = seeds.searchInward(iseed) && estimationStatus != ITrackParamsEstimationTool::kSeedRefitFailed;
        options.propagatorPlainOptions.direction = searchInward ? Acts::Direction::Backward() : Acts::Direction::Forward();
        secondOptions.propagatorPlainOptions.direction = options.propagatorPlainOptions.direction.invert();
        options.targetSurface = searchInward ? &pSurface : nullptr;
        secondOptions.targetSurface = searchInward ? nullptr : &pSurface;

        // @TODO introduce additional enums to distinguish Acts CKF from other implementations?
        //

        std::unique_ptr<ActsTrk::detail::MeasurementRangeListFlat> measurementRangesForced;
        if (m_forceTrackOnSeed) {
          seeds.seedMeasurements(iseed, seedMeasurements);
          measurementRangesForced = measurements.createMeasurementRangesForced(std::span(seedMeasurements.data(), seedMeasurements.size()),
                                                                               measurementIndex);
        }

        measurementSelector->setMeasurementRangesForced(measurementRangesForced.get());
        if (bremOptions.has_value()) {
            bremOptions.value().measurementSelector->setMeasurementRangesForced(measurementRangesForced.get());
        }
        if (measurementRangesForced)
          event_stat[category_i][kNForcedSeedMeasurements] += measurementRangesForced->size();

        std::size_t ntracks = 0ul;
        // Get the Acts tracks, given this seed
        ATH_CHECK(runTrackFinder<TrackFinderOptions>(
            detContext,
            pSurface,
            extrapolationStrategy,
            sharedHits,
            actsTracksContainer,
            measurementIndex,
            tracksContainerTemp,
            seedDeduplicator,
            destiny,
            event_stat,
            ntracks,
            iseed,
            category_i,
            seedType,
            trackCategories,
            options,
            secondOptions,
            *optTrackParams,
            searchInward));


        // No tracks and a matching calo ROI: run the brem track finding, outward first only.
        if (ntracks == 0 && !searchInward && caloClusterROIs != nullptr && hasMatchingCaloROI(detContext.geometry, optTrackParams, *caloClusterROIs)){
            ++event_stat[category_i][kNBremSeeds];
            ATH_CHECK(runTrackFinder<BremTrackFinderOptions>(
                detContext,
                pSurface,
                extrapolationStrategy,
                sharedHits,
                actsTracksContainer,
                measurementIndex,
                tracksContainerTemp,
                seedDeduplicator,
                destiny,
                event_stat,
                ntracks,
                iseed,
                category_i,
                seedType,
                trackCategories,
                bremOptions.value().options,
                bremOptions.value().secondOptions,
                *optTrackParams,
                /*searchInward=*/false));
        }

        if (m_storeDestinies) {
          if (ntracks == 0) {
            destiny->at(iseed) = DestinyType::FAILURE;
          } else {
            destiny->at(iseed) = DestinyType::SUCCEED;
          }
        }

        if (ntracks == 0){
            ATH_MSG_DEBUG("Track finding found no track candidates for " << seedType << " seed " << iseed);
            ++event_stat[category_i][kNoTrack];
        }
        else if (ntracks >= 2){
          ++event_stat[category_i][kMultipleBranches];
        }

        if (m_trackStatePrinter.isSet())
          std::cout << std::flush;
      } // loop on seeds

    ATH_MSG_DEBUG("Completed " << seedType << " track finding with " << computeStatSum(typeIndex, kNOutputTracks, event_stat) << " track candidates.");

    return StatusCode::SUCCESS;
  }

  void
  TrackFindingAlg::storeSeedInfo(const detail::RecoTrackContainer &tracksContainer,
                                 const detail::RecoTrackContainerProxy &track,
                                 Acts::SeedDeduplicator &seedDeduplicator,
                                 const detail::MeasurementIndex &measurementIndex) const {

    // Outliers count as shared measurements, as before
    std::vector<Acts::SeedDeduplicator::MeasurementKey> trackKeys;
    tracksContainer.trackStateContainer().visitBackwards(
                                                         track.tipIndex(),
                                                         [&trackKeys,&measurementIndex](const detail::RecoTrackStateContainer::ConstTrackStateProxy &state) -> void
                                                         {
                                                           // Check there is a source link
                                                           if (not state.hasUncalibratedSourceLink())
                                                             return;

                                                           auto sl = detail::xAODUncalibMeasCalibrator::unpack(state.getUncalibratedSourceLink());
                                                           const std::size_t hitIndex = measurementIndex.index(*sl);
                                                           if (hitIndex < measurementIndex.size()) {
                                                             trackKeys.push_back(static_cast<Acts::SeedDeduplicator::MeasurementKey>(hitIndex));
                                                           }
                                                         }); // end visitBackwards
    seedDeduplicator.addTrack(trackKeys);
  }

  void TrackFindingAlg::printSeed(unsigned int iseed,
                                  const DetectorContextHolder& detContext,
                                  const detail::ICkfSeedSource& seeds,
                                  const Acts::BoundTrackParameters &seedParameters,
                                  const detail::MeasurementIndex &measurementIndex,
                                  std::size_t& nPrinted,
                                  const char *seedType,
                                  bool isKF) const
  {
    if (not m_trackStatePrinter.isSet()) return;

    // the printer needs the space points, which are not available for all seed sources
    const ActsTrk::SeedContainer *seedContainer = seeds.seedContainer();
    if (seedContainer == nullptr) return;

    if (nPrinted == 0) {
      ATH_MSG_INFO("CKF results for " << seeds.size() << ' ' << seedType << " seeds:");
    }
    ++nPrinted;
    m_trackStatePrinter->printSeed(detContext.geometry, (*seedContainer)[iseed], seedParameters, measurementIndex, iseed, isKF);
  }

namespace {
struct Collector {
  using result_type = TrackFindingAlg::ExpectedLayerPattern*;

  template <typename propagator_state_t, typename stepper_t,
  typename navigator_t>
  Acts::Result<void> act(propagator_state_t& state, const stepper_t& /*stepper*/,
           const navigator_t& navigator, result_type& result,
           const Acts::Logger& /*logger*/) const {
    const Acts::Surface* currentSurface = navigator.currentSurface(state.navigation);
    if (currentSurface == nullptr) {
      return Acts::Result<void>::success();
    }

    assert(result != nullptr && "Result type is nullptr");
    const auto* detElem = getActsDetectorElement(currentSurface);
    if(detElem != nullptr) {
      detail::addToExpectedLayerPattern(*result, *detElem);
    }


    return Acts::Result<void>::success();
  }
};
}

Acts::Result<void> TrackFindingAlg::extrapolateTrackToReferenceSurface(
  const DetectorContextHolder& detContext,
  detail::RecoTrackContainerProxy &track,
  const Acts::Surface &referenceSurface,
  const detail::Extrapolator &propagator,
  Acts::TrackExtrapolationStrategy strategy,
  ExpectedLayerPattern& expectedLayerPattern) const {

    Acts::PropagatorOptions<detail::Stepper::Options, detail::Navigator::Options,
                            Acts::ActorList<Acts::MaterialInteractor, Collector>>
    options(detContext.geometry, detContext.magField);

    auto findResult = findTrackStateForExtrapolation(
        options.geoContext, track, referenceSurface, strategy, logger());

    if (!findResult.ok()) {
      ATH_MSG_WARNING("Failed to find track state for extrapolation");
      return findResult.error();
    }

    auto &[trackState, distance] = *findResult;

    options.direction = Acts::Direction::fromScalarZeroAsPositive(distance);

    Acts::BoundTrackParameters parameters = track.createParametersFromState(trackState);
    ATH_MSG_VERBOSE("Extrapolating track to reference surface at distance "
                << distance << " with direction " << options.direction
                << " with starting parameters " << parameters);

    auto state = propagator.makeState<decltype(options), Acts::ForcedSurfaceReached>(options);
    ExpectedLayerPattern*& collectorResult = state.get<TrackFindingAlg::ExpectedLayerPattern*>();
    collectorResult = &expectedLayerPattern;

    auto initRes = propagator.initialize<decltype(state), Acts::ForcedSurfaceReached>(
        state, parameters, &referenceSurface);
    if(!initRes.ok()) {
      ATH_MSG_WARNING("Failed to initialize propagation state: " << initRes.error().message());
      return initRes.error();
    }


    auto propagateOnlyResult =
        propagator.propagate(state);

    if (!propagateOnlyResult.ok()) {
      ATH_MSG_WARNING("Failed to extrapolate track: " << propagateOnlyResult.error().message());
      return propagateOnlyResult.error();
    }

    auto propagateResult = propagator.makeResult(
        std::move(state), propagateOnlyResult, options, true, &referenceSurface);

    if (!propagateResult.ok()) {
      ATH_MSG_WARNING("Failed to extrapolate track: " << propagateResult.error().message());
      return propagateResult.error();
    }

    track.setReferenceSurface(referenceSurface.getSharedPtr());
    track.parameters() = propagateResult->endParameters.value().parameters();
    track.covariance() =
        propagateResult->endParameters.value().covariance().value();

    return Acts::Result<void>::success();
 }

  StatusCode TrackFindingAlg::addTrack(const DetectorContextHolder& detContext,
                                       detail::RecoTrackContainerProxy &track,
                                       const Acts::Surface& pSurface,
                                       const Acts::TrackExtrapolationStrategy& extrapolationStrategy,
                                       detail::SharedHitCounter &sharedHits,
                                       detail::RecoTrackContainer &actsTracksContainer,
                                       const detail::MeasurementIndex& measurementIndex,
                                       const detail::RecoTrackContainer& tracksContainerTemp,
                                       Acts::SeedDeduplicator& seedDeduplicator,
                                       std::vector<int>* destiny,
                                       EventStats& event_stat,
                                       std::size_t& ntracks,
                                       std::size_t iseed,
                                       std::size_t category_i,
                                       const char *seedType,
                                       std::optional<std::vector<unsigned int>>& trackCategories) const
  {

    std::array<unsigned int, 4> expectedLayerPattern{};
    bool bremTrack = ActsTrk::TrackContainerUtils::isBremTrack(track);
    // if the the perigeeSurface was not hit (in particular the case for the inside-out pass,
    // the track has no reference surface and the extrapolation to the perigee has not been done
    // yet.
    if (not track.hasReferenceSurface()) {
        auto extrapolationResult =
          extrapolateTrackToReferenceSurface(detContext, track,
                                            pSurface,
                                            trackFinder().extrapolator,
                                            extrapolationStrategy,
                                            expectedLayerPattern);

      if (not extrapolationResult.ok()) {
        ATH_MSG_WARNING("Extrapolation for seed "
                        << iseed << " and " << track.index()
                        << " failed with error " << extrapolationResult.error()
                        << " dropping track candidate.");
        if (m_storeDestinies) destiny->at(iseed) = DestinyType::FAILURE;
        return StatusCode::SUCCESS;
      }
    }

    // Before trimming, inspect encountered surfaces from all track states
    for(const auto ts : track.trackStatesReversed()) {
      const auto* detElem = getActsDetectorElement(ts.referenceSurface());
      if(detElem != nullptr) {
        detail::addToExpectedLayerPattern(expectedLayerPattern, *detElem);
      }
    }

    // Trim tracks
    // - trimHoles
    // - trimOutliers
    // - trimMaterial
    // - trimOtherNoneMeasurement
    Acts::trimTrack(track, true, true, true, true);
    Acts::calculateTrackQuantities(track);
    if (m_addCounts) {
      initCounts(track);
      for (const auto trackState : track.trackStatesReversed()) {
        updateCounts(track, trackState.typeFlags(), measurementType(trackState));
      }
      if (m_checkCounts) {
        checkCounts(track);
      }
    }

    ++ntracks;
    ++event_stat[category_i][kNOutputTracks];
    if (bremTrack) ++event_stat[category_i][kNBremTracks];

    bool validTrack = (bremTrack) ? bremTrackFinder().trackSelector.isValidTrack(track) : trackFinder().trackSelector.isValidTrack(track);

    if ( not validTrack or not selectCountsFinal(track)) {
    ATH_MSG_DEBUG("Track " << ntracks << " from " << seedType << " seed " << iseed << " failed track selection");
    if ( m_trackStatePrinter.isSet() ) {
        m_trackStatePrinter->printTrack(detContext.geometry, tracksContainerTemp, track, measurementIndex, true);
    }
    return StatusCode::SUCCESS;
    }

    ++event_stat[category_i][kNSelectedTracks];
    if (bremTrack) ++event_stat[category_i][kNBremSelectedTracks];

    // Fill the track infos into the duplicate seed detector
    if (m_skipDuplicateSeeds) {
      storeSeedInfo(tracksContainerTemp, track, seedDeduplicator, measurementIndex);
    }

    auto actsDestProxy   = actsTracksContainer.makeTrack();
    actsDestProxy.copyFrom(track);  // make sure we copy track states!

    detail::ExpectedLayerPatternHelper::set(actsDestProxy, expectedLayerPattern);

    auto setTrackCategory = [&]() {
      if (!trackCategories) return;
      if (!(actsDestProxy.index() < trackCategories->size())) trackCategories->resize(actsDestProxy.index()+1);
      trackCategories->at(actsDestProxy.index()) = category_i;
    };

    if (not m_countSharedHits) {
      return StatusCode::SUCCESS;
    }

    auto [nShared, nBadTrackMeasurements] = sharedHits.computeSharedHits(actsDestProxy, actsTracksContainer, measurementIndex);

    if (nBadTrackMeasurements > 0) {
      ATH_MSG_ERROR("computeSharedHits: " << nBadTrackMeasurements << " track measurements not found in input for " << seedType << " seed " << iseed << " track");
    }

    ATH_MSG_DEBUG("found " << actsDestProxy.nSharedHits() << " shared hits in " << seedType << " seed " << iseed << " track");

    event_stat[category_i][kNTotalSharedHits] += nShared;

    if (m_ambiStrategy == 2u) { // run the ambiguity during track selection

      if (actsDestProxy.nSharedHits() <= m_maximumSharedHits) {
        setTrackCategory();
        ++event_stat[category_i][kNResolvedTracks];
      }
      else { // track fails the shared hit selection

        ATH_MSG_DEBUG("found " << actsDestProxy.nSharedHits() << " shared hits in " << seedType << " seed " << iseed << " track");
        // Reset the original track shared hits by running coumputeSharedHits
        // with removeSharedHits flag to true
        // nSharedRemoved contains the total shared hits that will be removed
        auto [nSharedRemoved, nRemoveBadTrackMeasurements] = sharedHits.computeSharedHits(actsDestProxy, actsTracksContainer, measurementIndex, true);

        ATH_MSG_DEBUG("Removed " << nSharedRemoved << " shared hits in " << seedType << " seed " << iseed << " track and the matching track");

        if (nRemoveBadTrackMeasurements > 0) {
          ATH_MSG_ERROR("computeSharedHits with remove flag ON: " << nRemoveBadTrackMeasurements <<
                        " track measurements not found in input for " << seedType << " seed " << iseed << " track");
        }

        if (actsDestProxy.nSharedHits() != 0) {
          ATH_MSG_ERROR("computeSharedHits with remove flag ON returned " <<
                        actsDestProxy.nSharedHits()<< " while expecting 0 for" <<
                        seedType << " seed " << iseed << " track");
        }

        // Remove the track from the container
        actsTracksContainer.removeTrack(actsDestProxy.index());
        ATH_MSG_DEBUG("Track " << ntracks << " from " << seedType << " seed " << iseed << " failed shared hit selection");
      }
    }
    else {
      // run ambi later
      setTrackCategory();
      if (m_trackStatePrinter.isSet()) {
        m_trackStatePrinter->printTrack(detContext.geometry, actsTracksContainer, actsDestProxy, measurementIndex);
      }
    }

    return StatusCode::SUCCESS;
  }

  template <typename TrackFinderOptionsType>
  StatusCode TrackFindingAlg::runTrackFinder(const DetectorContextHolder& detContext,
                                      const Acts::Surface& pSurface,
                                      const Acts::TrackExtrapolationStrategy& extrapolationStrategy,
                                      detail::SharedHitCounter &sharedHits,
                                      detail::RecoTrackContainer &actsTracksContainer,
                                      const detail::MeasurementIndex& measurementIndex,
                                      detail::RecoTrackContainer& tracksContainerTemp,
                                      Acts::SeedDeduplicator& seedDeduplicator,
                                      std::vector<int>* destiny,
                                      EventStats& event_stat,
                                      std::size_t& ntracks,
                                      std::size_t iseed,
                                      std::size_t category_i,
                                      const char *seedType,
                                      std::optional<std::vector<unsigned int>>& trackCategories,
                                      const TrackFinderOptionsType& options,
                                      const TrackFinderOptionsType& secondOptions,
                                      const Acts::BoundTrackParameters &seedParameters,
                                      bool searchInward) const
  {

      // The brem track finding runs a Gaussian mixture filter without smoothing,
      // so its tracks are no GSF fits. They are flagged as brem tracks instead.
      constexpr bool isBremTrackFinding = !std::is_same_v<TrackFinderOptionsType, TrackFinderOptions>;

      ATH_MSG_DEBUG("Running track finding for " << seedType << " seed " << iseed << ".");

      auto rootBranch = tracksContainerTemp.makeTrack();
      Acts::Result<std::vector<TrkProxy> > result = findTracksCKF(seedParameters, options, tracksContainerTemp, rootBranch);

      if (not result.ok()){
          ATH_MSG_WARNING("Track finding failed for " << seedType << " seed " << iseed << " with error" << result.error());
          if (m_storeDestinies) destiny->at(iseed) = DestinyType::FAILURE;
          return StatusCode::SUCCESS;
      }

      auto &tracksForSeed = result.value();

      std::size_t nfirst = 0;
      for (TrkProxy &firstTrack : tracksForSeed){
        ActsTrk::TrackContainerUtils::setFitterType(firstTrack, xAOD::KalmanFitter);
        ActsTrk::TrackContainerUtils::setBremTrack(firstTrack, isBremTrackFinding);
        // The brem track states are merged Gaussian mixtures, which the Kalman
        // smoother cannot handle consistently, so brem tracks are not smoothed
        auto smoothingResult = isBremTrackFinding ? Acts::Result<void>::success()
                                                  : Acts::smoothTrack(detContext.geometry, firstTrack, logger(), Acts::MbfSmoother());
        if (!smoothingResult.ok()){
            ATH_MSG_DEBUG("Smoothing for seed "
                        << iseed << " and first track " << firstTrack.index()
                        << " failed with error " << smoothingResult.error());
            continue;
        }
        // Add the first pass alone; reverse a copy if inward, siblings share its states
        auto addFirstTrack = [&]() -> StatusCode {
            TrkProxy track = firstTrack;
            if (searchInward) {
                track = tracksContainerTemp.makeTrack();
                track.copyFrom(firstTrack);
                track.reverseTrackStates(true);
            }
            return addTrack(detContext, track, pSurface, extrapolationStrategy, sharedHits,
                            actsTracksContainer, measurementIndex, tracksContainerTemp,
                            seedDeduplicator, destiny, event_stat, ntracks, iseed,
                            category_i, seedType, trackCategories);
        };

        if (not m_doTwoWay){
            ATH_CHECK( addFirstTrack() );
            ++nfirst;
            continue;
        }

        // TWO WAY STARTS HERE
        // We need the first measurement of the track
        std::optional<detail::RecoTrackStateContainerProxy> firstMeas = getFirstMeasurementFromTrack(firstTrack);
        // we are supposed to find a measurement
        if (not firstMeas.has_value()) {
            ATH_MSG_ERROR("First measurement for seed " << iseed << " and first track " << firstTrack.index() << "failed.");
            continue;
        }
        detail::RecoTrackStateContainerProxy& firstMeasurement = firstMeas.value();

        // Get the tracks from the second track finding
        std::vector<typename detail::RecoTrackContainer::TrackProxy> secondTracksForSeed;
        // Create initial parameters for the propagation
        // Without smoothing, the brem second pass starts from the seed parameters
        Acts::BoundTrackParameters secondInitialParameters = isBremTrackFinding
            ? seedParameters
            : firstTrack.createParametersFromState(detail::RecoConstTrackStateContainerProxy{firstMeasurement});
        if (secondInitialParameters.referenceSurface().insideBounds(secondInitialParameters.localPosition())){ // #3751
            // the seed parameters are not a smoothed state and are not inflated
            if (m_inflateCovarianceTwoWay && not isBremTrackFinding){
                inflateCovarianceTwoWay(secondInitialParameters);
            }
            auto secondRootBranch = tracksContainerTemp.makeTrack();
            secondRootBranch.copyFromWithoutStates(firstTrack); // #3634
            // addTrack extrapolates the joined track to the perigee again
            if (searchInward) {
                secondRootBranch.setReferenceSurface(nullptr);
            }

            // perform track finding
            auto secondResult = findTracksCKF(secondInitialParameters, secondOptions, tracksContainerTemp, secondRootBranch);
            if (secondResult.ok()){
                secondTracksForSeed = std::move(secondResult.value());
            }
        }

        if ( secondTracksForSeed.empty() ) {
            ATH_MSG_DEBUG("No viable result from second track finding for " << seedType << " seed " << iseed << " track " << nfirst);
            ++event_stat[category_i][kNoSecond];
            ATH_CHECK( addFirstTrack() );
        }

        // KF: smooth and condition the joined passes; brem: link the unsmoothed passes
        const auto& joiner = [&]() -> const auto& {
            if constexpr (isBremTrackFinding) {
                static const BremLinker bremLinker;
                return bremLinker;
            } else {
                return *m_trackletJoiner;
            }
        }();

        // In place only for the last join of the seed: earlier siblings are already stored
        const bool lastFirstTrack = &firstTrack == &tracksForSeed.back();
        std::size_t nJoined = 0;
        bool firstTrackModified = false;
        for (std::size_t iSecond = 0; iSecond < secondTracksForSeed.size(); ++iSecond) {
            auto &secondTrack = secondTracksForSeed[iSecond];
            const bool inPlace = lastFirstTrack && iSecond + 1 == secondTracksForSeed.size();
            Acts::Result<TrkProxy> joinedTrack = Acts::Result<TrkProxy>::success(firstTrack);
            if (inPlace) {
                auto res = joiner.joinInPlace(detContext.geometry, firstTrack, firstMeasurement, secondTrack);
                if (not res.ok()) {
                    firstTrackModified = true;
                    joinedTrack = Acts::Result<TrkProxy>::failure(res.error());
                }
            } else {
                joinedTrack = joiner.joinCopy(detContext.geometry, tracksContainerTemp,
                                              firstTrack, firstMeasurement, secondTrack);
            }
            if (not joinedTrack.ok()) {
                ATH_MSG_DEBUG("Join of the two passes failed for " << seedType << " seed " << iseed
                              << " with error " << joinedTrack.error().message());
                continue;
            }
            // Put the innermost state first, as in the forward search
            if (searchInward) {
                joinedTrack->reverseTrackStates(true);
            }
            ATH_CHECK( addTrack(detContext,
                                *joinedTrack,
                                pSurface,
                                extrapolationStrategy,
                                sharedHits,
                                actsTracksContainer,
                                measurementIndex,
                                tracksContainerTemp,
                                seedDeduplicator,
                                destiny,
                                event_stat,
                                ntracks,
                                iseed,
                                category_i,
                                seedType,
                                trackCategories) );
            ++nJoined;
        }

        if (not secondTracksForSeed.empty() && nJoined == 0) {
            // A failed in-place join can leave the first track conditioned: smooth it again
            if (firstTrackModified) {
                auto resmooth = Acts::smoothTrack(detContext.geometry, firstTrack, logger(), Acts::MbfSmoother());
                if (not resmooth.ok()) {
                    ATH_MSG_DEBUG("Smoothing the first track again failed for " << seedType << " seed " << iseed);
                    nfirst++;
                    continue;
                }
            }
            ATH_CHECK( addFirstTrack() );
        }
        nfirst++;
    } // loop on tracks from seed

      return StatusCode::SUCCESS;
  }
} // namespace
