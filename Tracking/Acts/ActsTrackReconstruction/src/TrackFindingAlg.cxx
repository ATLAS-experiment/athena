/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
#include "Acts/Utilities/TrackHelpers.hpp"
#include "Acts/TrackFitting/MbfSmoother.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "ActsInterop/Logger.h"

// ActsTrk
#include "ActsCalibBase/CalibrationContext.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "src/detail/ExpectedHitUtils.h"
#include "src/detail/TrackFindingMeasurements.h"
#include "src/detail/SharedHitCounter.h"

// STL
#include <Acts/Propagator/StandardAborters.hpp>
#include <sstream>
#include <functional>
#include <stdexcept>
#include <utility>
#include <algorithm>
#include <variant>

namespace {
  static std::size_t sourceLinkHash(const Acts::SourceLink& slink) {
    const ActsTrk::ATLASUncalibSourceLink &atlasSourceLink = slink.get<ActsTrk::ATLASUncalibSourceLink>();
    const xAOD::UncalibratedMeasurement &uncalibMeas = ActsTrk::getUncalibratedMeasurement(atlasSourceLink);
    return uncalibMeas.identifier();
  }

  static bool sourceLinkEquality(const Acts::SourceLink& a, const Acts::SourceLink& b) {
    const xAOD::UncalibratedMeasurement &uncalibMeas_a = ActsTrk::getUncalibratedMeasurement(a.get<ActsTrk::ATLASUncalibSourceLink>());
    const xAOD::UncalibratedMeasurement &uncalibMeas_b = ActsTrk::getUncalibratedMeasurement(b.get<ActsTrk::ATLASUncalibSourceLink>());
    return uncalibMeas_a.identifier() == uncalibMeas_b.identifier();
  }

  static std::optional<ActsTrk::detail::RecoTrackStateContainerProxy> getFirstMeasurementFromTrack(typename ActsTrk::detail::RecoTrackContainer::TrackProxy trackProxy) {
    std::optional<ActsTrk::detail::RecoTrackStateContainerProxy> firstMeasurement {std::nullopt};
    for (auto st : trackProxy.trackStatesReversed()) {
      // We are excluding non measurement states and outlier here. Those can
      // decrease resolution because only the smoothing corrected the very
      // first prediction as filtering is not possible.
      if (not st.typeFlags().test(Acts::TrackStateFlag::MeasurementFlag)) continue;
      if (st.typeFlags().test(Acts::TrackStateFlag::OutlierFlag)) continue;
      firstMeasurement = st;
    }
    return firstMeasurement;
  }

}



namespace ActsTrk
{
  struct TrackFindingBaseAlg::CKF_pimpl : public detail::CKF_config {};

  TrackFindingAlg::TrackFindingAlg(const std::string &name, ISvcLocator *pSvcLocator)
    : TrackFindingBaseAlg(name, pSvcLocator) {}

  // === initialize ==========================================================

  StatusCode TrackFindingAlg::initialize()
  {
    ATH_MSG_INFO("Initializing " << name() << " ... ");

    ATH_CHECK(TrackFindingBaseAlg::initialize());
    ATH_MSG_DEBUG("   " << m_skipDuplicateSeeds);
    ATH_MSG_DEBUG("   " << m_refitSeeds);
    ATH_MSG_DEBUG("   " << m_statEtaBins);
    ATH_MSG_DEBUG("   " << m_seedLabels);
    ATH_MSG_DEBUG("   " << m_dumpAllStatEtaBins);
    ATH_MSG_DEBUG("   " << m_useTopSpRZboundary);
    ATH_MSG_DEBUG("   " << m_seedMeasOffset);
    ATH_MSG_DEBUG("   " << m_ambiStrategy);
    ATH_MSG_DEBUG("   " << m_autoReverseSearch);
    ATH_MSG_DEBUG("   " << m_countSharedHits);
    ATH_MSG_DEBUG("   " << m_forceTrackOnSeed);

    ATH_CHECK(m_paramEstimationTool.retrieve());
    ATH_CHECK(m_seedContainerKeys.initialize());
    ATH_CHECK(m_detEleCollKeys.initialize());
    ATH_CHECK(m_uncalibratedMeasurementContainerKeys.initialize());
    ATH_CHECK(m_volumeIdToDetectorElementCollMapKey.initialize());
    ATH_CHECK(m_detElStatus.initialize());
    ATH_CHECK(m_beamSpotKey.initialize());

    m_storeDestinies = not m_seedDestiny.empty();
    ATH_CHECK(m_seedDestiny.initialize(m_storeDestinies));

    if (m_seedContainerKeys.size() != m_detEleCollKeys.size())
      {
        ATH_MSG_FATAL("There are " << m_detEleCollKeys.size() << " DetectorElementsKeys, but " << m_seedContainerKeys.size() << " SeedContainerKeys");
        return StatusCode::FAILURE;
      }

    if (m_detEleCollKeys.size() != m_seedLabels.size())
      {
        ATH_MSG_FATAL("There are " << m_seedLabels.size() << " SeedLabels, but " << m_detEleCollKeys.size() << " DetectorElementsKeys");
        return StatusCode::FAILURE;
      }

    if (m_useTopSpRZboundary.size() != 2)
      {
        ATH_MSG_FATAL("useTopSpRZboundary must have 2 elements, but has " << m_useTopSpRZboundary.size());
        return StatusCode::FAILURE;
      }

    if (m_ambiStrategy == 1u /* END_OF_TF */) {
      Acts::GreedyAmbiguityResolution::Config cfg;
      cfg.maximumSharedHits = m_maximumSharedHits;
      cfg.maximumIterations = m_maximumIterations;
      cfg.nMeasurementsMin = m_nMeasurementsMin;

      m_ambi.emplace(std::move(cfg), makeActsAthenaLogger(this, "Acts"));
    }

    if (m_storeDestinies) {
      if (m_seedDestiny.size() != m_seedContainerKeys.size()) {
        ATH_MSG_ERROR("There are " << m_seedDestiny.size() << " seed destiny collections, but " << m_seedContainerKeys.size() << " seed collections");
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

    // SEED TRIPLETS
    std::vector<const ActsTrk::SeedContainer *> seedContainers;
    std::size_t total_seeds = 0;
    ATH_CHECK(getContainersFromKeys(ctx, m_seedContainerKeys, seedContainers, total_seeds));

    // DESTINIES
    std::vector< std::unique_ptr< std::vector<int> > > destinies {};
    if (m_storeDestinies) {
      destinies.reserve( seedContainers.size() );
      for (std::size_t i(0); i<seedContainers.size(); ++i) {
        destinies.push_back( std::make_unique< std::vector<int> >( seedContainers.at(i)->size(), DestinyType::UNKNOWN) );
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
    std::vector< const InDet::SiDetectorElementStatus *> det_el_status_arr;
    const std::vector<const InDetDD::SiDetectorElementCollection*> &det_el_collections =volumeIdToDetectorElementCollMap->collections();
    det_el_status_arr.resize( det_el_collections.size(), nullptr);
    for (const SG::ReadHandleKey<InDet::SiDetectorElementStatus> &det_el_status_key : m_detElStatus) {
      SG::ReadHandle<InDet::SiDetectorElementStatus> det_el_status(det_el_status_key,ctx);
      ATH_CHECK( det_el_status.isValid());
      const std::vector<const InDetDD::SiDetectorElementCollection*>::const_iterator
        det_el_col_iter = std::find(det_el_collections.begin(),
                                    det_el_collections.end(),
                                    &det_el_status->getDetectorElements());
      det_el_status_arr.at(det_el_col_iter - det_el_collections.begin()) = det_el_status.cptr();
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
                                   *m_trackingGeometryTool->surfaceIdMap(),
                                   m_forceTrackOnSeed ? &measurementIndex : nullptr);
    }

    ATH_MSG_DEBUG("measurement index size = " << measurementIndex.size());

    ATH_CHECK( propagateDetectorElementStatusToMeasurements(*(volumeIdToDetectorElementCollMap.cptr()), det_el_status_arr, measurements) );

    if (m_trackStatePrinter.isSet()) {
      m_trackStatePrinter->printMeasurements(ctx, uncalibratedMeasurementContainers, measurements.measurementOffsets());
    }

    detail::DuplicateSeedDetector duplicateSeedDetector(total_seeds,
                                                        m_seedMeasOffset.value(),
                                                        m_skipDuplicateSeeds);
    for (std::size_t icontainer = 0; icontainer < seedContainers.size(); ++icontainer)
      {
        duplicateSeedDetector.addSeeds(icontainer, *seedContainers[icontainer], measurementIndex,
                                       m_paramEstimationTool->spacePointIndicesFun(),
                                       [this,icontainer](const ActsTrk::Seed& seed) -> bool {
                                         const bool reverseSearch = m_autoReverseSearch && shouldReverseSearch(seed);
                                         const bool refitSeeds = icontainer < m_refitSeeds.size() && m_refitSeeds[icontainer];
                                         const bool useTopSp = reverseSearch && !refitSeeds;
                                         return useTopSp;
                                       });
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

    std::vector<const InDetDD::SiDetectorElementCollection*> detElementsCollections;
    std::size_t total_detElems = 0;
    ATH_CHECK(getContainersFromKeys(ctx, m_detEleCollKeys, detElementsCollections, total_detElems));

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

    if (m_addCounts) {
      addCounts(actsTracksContainer);
    }

    detail::ExpectedLayerPatternHelper::add(actsTracksContainer);

    EventStats event_stat;
    event_stat.resize(m_stat.size());

    DetectorContextHolder detContext {
      .geometry = m_trackingGeometryTool->getGeometryContext(ctx).context(),
      .magField = m_extrapolationTool->getMagneticFieldContext(ctx),
      // CalibrationContext converter not implemented yet.
      .calib = getCalibrationContext(ctx)
    };

    detail::SharedHitCounter sharedHits;

    // Perform the track finding for all initial parameters.
    for (std::size_t icontainer = 0; icontainer < seedContainers.size(); ++icontainer)
      {
        ATH_CHECK(findTracks(detContext,
                             measurements,
                             measurementIndex,
                             sharedHits,
                             duplicateSeedDetector,
                             *seedContainers.at(icontainer),
                             *detElementsCollections.at(icontainer),
                             actsTracksContainer,
                             icontainer,
                             icontainer < m_seedLabels.size() ? m_seedLabels[icontainer].c_str() : m_seedContainerKeys[icontainer].key().c_str(),
                             event_stat,
                             m_storeDestinies ? destinies.at(icontainer).get() : nullptr,
                             *pSurface.get()));
      }

    ATH_MSG_DEBUG("    \\__ Created " << actsTracksContainer.size() << " tracks");

    mon_nTracks = actsTracksContainer.size();
    copyStats(event_stat);


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

    if (m_addCounts) {
      addCounts(resolvedTracksContainer);
    }

    // Start ambiguity resolution
    Acts::GreedyAmbiguityResolution::State state;
    m_ambi->computeInitialState(actsTracksContainer, state, &sourceLinkHash,
                                &sourceLinkEquality);
    m_ambi->resolve(state);

    // Copy the resolved tracks into the output container
    // We need a different sharedHits counter here because it saves the track index
    // and since I ran the resolving the track indices changed.
    detail::SharedHitCounter sharedHits_forFinalAmbi;

    // shotlist
    for (auto iTrack : state.selectedTracks) {
      auto destProxy = resolvedTracksContainer.makeTrack();
      destProxy.copyFrom(actsTracksContainer.getTrack(state.trackTips.at(iTrack)));

      if (m_countSharedHits) {
        auto [nShared, nBadTrackMeasurements] = sharedHits_forFinalAmbi.computeSharedHits(destProxy, resolvedTracksContainer, measurementIndex);
        if (nBadTrackMeasurements > 0)
          ATH_MSG_ERROR("computeSharedHits: " << nBadTrackMeasurements << " track measurements not found in input track");
      }
    } // loop on tracks

    ATH_MSG_DEBUG("    \\__ Created " << resolvedTracksContainer.size() << " resolved tracks");

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

  bool TrackFindingAlg::shouldReverseSearch(const ActsTrk::Seed& seed) const {
    const auto& bottom_sp = seed.sp().front();

    const double r = bottom_sp->radius();
    const double z = std::abs(bottom_sp->z());

    const double rBoundary = m_useTopSpRZboundary.value()[0];
    const double zBoundary = m_useTopSpRZboundary.value()[1];

    return r > rBoundary || z > zBoundary;
  }

  // === findTracks ==========================================================

  StatusCode
  TrackFindingAlg::findTracks(const DetectorContextHolder& detContext,
                              const detail::TrackFindingMeasurements &measurements,
                              const detail::MeasurementIndex &measurementIndex,
                              detail::SharedHitCounter &sharedHits,
                              detail::DuplicateSeedDetector &duplicateSeedDetector,
                              const ActsTrk::SeedContainer &seeds,
                              const InDetDD::SiDetectorElementCollection& detElements,
                              detail::RecoTrackContainer &actsTracksContainer,
                              std::size_t typeIndex,
                              const char *seedType,
                              EventStats &event_stat,
                              std::vector<int>* destiny,
                              const Acts::PerigeeSurface& pSurface) const
  {
    ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);

    auto [options, secondOptions, measurementSelector] = getDefaultOptions(detContext, measurements, &pSurface);

    // ActsTrk::MutableTrackContainer tracksContainerTemp;
    Acts::VectorTrackContainer trackBackend;
    Acts::VectorMultiTrajectory trackStateBackend;
    detail::RecoTrackContainer tracksContainerTemp(trackBackend, trackStateBackend);

    if (m_addCounts) {
      addCounts(tracksContainerTemp);
    }

    detail::ExpectedLayerPatternHelper::add(tracksContainerTemp);

    std::size_t category_i = 0;
    const auto &trackSelectorCfg = trackFinder().trackSelector.config();
    auto stopBranchProxy = [&](const detail::RecoTrackContainer::TrackProxy &track,
                               const detail::RecoTrackContainer::TrackStateProxy &trackState) -> BranchStopperResult {
      return stopBranch(track, trackState, trackSelectorCfg, detContext.geometry, measurementIndex, typeIndex, event_stat[category_i]);
    };
    options.extensions.branchStopper.connect(stopBranchProxy);

    Acts::PropagatorOptions<detail::Stepper::Options, detail::Navigator::Options,
                            Acts::ActorList<Acts::MaterialInteractor>>
      extrapolationOptions(detContext.geometry, detContext.magField);

    Acts::TrackExtrapolationStrategy extrapolationStrategy =
      Acts::TrackExtrapolationStrategy::first;

    // Perform the track finding for all initial parameters
    ATH_MSG_DEBUG("Invoke track finding with " << seeds.size() << ' ' << seedType << " seeds.");


    std::size_t nPrinted = 0;

    // Function for Estimate Track Parameters
    auto retrieveSurfaceFunction =
      [this, &detElements] (const ActsTrk::Seed& seed, bool useTopSp) -> const Acts::Surface& {
        const xAOD::SpacePoint* sp = useTopSp ? seed.sp().back() : seed.sp().front();
        const InDetDD::SiDetectorElement* element = detElements.getDetectorElement(useTopSp ? sp->elementIdList().back()
                                                                                   : sp->elementIdList().front());
        const Trk::Surface& atlas_surface = element->surface();
        return m_ATLASConverterTool->trkSurfaceToActsSurface(atlas_surface);
      };



    // Loop over the track finding results for all initial parameters
    for (unsigned int iseed = 0; iseed < seeds.size(); ++iseed)
      {
        // Get the seed
        const ActsTrk::Seed& seed = *seeds[iseed];

        category_i = typeIndex * (m_statEtaBins.size() + 1);
        tracksContainerTemp.clear();

        const bool reverseSearch = m_autoReverseSearch && shouldReverseSearch(seed);
        const bool refitSeeds = typeIndex < m_refitSeeds.size() && m_refitSeeds[typeIndex];
        const bool useTopSp = reverseSearch && !refitSeeds;

        // Check if the seed is a duplicate seed
        const bool isDupSeed = duplicateSeedDetector.isDuplicate(typeIndex, iseed);

        if (isDupSeed) {
          ATH_MSG_DEBUG("skip " << seedType << " seed " << iseed << " - already found");
          category_i = getSeedCategory(typeIndex, seed, useTopSp);
          ++event_stat[category_i][kNTotalSeeds];
          ++event_stat[category_i][kNDuplicateSeeds];
          if (m_storeDestinies) destiny->at(iseed) = DestinyType::DUPLICATE;
          if (!m_trackStatePrinter.isSet()) continue;  // delay continue to estimate track parms for TrackStatePrinter?
        }

        // Set the option accordingly - we change the direction and the target surface accordingly
        options.propagatorPlainOptions.direction = reverseSearch ? Acts::Direction::Backward() : Acts::Direction::Forward();
        secondOptions.propagatorPlainOptions.direction = options.propagatorPlainOptions.direction.invert();
        options.targetSurface = reverseSearch ? &pSurface : nullptr;
        secondOptions.targetSurface = reverseSearch ? nullptr : &pSurface;
        // TODO since the second pass is strictly an extension we should have a separate branch stopper which never drops and always extrapolates to the target surface

        // Get first estimate of parameters from the seed
        std::optional<Acts::BoundTrackParameters> optTrackParams =
          m_paramEstimationTool->estimateTrackParameters(seed,
                                                         useTopSp,
                                                         detContext.geometry,
                                                         detContext.magField,
                                                         retrieveSurfaceFunction);

        if (!optTrackParams) {
          ATH_MSG_DEBUG("Failed to estimate track parameters for seed " << iseed);
          if (!isDupSeed) {
            category_i = getSeedCategory(typeIndex, seed, useTopSp);
            ++event_stat[category_i][kNTotalSeeds];
            ++event_stat[category_i][kNNoEstimatedParams];
            if (m_storeDestinies) destiny->at(iseed) = DestinyType::FAILURE;
          }
          continue;
        }

        Acts::BoundTrackParameters *initialParameters = &(*optTrackParams);
        printSeed(iseed, detContext, seeds, *initialParameters, measurementIndex, nPrinted, seedType);
        if (isDupSeed) continue;  // skip now if not done before

        double etaInitial = -std::log(std::tan(0.5 * initialParameters->theta()));
        category_i = getStatCategory(typeIndex, etaInitial);
        ++event_stat[category_i][kNTotalSeeds];  // also updated for duplicate seeds
        ++event_stat[category_i][kNUsedSeeds];

        // Optional refit track parameters to get more refined value
        std::unique_ptr<Acts::BoundTrackParameters> refitSeedParameters;
        if (refitSeeds) {
          refitSeedParameters = doRefit(seed, *initialParameters, detContext, reverseSearch);
          if (refitSeedParameters.get() == nullptr) {
            ++event_stat[category_i][kNRejectedRefinedSeeds];
            if (m_storeDestinies) destiny->at(iseed) = DestinyType::FAILURE;
            continue;
          }
          if (refitSeedParameters.get() != initialParameters) {
            initialParameters = refitSeedParameters.get();
            printSeed(iseed, detContext, seeds, *initialParameters, measurementIndex, nPrinted, seedType, true);
          }
        }

        auto measurementRangesForced =
            m_forceTrackOnSeed ? std::make_unique<ActsTrk::detail::MeasurementRangeListFlat>(measurements.setMeasurementRangesForced(seed, measurementIndex))
                               : nullptr;
        measurementSelector->setMeasurementRangesForced(measurementRangesForced.get());
        if (measurementRangesForced)
          event_stat[category_i][kNForcedSeedMeasurements] += measurementRangesForced->size();

        // Get the Acts tracks, given this seed
        // Result here contains a vector of TrackProxy objects
        auto result = trackFinder().ckf.findTracks(*initialParameters, options, tracksContainerTemp);

        // The result for this seed
        if (not result.ok()) {
          ATH_MSG_WARNING("Track finding failed for " << seedType << " seed " << iseed << " with error" << result.error());
          if (m_storeDestinies) destiny->at(iseed) = DestinyType::FAILURE;
          continue;
        }
        auto &tracksForSeed = result.value();



        std::size_t ntracks = 0ul;

        // loop on the tracks we have just found from the seed
        std::size_t nfirst = 0;
        for (TrkProxy &firstTrack : tracksForSeed) {
          // smoothing
          auto smoothingResult = Acts::smoothTrack(detContext.geometry, firstTrack, logger(), Acts::MbfSmoother());
          if (!smoothingResult.ok()) {
            ATH_MSG_DEBUG("Smoothing for seed "
                          << iseed << " and first track " << firstTrack.index()
                          << " failed with error " << smoothingResult.error());
            continue;
          }

          // if no two way, just add the track and move on
          if (not m_doTwoWay) {
            // add the track to the collection
            ATH_CHECK( addTrack(detContext,
                                firstTrack,
                                pSurface,
                                extrapolationStrategy,
                                sharedHits,
                                actsTracksContainer,
                                measurementIndex,
                                tracksContainerTemp,
                                duplicateSeedDetector,
                                destiny,
                                event_stat,
                                ntracks,
                                iseed,
                                category_i,
                                seedType) );
            ++nfirst;
            continue;
          }

          // TWO WAY STARTS HERE
          // We need the first measurement of the track
          std::optional<detail::RecoTrackStateContainerProxy> firstMeas = getFirstMeasurementFromTrack(firstTrack);
          // we are supposed to find a measurement
          if (not firstMeas.has_value()) {
            ATH_MSG_ERROR("Could not retrieve first measurement from track proxy. Is it ill-formed?");
            return StatusCode::FAILURE;
          }
          detail::RecoTrackStateContainerProxy& firstMeasurement = firstMeas.value();

          // Get the tracks from the second track finding
          std::vector<typename detail::RecoTrackContainer::TrackProxy> secondTracksForSeed =
            doTwoWayTrackFinding(firstMeasurement,
                                 firstTrack,
                                 tracksContainerTemp,
                                 secondOptions);

          if ( secondTracksForSeed.empty() ) {
            ATH_MSG_DEBUG("No viable result from second track finding for " << seedType << " seed " << iseed << " track " << nfirst);
            ++event_stat[category_i][kNoSecond];
            ATH_CHECK( addTrack(detContext,
                                firstTrack,
                                pSurface,
                                extrapolationStrategy,
                                sharedHits,
                                actsTracksContainer,
                                measurementIndex,
                                tracksContainerTemp,
                                duplicateSeedDetector,
                                destiny,
                                event_stat,
                                ntracks,
                                iseed,
                                category_i,
                                seedType) );
          }

          // need to add tracks here
          // do the stiching
          // store the original previous state to restore it later
          auto originalFirstMeasurementPrevious = firstMeasurement.previous();
          for (auto &secondTrack : secondTracksForSeed) {
            secondTrack.reverseTrackStates(true);

            firstMeasurement.previous() = secondTrack.outermostTrackState().index();
            secondTrack.tipIndex() = firstTrack.tipIndex();

            if (reverseSearch) {
              // smooth the full track
              auto secondSmoothingResult = Acts::smoothTrack(detContext.geometry,
                                                             secondTrack,
                                                             logger());
              if ( not secondSmoothingResult.ok() ) {
                continue;
              }
              secondTrack.reverseTrackStates(true);
            }

            // Add track to collection
            ATH_CHECK( addTrack(detContext,
                                secondTrack,
                                pSurface,
                                extrapolationStrategy,
                                sharedHits,
                                actsTracksContainer,
                                measurementIndex,
                                tracksContainerTemp,
                                duplicateSeedDetector,
                                destiny,
                                event_stat,
                                ntracks,
                                iseed,
                                category_i,
                                seedType) );
          } // loop on tracks

          // finish the stiching
          // restore the original previous state for the first track
          firstMeasurement.previous() = originalFirstMeasurementPrevious;

          nfirst++;
        } // loop on tracks from seed

        if (m_storeDestinies) {
          if (ntracks == 0) {
            destiny->at(iseed) = DestinyType::FAILURE;
          } else {
            destiny->at(iseed) = DestinyType::SUCCEED;
          }
        }

        if (ntracks == 0) {
          ATH_MSG_DEBUG("Track finding found no track candidates for " << seedType << " seed " << iseed);
          ++event_stat[category_i][kNoTrack];
        } else if (ntracks >= 2) {
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
                                 detail::DuplicateSeedDetector &duplicateSeedDetector,
                                 const detail::MeasurementIndex &measurementIndex) const {

    const auto lastMeasurementIndex = track.tipIndex();
    duplicateSeedDetector.newTrajectory();

    tracksContainer.trackStateContainer().visitBackwards(
                                                         lastMeasurementIndex,
                                                         [&duplicateSeedDetector,&measurementIndex](const detail::RecoTrackStateContainer::ConstTrackStateProxy &state) -> void
                                                         {
                                                           // Check there is a source link
                                                           if (not state.hasUncalibratedSourceLink())
                                                             return;

                                                           // Fill the duplicate selector
                                                           auto sl = state.getUncalibratedSourceLink().template get<ATLASUncalibSourceLink>();
                                                           duplicateSeedDetector.addMeasurement(sl, measurementIndex);
                                                         }); // end visitBackwards
  }

  StatusCode TrackFindingAlg::propagateDetectorElementStatusToMeasurements(const ActsTrk::ActsVolumeIdToDetectorElementCollectionMap &volume_id_to_det_el_coll,
                                                                           const std::vector< const InDet::SiDetectorElementStatus *> &det_el_status_arr,
                                                                           detail::TrackFindingMeasurements &measurements) const {
    const Acts::TrackingGeometry *
      acts_tracking_geometry = m_trackingGeometryTool->trackingGeometry().get();
    ATH_CHECK(acts_tracking_geometry != nullptr);

    using Counter = struct { unsigned int n_volumes, n_volumes_with_status, n_missing_detector_elements, n_detector_elements, n_disabled_detector_elements;};
    Counter counter {0u,0u,0u,0u,0u};
    acts_tracking_geometry->visitVolumes([&counter,
                                          &volume_id_to_det_el_coll,
                                          &det_el_status_arr,
                                          &measurements,
                                          this](const Acts::TrackingVolume *volume_ptr) {
      ++counter.n_volumes;
      if (!volume_ptr) return;

      const InDet::SiDetectorElementStatus*
        det_el_status = det_el_status_arr.at(volume_id_to_det_el_coll.collecionMap().at(volume_ptr->geometryId().volume()));
      if (det_el_status) {
        ++counter.n_volumes_with_status;
        volume_ptr->visitSurfaces([&counter, det_el_status, &measurements,this](const Acts::Surface *surface_ptr) {
          if (!surface_ptr) return;
          const Acts::Surface &surface = *surface_ptr;
          const Acts::DetectorElementBase*detector_element = surface.associatedDetectorElement();
          if (detector_element) {
            ++counter.n_detector_elements;
            const ActsDetectorElement *acts_detector_element = static_cast<const ActsDetectorElement*>(detector_element);
            if (!det_el_status->isGood( acts_detector_element->identifyHash() )) {
              ActsTrk::detail::MeasurementRange old_range = measurements.markSurfaceInsensitive(surface_ptr->geometryId());
              if (!old_range.empty()) {
                auto geoid_to_string = [](const Acts::GeometryIdentifier &id) -> std::string  {
                  std::stringstream amsg;
                  amsg << id;
                  return amsg.str();
                };
                std::string a_msg ( geoid_to_string(surface_ptr->geometryId()));
                ATH_MSG_WARNING("Reject " << (old_range.elementEndIndex() - old_range.elementBeginIndex())
                                << " measurements because surface " << a_msg);
              }
              ++counter.n_disabled_detector_elements;
            }
          }
        }, true /*only sensitive surfaces*/);
      }
      else {
        ++counter.n_missing_detector_elements;
      }
    });
    ATH_MSG_DEBUG("Volumes with detector element status " << counter.n_volumes_with_status << " / " << counter.n_volumes
                  << " disabled detector elements " << counter.n_disabled_detector_elements
                  << " / " << counter.n_detector_elements
                  << " missing detector elements "
                  << counter.n_missing_detector_elements);
    return StatusCode::SUCCESS;
  }

  std::size_t TrackFindingAlg::getSeedCategory(std::size_t typeIndex,
                                               const ActsTrk::Seed& seed,
                                               bool useTopSp) const
  {
    const xAOD::SpacePoint* sp = useTopSp ? seed.sp().back() : seed.sp().front();
    const xAOD::SpacePoint::ConstVectorMap pos = sp->globalPosition();
    double etaSeed = std::atanh(pos[2] / pos.norm());
    return getStatCategory(typeIndex, etaSeed);
  }

  void TrackFindingAlg::printSeed(unsigned int iseed,
                                  const DetectorContextHolder& detContext,
                                  const ActsTrk::SeedContainer& seeds,
                                  const Acts::BoundTrackParameters &seedParameters,
                                  const detail::MeasurementIndex &measurementIndex,
                                  std::size_t& nPrinted,
                                  const char *seedType,
                                  bool isKF) const
  {
    if (not m_trackStatePrinter.isSet()) return;

    if (nPrinted == 0) {
      ATH_MSG_INFO("CKF results for " << seeds.size() << ' ' << seedType << " seeds:");
    }
    ++nPrinted;
    m_trackStatePrinter->printSeed(detContext.geometry, *seeds[iseed], seedParameters, measurementIndex, iseed, isKF);
  }

namespace {
struct Collector {

  using result_type = TrackFindingAlg::ExpectedLayerPattern*;

  template <typename propagator_state_t, typename stepper_t,
  typename navigator_t>
  void act(propagator_state_t& state, const stepper_t& /*stepper*/,
           const navigator_t& navigator, result_type& result,
           const Acts::Logger& /*logger*/) const {
    const auto* currentSurface = navigator.currentSurface(state.navigation);
    if (currentSurface == nullptr) {
      return;
    }

    assert(result != nullptr && "Result type is nullptr");

    if (currentSurface->associatedDetectorElement() != nullptr) {
      const auto* detElem = dynamic_cast<const ActsDetectorElement*>(currentSurface->associatedDetectorElement());
      if(detElem != nullptr) {
        detail::addToExpectedLayerPattern(*result, *detElem);
      }
    }
  };
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
      ACTS_ERROR("failed to find track state for extrapolation");
      return findResult.error();
    }

    auto &[trackState, distance] = *findResult;

    options.direction = Acts::Direction::fromScalarZeroAsPositive(distance);

    Acts::BoundTrackParameters parameters = track.createParametersFromState(trackState);
    ACTS_VERBOSE("extrapolating track to reference surface at distance "
                << distance << " with direction " << options.direction
                << " with starting parameters " << parameters);

    auto state = propagator.makeState<decltype(options), Acts::ForcedSurfaceReached>(referenceSurface, options);
    ExpectedLayerPattern*& collectorResult = state.get<TrackFindingAlg::ExpectedLayerPattern*>();
    collectorResult = &expectedLayerPattern;

    auto initRes = propagator.initialize(state, parameters);
    if(!initRes.ok()) {
      ACTS_ERROR("Failed to initialize propgation state: " << initRes.error().message());
      return initRes.error();
    }


    auto propagateOnlyResult =
        propagator.propagate(state);

    if (!propagateOnlyResult.ok()) {
      ACTS_ERROR("failed to extrapolate track: " << propagateOnlyResult.error().message());
      return propagateOnlyResult.error();
    }

    auto propagateResult = propagator.makeResult(
        std::move(state), propagateOnlyResult, referenceSurface, options);

    if (!propagateResult.ok()) {
      ACTS_ERROR("failed to extrapolate track: " << propagateResult.error().message());
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
                                       detail::DuplicateSeedDetector& duplicateSeedDetector,
                                       std::vector<int>* destiny,
                                       EventStats& event_stat,
                                       std::size_t& ntracks,
                                       std::size_t iseed,
                                       std::size_t category_i,
                                       const char *seedType) const
  {

    std::array<unsigned int, 4> expectedLayerPattern;

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
      const auto& surface = ts.referenceSurface();
      if(surface.associatedDetectorElement() != nullptr) {
        const auto* detElem = dynamic_cast<const ActsDetectorElement*>(surface.associatedDetectorElement());
        if(detElem != nullptr) {
          detail::addToExpectedLayerPattern(expectedLayerPattern, *detElem);
        }
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
      checkCounts(track);
    }

    ++ntracks;
    ++event_stat[category_i][kNOutputTracks];

    if ( not trackFinder().trackSelector.isValidTrack(track) or
         not selectCountsFinal(track)) {
      ATH_MSG_DEBUG("Track " << ntracks << " from " << seedType << " seed " << iseed << " failed track selection");
      if ( m_trackStatePrinter.isSet() ) {
        m_trackStatePrinter->printTrack(detContext.geometry, tracksContainerTemp, track, measurementIndex, true);
      }
      return StatusCode::SUCCESS;
    }

    // Fill the track infos into the duplicate seed detector
    if (m_skipDuplicateSeeds) {
      storeSeedInfo(tracksContainerTemp, track, duplicateSeedDetector, measurementIndex);
    }

    auto actsDestProxy   = actsTracksContainer.makeTrack();
    actsDestProxy.copyFrom(track);  // make sure we copy track states!

    detail::ExpectedLayerPatternHelper::set(actsDestProxy, expectedLayerPattern);

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
        ++event_stat[category_i][kNSelectedTracks];
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
    else { // use ambi during selection
      ++event_stat[category_i][kNSelectedTracks];

      if (m_trackStatePrinter.isSet()) {
        m_trackStatePrinter->printTrack(detContext.geometry, actsTracksContainer, actsDestProxy, measurementIndex);
      }
    }

    return StatusCode::SUCCESS;
  }

} // namespace
