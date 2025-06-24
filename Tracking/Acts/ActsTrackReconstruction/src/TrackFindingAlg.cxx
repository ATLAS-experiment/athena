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
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Utilities/TrackHelpers.hpp"
#include "Acts/TrackFitting/MbfSmoother.hpp"

// ActsTrk
#include "ActsCalibBase/CalibrationContext.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "src/detail/TrackFindingMeasurements.h"
#include "src/detail/SharedHitCounter.h"

// STL
#include <sstream>
#include <functional>
#include <utility>
#include <algorithm>

namespace ActsTrk
{
  struct TrackFindingBaseAlg::CKF_pimpl : public detail::CKF_config {};

  TrackFindingAlg::TrackFindingAlg(const std::string &name, ISvcLocator *pSvcLocator)
      : TrackFindingBaseAlg(name, pSvcLocator) {}

  TrackFindingAlg::~TrackFindingAlg() = default;

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

    ATH_CHECK(m_seedContainerKeys.initialize());
    ATH_CHECK(m_detEleCollKeys.initialize());
    ATH_CHECK(m_uncalibratedMeasurementContainerKeys.initialize());
    ATH_CHECK(m_volumeIdToDetectorElementCollMapKey.initialize());
    ATH_CHECK(m_detElStatus.initialize());

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

    detail::TrackFindingMeasurements measurements(uncalibratedMeasurementContainers.size() /* number of measurement containers*/);
    std::size_t measurementIndexContainersSize = (m_skipDuplicateSeeds || m_countSharedHits || m_trackStatePrinter.isSet()) ? uncalibratedMeasurementContainers.size() : 0ul;
    detail::MeasurementIndex measurementIndex(measurementIndexContainersSize);
    detail::SharedHitCounter sharedHits;

    for (std::size_t icontainer = 0; icontainer < uncalibratedMeasurementContainers.size(); ++icontainer) {
      ATH_MSG_DEBUG("Create " << uncalibratedMeasurementContainers[icontainer]->size() << " source links from measurements in " << m_uncalibratedMeasurementContainerKeys[icontainer].key());
      measurements.addMeasurements(icontainer,
                                   *uncalibratedMeasurementContainers[icontainer],
                                   *m_trackingGeometryTool->surfaceIdMap());
      if (measurementIndexContainersSize > 0ul)
        measurementIndex.addMeasurements(*uncalibratedMeasurementContainers[icontainer]);
    }

    ATH_MSG_DEBUG("measurement index size = " << measurementIndex.size());

    ATH_CHECK( propagateDetectorElementStatusToMeasurements(*(volumeIdToDetectorElementCollMap.cptr()), det_el_status_arr, measurements) );

    if (m_trackStatePrinter.isSet()) {
      m_trackStatePrinter->printMeasurements(ctx, uncalibratedMeasurementContainers, measurements.measurementOffsets());
    }

    detail::DuplicateSeedDetector duplicateSeedDetector(total_seeds, m_skipDuplicateSeeds);
    for (std::size_t icontainer = 0; icontainer < seedContainers.size(); ++icontainer)
    {
      duplicateSeedDetector.addSeeds(icontainer, *seedContainers[icontainer], measurementIndex);
    }

    // ================================================== //
    // ===================== CONDS ====================== // 
    // ================================================== //

    std::vector<const InDetDD::SiDetectorElementCollection*> detElementsCollections;
    std::size_t total_detElems = 0;
    ATH_CHECK(getContainersFromKeys(ctx, m_detEleCollKeys, detElementsCollections, total_detElems));

    // ================================================== //
    // ===================== COMPUTATION ================ //
    // ================================================== //
    ActsTrk::MutableTrackContainer tracksContainer;
    EventStats event_stat;
    event_stat.resize(m_stat.size());

    // Perform the track finding for all initial parameters.
    for (std::size_t icontainer = 0; icontainer < seedContainers.size(); ++icontainer)
    {
      ATH_CHECK(findTracks(ctx,
                           measurements,
                           measurementIndex,
                           sharedHits,
                           duplicateSeedDetector,
                           *seedContainers.at(icontainer),
                           *detElementsCollections.at(icontainer),
                           tracksContainer,
                           icontainer,
                           icontainer < m_seedLabels.size() ? m_seedLabels[icontainer].c_str() : m_seedContainerKeys[icontainer].key().c_str(),
                           event_stat));
    }

    ATH_MSG_DEBUG("    \\__ Created " << tracksContainer.size() << " tracks");

    mon_nTracks = tracksContainer.size();

    copyStats(event_stat);

    std::unique_ptr<ActsTrk::TrackContainer> constTracksContainer = m_tracksBackendHandlesHelper.moveToConst(std::move(tracksContainer), 
      m_trackingGeometryTool->getGeometryContext(ctx).context(), ctx);
    // ================================================== //
    // ===================== OUTPUTS ==================== //
    // ================================================== //
    auto trackContainerHandle = SG::makeHandle(m_trackContainerKey, ctx);
    ATH_MSG_DEBUG("    \\__ Tracks Container `" << m_trackContainerKey.key() << "` created ...");

    ATH_CHECK(trackContainerHandle.record(std::move(constTracksContainer)));
    if (!trackContainerHandle.isValid())
    {
      ATH_MSG_FATAL("Failed to write TrackContainer with key " << m_trackContainerKey.key());
      return StatusCode::FAILURE;
    }

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
  TrackFindingAlg::findTracks(const EventContext &ctx,
                              const detail::TrackFindingMeasurements &measurements,
                              const detail::MeasurementIndex &measurementIndex,
                              detail::SharedHitCounter &sharedHits,
                              detail::DuplicateSeedDetector &duplicateSeedDetector,
                              const ActsTrk::SeedContainer &seeds,
                              const InDetDD::SiDetectorElementCollection& detElements,
                              ActsTrk::MutableTrackContainer &tracksContainer,
                              std::size_t typeIndex,
                              const char *seedType,
                              EventStats &event_stat) const
  {
    ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);

    // Construct a perigee surface as the target surface
    auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());

    DetectorContextHolder detContext {
      .geometry = m_trackingGeometryTool->getGeometryContext(ctx).context(),
      .magField = m_extrapolationTool->getMagneticFieldContext(ctx),
      // CalibrationContext converter not implemented yet.
      .calib = getCalibrationContext(ctx)
    };

    auto [options, secondOptions, measurementSelector] = getDefaultOptions(detContext, measurements, pSurface.get());

    // ActsTrk::MutableTrackContainer tracksContainerTemp;
    Acts::VectorTrackContainer trackBackend;
    Acts::VectorMultiTrajectory trackStateBackend;
    detail::RecoTrackContainer tracksContainerTemp(trackBackend, trackStateBackend);

    if (m_addPixelStripCounts) {
      addPixelStripCounts(tracksContainerTemp);
    }

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
    auto printSeed = [&](unsigned int iseed, const Acts::BoundTrackParameters &seedParameters, bool isKF = false)
    {
      if (!m_trackStatePrinter.isSet())
        return;
      if (!nPrinted++)
      {
        ATH_MSG_INFO("CKF results for " << seeds.size() << ' ' << seedType << " seeds:");
      }
      m_trackStatePrinter->printSeed(detContext.geometry, *seeds[iseed], seedParameters, measurementIndex, iseed, isKF);
    };

    // Loop over the track finding results for all initial parameters
    for (unsigned int iseed = 0; iseed < seeds.size(); ++iseed)
    {
      const ActsTrk::Seed& seed = *seeds[iseed];

      category_i = typeIndex * (m_statEtaBins.size() + 1);
      tracksContainerTemp.clear();

      const bool reverseSearch = m_autoReverseSearch && shouldReverseSearch(seed);
      const bool refitSeeds = typeIndex < m_refitSeeds.size() && m_refitSeeds[typeIndex];
      const bool useTopSp = reverseSearch && !refitSeeds;

      auto getSeedCategory = [this, useTopSp](std::size_t typeIndex, const ActsTrk::Seed& seed) -> std::size_t {
        const xAOD::SpacePoint* sp = useTopSp ? seed.sp().back() : seed.sp().front();
        const xAOD::SpacePoint::ConstVectorMap pos = sp->globalPosition();
        double etaSeed = std::atanh(pos[2] / pos.norm());
        return getStatCategory(typeIndex, etaSeed);
      };

      const bool isDupSeed = duplicateSeedDetector.isDuplicate(typeIndex, iseed);
      if (isDupSeed) {
        ATH_MSG_DEBUG("skip " << seedType << " seed " << iseed << " - already found");
        category_i = getSeedCategory(typeIndex, seed);
        ++event_stat[category_i][kNTotalSeeds];
        ++event_stat[category_i][kNDuplicateSeeds];
        if (!m_trackStatePrinter.isSet()) continue;  // delay continue to estimate track parms for TrackStatePrinter?
      }

      options.propagatorPlainOptions.direction = reverseSearch ? Acts::Direction::Backward() : Acts::Direction::Forward();
      secondOptions.propagatorPlainOptions.direction = options.propagatorPlainOptions.direction.invert();
      options.targetSurface = reverseSearch ? pSurface.get() : nullptr;
      secondOptions.targetSurface = reverseSearch ? nullptr : pSurface.get();
      // TODO since the second pass is strictly an extension we should have a separate branch stopper which never drops and always extrapolates to the target surface

      // Estimate Track Parameters
      auto retrieveSurfaceFunction = 
        [this, &detElements] (const ActsTrk::Seed& seed, bool useTopSp) -> const Acts::Surface& { 
          const xAOD::SpacePoint* sp = useTopSp ? seed.sp().back() : seed.sp().front();
          const InDetDD::SiDetectorElement* element = detElements.getDetectorElement(
                useTopSp ? sp->elementIdList().back()
                                : sp->elementIdList().front());
          const Trk::Surface& atlas_surface = element->surface();
          return this->m_ATLASConverterTool->trkSurfaceToActsSurface(atlas_surface);
        };

      std::optional<Acts::BoundTrackParameters> optTrackParams =
        m_paramEstimationTool->estimateTrackParameters(
						       seed,
						       useTopSp,
						       detContext.geometry,
						       detContext.magField,
						       retrieveSurfaceFunction);

      if (!optTrackParams) {
        ATH_MSG_DEBUG("Failed to estimate track parameters for seed " << iseed);
        if (!isDupSeed) {
          category_i = getSeedCategory(typeIndex, seed);
          ++event_stat[category_i][kNTotalSeeds];
          ++event_stat[category_i][kNNoEstimatedParams];
        }
        continue;
      }

      Acts::BoundTrackParameters *initialParameters = &(*optTrackParams);
      printSeed(iseed, *initialParameters);
      if (isDupSeed) continue;  // skip now if not done before

      double etaInitial = -std::log(std::tan(0.5 * initialParameters->theta()));
      category_i = getStatCategory(typeIndex, etaInitial);
      ++event_stat[category_i][kNTotalSeeds];  // also updated for duplicate seeds
      ++event_stat[category_i][kNUsedSeeds];

      std::unique_ptr<Acts::BoundTrackParameters> refitSeedParameters;
      if (refitSeeds) {
        refitSeedParameters = doRefit(seed, *initialParameters, detContext, reverseSearch);
        if (refitSeedParameters.get() == nullptr) {
          ++event_stat[category_i][kNRejectedRefinedSeeds];
          continue;
        }
        if (refitSeedParameters.get() != initialParameters) {
          initialParameters = refitSeedParameters.get();
          printSeed(iseed, *initialParameters, true);
        }
      }

      // Get the Acts tracks, given this seed
      // Result here contains a vector of TrackProxy objects

      auto result = trackFinder().ckf.findTracks(*initialParameters, options, tracksContainerTemp);

      // The result for this seed
      if (not result.ok()) {
        ATH_MSG_WARNING("Track finding failed for " << seedType << " seed " << iseed << " with error" << result.error());
        continue;
      }
      auto &tracksForSeed = result.value();

      size_t ntracks = 0;

      // lambda to collect together all the things we do with a viable track.
      auto addTrack = [&](detail::RecoTrackContainerProxy &track) {
        // if the the perigeeSurface was not hit (in particular the case for the inside-out pass,
        // the track has no reference surface and the extrapolation to the perigee has not been done
        // yet.
        if (!track.hasReferenceSurface()) {
           auto extrapolationResult = Acts::extrapolateTrackToReferenceSurface(
                   track, *pSurface, trackFinder().extrapolator, extrapolationOptions,
                   extrapolationStrategy, logger());
           if (!extrapolationResult.ok()) {
              ATH_MSG_WARNING("Extrapolation for seed "
                              << iseed << " and " << track.index()
                              << " failed with error " << extrapolationResult.error()
                              << " dropping track candidate.");
              return;
           }
        }

        Acts::trimTrack(track, true, true, true, true);
        Acts::calculateTrackQuantities(track);
        if (m_addPixelStripCounts) {
          initPixelStripCounts(track);
          for (const auto trackState : track.trackStatesReversed()) {
            updatePixelStripCounts(track, trackState.typeFlags(), measurementType(trackState));
          }
          checkPixelStripCounts(track);
        }

        ++ntracks;
        ++event_stat[category_i][kNOutputTracks];

        auto selectPixelStripCountsFinal = [this](const detail::RecoTrackContainer::TrackProxy &track) {
          if (!m_addPixelStripCounts) return true;
          double eta = -std::log(std::tan(0.5 * track.theta()));
          auto [enoughMeasurementsPS, tooManyHolesPS, tooManyOutliersPS] = selectPixelStripCounts(track, eta);
          return enoughMeasurementsPS && !tooManyHolesPS && !tooManyOutliersPS;
        };
        if (trackFinder().trackSelector.isValidTrack(track) &&
            selectPixelStripCountsFinal(track)) {

          // Fill the track infos into the duplicate seed detector
          if (m_skipDuplicateSeeds) {
            storeSeedInfo(tracksContainerTemp, track, duplicateSeedDetector, measurementIndex);
          }

          // copy selected track into output tracksContainer
          auto destProxy = tracksContainer.getTrack(tracksContainer.addTrack());
          destProxy.copyFrom(track, true);  // make sure we copy track states!

          if (m_countSharedHits) {
            auto [nShared, nBadTrackMeasurements] = sharedHits.computeSharedHits(destProxy, tracksContainer, measurementIndex);
            if (nBadTrackMeasurements > 0)
              ATH_MSG_ERROR("computeSharedHits: " << nBadTrackMeasurements << " track measurements not found in input for " << seedType << " seed " << iseed << " track");
            ATH_MSG_DEBUG("found " << destProxy.nSharedHits() << " shared hits in " << seedType << " seed " << iseed << " track");
            event_stat[category_i][kNTotalSharedHits] += nShared;
          }

          ++event_stat[category_i][kNSelectedTracks];

          if (m_trackStatePrinter.isSet()) {
            m_trackStatePrinter->printTrack(detContext.geometry, tracksContainer, destProxy, measurementIndex);
          }

        } else {
          ATH_MSG_DEBUG("Track " << ntracks << " from " << seedType << " seed " << iseed << " failed track selection");
          if (m_trackStatePrinter.isSet()) {
            m_trackStatePrinter->printTrack(detContext.geometry, tracksContainerTemp, track, measurementIndex, true);
          }
        }
      };

      std::size_t nfirst = 0;
      for (TrkProxy &firstTrack : tracksForSeed) {
        auto smoothingResult = Acts::smoothTrack(detContext.geometry, firstTrack, logger(), Acts::MbfSmoother());
        if (!smoothingResult.ok()) {
          ATH_MSG_DEBUG("Smoothing for seed "
                     << iseed << " and first track " << firstTrack.index()
                     << " failed with error " << smoothingResult.error());
          continue;
        }

        const std::size_t nsecond =
            m_doTwoWay ? doTwoWayTrackFinding(addTrack, firstTrack, tracksContainerTemp, secondOptions, detContext.geometry, reverseSearch)
                       : 0;

        if (nsecond == 0) {
          if (m_doTwoWay) {
            ATH_MSG_DEBUG("No viable result from second track finding for " << seedType << " seed " << iseed << " track " << nfirst);
            ++event_stat[category_i][kNoSecond];
          }

          addTrack(firstTrack);
        }
        nfirst++;
      }
      if (ntracks == 0) {
        ATH_MSG_DEBUG("Track finding found no track candidates for " << seedType << " seed " << iseed);
        ++event_stat[category_i][kNoTrack];
      } else if (ntracks >= 2) {
        ++event_stat[category_i][kMultipleBranches];
      }
      if (m_trackStatePrinter.isSet())
        std::cout << std::flush;
    }

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
                 const ActsDetectorElement *acts_detector_element = dynamic_cast<const ActsDetectorElement*>(detector_element);
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
} // namespace
