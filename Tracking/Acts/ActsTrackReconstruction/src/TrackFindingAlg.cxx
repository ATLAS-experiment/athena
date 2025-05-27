/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "src/TrackFindingAlg.h"

// Athena
#include "AsgTools/ToolStore.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkTrackSummary/TrackSummary.h"
#include "InDetPrepRawData/PixelClusterCollection.h"
#include "InDetPrepRawData/SCT_ClusterCollection.h"
#include "HGTD_PrepRawData/HGTD_ClusterCollection.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "InDetRIO_OnTrack/PixelClusterOnTrack.h"
#include "InDetRIO_OnTrack/SCT_ClusterOnTrack.h"
#include "HGTD_RIO_OnTrack/HGTD_ClusterOnTrack.h"

// ACTS
#include "Acts/Definitions/Units.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Utilities/TrackHelpers.hpp"
#include "Acts/TrackFitting/MbfSmoother.hpp"

// ActsTrk
#include "ActsCalibration/CalibrationContext.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsInterop/Logger.h"
#include "ActsInterop/TableUtils.h"
#include "src/detail/AtlasMeasurementSelector.h"
#include "src/detail/TrackFindingMeasurements.h"
#include "src/detail/SharedHitCounter.h"
#include "src/detail/FitterHelperFunctions.h"

// STL
#include <sstream>
#include <functional>
#include <tuple>
#include <utility>
#include <algorithm>

namespace ActsTrk
{
  struct TrackFindingAlg::CKF_pimpl : public detail::CKF_config
  {
  };

  TrackFindingAlg::CKF_pimpl &TrackFindingAlg::trackFinder() { return *m_trackFinder; }
  const TrackFindingAlg::CKF_pimpl &TrackFindingAlg::trackFinder() const { return *m_trackFinder; }

  TrackFindingAlg::TrackFindingAlg(const std::string &name,
                                   ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator)
  {
  }

  TrackFindingAlg::~TrackFindingAlg() = default;

  // === initialize ==========================================================

  StatusCode TrackFindingAlg::initialize()
  {
    ATH_MSG_INFO("Initializing " << name() << " ... ");
    ATH_MSG_DEBUG("Properties Summary:");
    ATH_MSG_DEBUG("   " << m_maxPropagationStep);
    ATH_MSG_DEBUG("   " << m_skipDuplicateSeeds);
    ATH_MSG_DEBUG("   " << m_refitSeeds);
    ATH_MSG_DEBUG("   " << m_etaBins);
    ATH_MSG_DEBUG("   " << m_chi2CutOff);
    ATH_MSG_DEBUG("   " << m_chi2OutlierCutOff);
    ATH_MSG_DEBUG("   " << m_numMeasurementsCutOff);
    ATH_MSG_DEBUG("   " << m_ptMinMeasurements);
    ATH_MSG_DEBUG("   " << m_absEtaMaxMeasurements);
    ATH_MSG_DEBUG("   " << m_doBranchStopper);
    ATH_MSG_DEBUG("   " << m_addPixelStripCounts);
    ATH_MSG_DEBUG("   " << m_doTwoWay);
    ATH_MSG_DEBUG("   " << m_autoReverseSearch);
    ATH_MSG_DEBUG("   " << m_useTopSpRZboundary);
    ATH_MSG_DEBUG("   " << m_countSharedHits);
    ATH_MSG_DEBUG("   " << m_phiMin);
    ATH_MSG_DEBUG("   " << m_phiMax);
    ATH_MSG_DEBUG("   " << m_etaMin);
    ATH_MSG_DEBUG("   " << m_etaMax);
    ATH_MSG_DEBUG("   " << m_absEtaMin);
    ATH_MSG_DEBUG("   " << m_absEtaMax);
    ATH_MSG_DEBUG("   " << m_ptMin);
    ATH_MSG_DEBUG("   " << m_ptMax);
    ATH_MSG_DEBUG("   " << m_z0Min);
    ATH_MSG_DEBUG("   " << m_z0Max);
    ATH_MSG_DEBUG("   " << m_minMeasurements);
    ATH_MSG_DEBUG("   " << m_minPixelHits);
    ATH_MSG_DEBUG("   " << m_minStripHits);
    ATH_MSG_DEBUG("   " << m_maxHoles);
    ATH_MSG_DEBUG("   " << m_maxPixelHoles);
    ATH_MSG_DEBUG("   " << m_maxStripHoles);
    ATH_MSG_DEBUG("   " << m_maxOutliers);
    ATH_MSG_DEBUG("   " << m_maxPixelOutliers);
    ATH_MSG_DEBUG("   " << m_maxStripOutliers);
    ATH_MSG_DEBUG("   " << m_maxSharedHits);
    ATH_MSG_DEBUG("   " << m_maxChi2);
    ATH_MSG_DEBUG("   " << m_statEtaBins);
    ATH_MSG_DEBUG("   " << m_seedLabels);
    ATH_MSG_DEBUG("   " << m_dumpAllStatEtaBins);
    ATH_MSG_DEBUG("   " << m_branchStopperPtMinFactor);
    ATH_MSG_DEBUG("   " << m_branchStopperAbsEtaMaxExtra);

    // Read and Write handles
    ATH_CHECK(m_seedContainerKeys.initialize());
    ATH_CHECK(m_detEleCollKeys.initialize());
    ATH_CHECK(m_uncalibratedMeasurementContainerKeys.initialize());
    ATH_CHECK(m_volumeIdToDetectorElementCollMapKey.initialize());
    ATH_CHECK(m_detElStatus.initialize());

    ATH_CHECK(m_trackContainerKey.initialize());
    ATH_CHECK(m_tracksBackendHandlesHelper.initialize(ActsTrk::prefixFromTrackContainerName(m_trackContainerKey.key())));

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

    ATH_CHECK(m_monTool.retrieve(EnableTool{not m_monTool.empty()}));
    ATH_CHECK(m_trackingGeometryTool.retrieve());
    ATH_CHECK(m_extrapolationTool.retrieve());
    ATH_CHECK(m_trackStatePrinter.retrieve(EnableTool{not m_trackStatePrinter.empty()}));
    ATH_CHECK(m_ATLASConverterTool.retrieve());
    ATH_CHECK(m_paramEstimationTool.retrieve());
    ATH_CHECK(m_fitterTool.retrieve());
    ATH_CHECK(m_pixelCalibTool.retrieve(EnableTool{not m_pixelCalibTool.empty()}));
    ATH_CHECK(m_stripCalibTool.retrieve(EnableTool{not m_stripCalibTool.empty()}));
    ATH_CHECK(m_hgtdCalibTool.retrieve(EnableTool{not m_hgtdCalibTool.empty()}));

    m_logger = makeActsAthenaLogger(this, "Acts");

    auto magneticField = std::make_unique<ATLASMagneticFieldWrapper>();
    auto trackingGeometry = m_trackingGeometryTool->trackingGeometry();

    detail::Stepper stepper(std::move(magneticField));
    detail::Navigator::Config config{trackingGeometry};
    config.resolvePassive = false;
    config.resolveMaterial = true;
    config.resolveSensitive = true;
    detail::Navigator navigator(config, logger().cloneWithSuffix("Navigator"));
    detail::Propagator propagator(std::move(stepper), std::move(navigator), logger().cloneWithSuffix("Prop"));

    // Using the CKF propagator as extrapolator
    detail::Extrapolator extrapolator = propagator;

    // m_etaBins (from flags.Tracking.ActiveConfig.etaBins) includes a dummy first and last bin, which we ignore
    std::vector<double> absEtaEdges;
    if (m_etaBins.size() <= 2)
    {
      absEtaEdges.reserve(2ul);
      absEtaEdges.push_back(0.0);
      absEtaEdges.push_back(std::numeric_limits<double>::infinity());
    }
    else
    {
      absEtaEdges.reserve(m_etaBins.size());
      absEtaEdges.push_back(m_absEtaMin);
      absEtaEdges.insert(absEtaEdges.end(), m_etaBins.begin() + 1, m_etaBins.end() - 1);
      absEtaEdges.push_back(m_absEtaMax);
    }

    auto setCut = [](auto &cfgVal, const auto &cuts, size_t ind) -> void
    {
      if (cuts.empty())
        return;
      cfgVal = (ind < cuts.size()) ? cuts[ind] : cuts[cuts.size() - 1];
    };

    Acts::TrackSelector::EtaBinnedConfig trackSelectorCfg{std::move(absEtaEdges)};
    if (m_etaBins.size() <= 2)
    {
      assert(trackSelectorCfg.cutSets.size() == 1);
      trackSelectorCfg.cutSets[0].absEtaMin = m_absEtaMin;
      trackSelectorCfg.cutSets[0].absEtaMax = m_absEtaMax;
    }
    size_t cutIndex = 0;
    for (auto &cfg : trackSelectorCfg.cutSets)
    {
      setCut(cfg.phiMin, m_phiMin, cutIndex);
      setCut(cfg.phiMax, m_phiMax, cutIndex);
      setCut(cfg.etaMin, m_etaMin, cutIndex);
      setCut(cfg.etaMax, m_etaMax, cutIndex);
      setCut(cfg.ptMin, m_ptMin, cutIndex);
      setCut(cfg.ptMax, m_ptMax, cutIndex);
      setCut(cfg.loc0Min, m_d0Min, cutIndex);
      setCut(cfg.loc0Max, m_d0Max, cutIndex);
      setCut(cfg.loc1Min, m_z0Min, cutIndex);
      setCut(cfg.loc1Max, m_z0Max, cutIndex);
      setCut(cfg.minMeasurements, m_minMeasurements, cutIndex);
      setCut(cfg.maxHoles, m_maxHoles, cutIndex);
      setCut(cfg.maxOutliers, m_maxOutliers, cutIndex);
      setCut(cfg.maxSharedHits, m_maxSharedHits, cutIndex);
      setCut(cfg.maxChi2, m_maxChi2, cutIndex);
      ++cutIndex;
    }

    ATH_MSG_INFO(trackSelectorCfg);

    // initializer measurement selector and connect it to the delegates of the track finder optins
    ATH_CHECK( initializeMeasurementSelector());

    detail::CKF_config ckfConfig{
        std::move(extrapolator),
        detail::CKF{std::move(propagator), logger().cloneWithSuffix("CKF")},
        {},
        Acts::TrackSelector{trackSelectorCfg}};

    m_trackFinder = std::make_unique<CKF_pimpl>(std::move(ckfConfig));

    trackFinder().ckfExtensions.updater.connect<&ActsTrk::detail::FitterHelperFunctions::gainMatrixUpdate<detail::RecoTrackStateContainer>>();
    initStatTables();

    return StatusCode::SUCCESS;
  }

  // === finalize ============================================================

  StatusCode TrackFindingAlg::finalize()
  {
    printStatTables();
    return StatusCode::SUCCESS;
  }

  // === execute =============================================================

  StatusCode TrackFindingAlg::execute(const EventContext &ctx) const
  {
    ATH_MSG_DEBUG("Executing " << name() << " ... ");

    auto timer = Monitored::Timer<std::chrono::milliseconds>("TIME_execute");
    auto mon_nTracks = Monitored::Scalar<int>("nTracks");
    auto mon = Monitored::Group(m_monTool, timer, mon_nTracks);
    const auto* detectorElementToGeometryIdMap =  m_trackingGeometryTool->surfaceIdMap();


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
                                   *detectorElementToGeometryIdMap);
      if (measurementIndexContainersSize > 0ul)
        measurementIndex.addMeasurements(*uncalibratedMeasurementContainers[icontainer]);
    }

    ATH_MSG_DEBUG("measurement index size = " << measurementIndex.size());

    ATH_CHECK( propagateDetectorElementStatusToMeasurements(*(volumeIdToDetectorElementCollMap.cptr()), det_el_status_arr, measurements) );

    if (m_trackStatePrinter.isSet()) {
      m_trackStatePrinter->printMeasurements(ctx, uncalibratedMeasurementContainers, *detectorElementToGeometryIdMap, measurements.measurementOffsets());
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
                           *detectorElementToGeometryIdMap,
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

  std::unique_ptr<ActsTrk::IMeasurementSelector> TrackFindingAlg::setMeasurementSelector(
      const detail::TrackFindingMeasurements &measurements,
      TrackFinderOptions &options) const {
    ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);

    std::unique_ptr<ActsTrk::IMeasurementSelector> measurementSelector = ActsTrk::detail::getMeasurementSelector(
        m_pixelCalibTool.isEnabled() ? &(*m_pixelCalibTool) : nullptr,
        measurements.measurementRanges(),
        m_measurementSelectorConfig.m_etaBins,
        m_measurementSelectorConfig.m_chi2CutOffOutlier,
        m_numMeasurementsCutOff.value());

    measurementSelector->connect(&options.extensions.createTrackStates);

    return measurementSelector;
  }

  // === findTracks ==========================================================

  StatusCode
  TrackFindingAlg::findTracks(const EventContext &ctx,
                              const ActsTrk::DetectorElementToActsGeometryIdMap &detectorElementToGeoId,
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
        refitSeedParameters = doRefit(ctx, seed, *initialParameters, detContext, detectorElementToGeoId, reverseSearch);
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

  TrackFindingAlg::TrackFindingDefaultOptions TrackFindingAlg::getDefaultOptions(
      const DetectorContextHolder &detContext,
      const detail::TrackFindingMeasurements &measurements,
      const Acts::PerigeeSurface* pSurface) const {
    Acts::PropagatorPlainOptions plainOptions{detContext.geometry, detContext.magField};
    Acts::PropagatorPlainOptions plainSecondOptions{detContext.geometry, detContext.magField};

    plainOptions.maxSteps = m_maxPropagationStep;
    plainOptions.direction = Acts::Direction::Forward();
    plainOptions.endOfWorldVolumeIds = m_endOfWorldVolumeIds;
    plainSecondOptions.maxSteps = m_maxPropagationStep;
    plainSecondOptions.direction = plainOptions.direction.invert();

    // Set the CombinatorialKalmanFilter options
    TrackFinderOptions options(detContext.geometry, detContext.magField, detContext.calib,
                               trackFinder().ckfExtensions, plainOptions, pSurface);

    std::unique_ptr<ActsTrk::IMeasurementSelector> measurementSelector = setMeasurementSelector(measurements, options);

    TrackFinderOptions secondOptions(detContext.geometry, detContext.magField, detContext.calib,
                                     options.extensions, plainSecondOptions, pSurface);
    secondOptions.targetSurface = pSurface;
    secondOptions.skipPrePropagationUpdate = true;

    return {options, secondOptions, std::move(measurementSelector)};
  };

  TrackFindingAlg::BranchStopperResult TrackFindingAlg::stopBranch(
      const detail::RecoTrackContainer::TrackProxy &track,
      const detail::RecoTrackContainer::TrackStateProxy &trackState,
      const Acts::TrackSelector::EtaBinnedConfig &trackSelectorCfg,
      const Acts::GeometryContext &tgContext,
      const detail::MeasurementIndex &measurementIndex,
      const std::size_t typeIndex,
      EventStats::value_type &event_stat_category_i) const {
    if (m_addPixelStripCounts) {
      updatePixelStripCounts(track, trackState.typeFlags(),
                             measurementType(trackState));
      checkPixelStripCounts(track);
    }

    if (m_trackStatePrinter.isSet()) {
      m_trackStatePrinter->printTrackState(tgContext, trackState,
                                           measurementIndex, true);
    }

    if (!m_doBranchStopper) {
      return BranchStopperResult::Continue;
    }

    const auto &parameters = trackState.hasFiltered() ? trackState.filtered()
                                                      : trackState.predicted();
    double eta = -std::log(std::tan(0.5 * parameters[Acts::eBoundTheta]));
    const auto &cutSet = getCuts(eta);

    if (typeIndex < m_ptMinMeasurements.size() &&
        !(track.nMeasurements() < m_ptMinMeasurements[typeIndex])) {
      double pT = std::sin(parameters[Acts::eBoundTheta]) /
                  parameters[Acts::eBoundQOverP];
      if (std::abs(pT) < cutSet.ptMin * m_branchStopperPtMinFactor) {
        ++event_stat_category_i[kNStoppedTracksMinPt];
        ATH_MSG_DEBUG("CkfBranchStopper: drop branch with q*pT="
                      << pT << " after " << track.nMeasurements()
                      << " measurements");
        return BranchStopperResult::StopAndDrop;
      }
    }

    if (typeIndex < m_absEtaMaxMeasurements.size() &&
        !(track.nMeasurements() < m_absEtaMaxMeasurements[typeIndex]) &&
        !(std::abs(eta) < trackSelectorCfg.absEtaEdges.back() +
                              m_branchStopperAbsEtaMaxExtra)) {
      ++event_stat_category_i[kNStoppedTracksMaxEta];
      ATH_MSG_DEBUG("CkfBranchStopper: drop branch with eta="
                    << eta << " after " << track.nMeasurements()
                    << " measurements");
      return BranchStopperResult::StopAndDrop;
    }

    bool enoughMeasurements = (track.nMeasurements() >= cutSet.minMeasurements);
    bool tooManyHoles = (track.nHoles() > cutSet.maxHoles);
    bool tooManyOutliers = (track.nOutliers() > cutSet.maxOutliers);

    if (m_addPixelStripCounts) {
      auto [enoughMeasurementsPS, tooManyHolesPS, tooManyOutliersPS] =
          selectPixelStripCounts(track, eta);
      enoughMeasurements = enoughMeasurements && enoughMeasurementsPS;
      tooManyHoles = tooManyHoles || tooManyHolesPS;
      tooManyOutliers = tooManyOutliers || tooManyOutliersPS;
    }

    if (!(tooManyHoles || tooManyOutliers)) {
      return BranchStopperResult::Continue;
    }

    if (!enoughMeasurements) {
      ++event_stat_category_i[kNStoppedTracksMaxHoles];
    }

    if (m_addPixelStripCounts) {
      ATH_MSG_DEBUG("CkfBranchStopper: stop and "
                    << (enoughMeasurements ? "keep" : "drop")
                    << " branch with nHoles=" << track.nHoles() << " ("
                    << s_branchState.nPixelHoles(track) << " pixel+"
                    << s_branchState.nStripHoles(track)
                    << " strip), nOutliers=" << track.nOutliers() << " ("
                    << s_branchState.nPixelOutliers(track) << "+"
                    << s_branchState.nStripOutliers(track)
                    << "), nMeasurements=" << track.nMeasurements() << " ("
                    << s_branchState.nPixelHits(track) << "+"
                    << s_branchState.nStripHits(track) << ")");
    } else {
      ATH_MSG_DEBUG("CkfBranchStopper: stop and "
                    << (enoughMeasurements ? "keep" : "drop")
                    << " branch with nHoles=" << track.nHoles()
                    << ", nOutliers=" << track.nOutliers()
                    << ", nMeasurements=" << track.nMeasurements());
    }

    return enoughMeasurements ? BranchStopperResult::StopAndKeep
                              : BranchStopperResult::StopAndDrop;
  }

  std::size_t TrackFindingAlg::doTwoWayTrackFinding(
      std::function<void(detail::RecoTrackContainerProxy &)> addTrack,
      TrkProxy &trackProxy,
      detail::RecoTrackContainer &tracksContainerTemp,
      const TrackFinderOptions &options,
      Acts::GeometryContext &tgContext,
      const bool reverseSearch) const {
    std::size_t count = 0;

    std::optional<detail::RecoTrackStateContainerProxy> firstMeasurement;
    for (auto st : trackProxy.trackStatesReversed()) {
      bool isMeasurement = st.typeFlags().test(Acts::TrackStateFlag::MeasurementFlag);
      bool isOutlier = st.typeFlags().test(Acts::TrackStateFlag::OutlierFlag);
      // We are excluding non measurement states and outlier here. Those can
      // decrease resolution because only the smoothing corrected the very
      // first prediction as filtering is not possible.
      if (isMeasurement && !isOutlier) {
        firstMeasurement = st;
      }
    }

    if (!firstMeasurement.has_value()) {
      return 0;
    }

    Acts::BoundTrackParameters secondInitialParameters = trackProxy.createParametersFromState(detail::RecoConstTrackStateContainerProxy{*firstMeasurement});

    if (!secondInitialParameters.referenceSurface().insideBounds(secondInitialParameters.localPosition())) {  // #3751
      return 0;
    }

    auto rootBranch = tracksContainerTemp.makeTrack();
    rootBranch.copyFrom(trackProxy, false);  // #3534
    if (m_addPixelStripCounts) {
      copyPixelStripCounts(rootBranch, trackProxy);
    }
    auto secondResult = trackFinder().ckf.findTracks(secondInitialParameters, options, tracksContainerTemp, rootBranch);

    if (not secondResult.ok()) {
      return 0;
    }

    // store the original previous state to restore it later
    auto originalFirstMeasurementPrevious = firstMeasurement->previous();

    auto &secondTracksForSeed = secondResult.value();
    for (auto &secondTrack : secondTracksForSeed) {
      secondTrack.reverseTrackStates(true);

      firstMeasurement->previous() = secondTrack.outermostTrackState().index();
      secondTrack.tipIndex() = trackProxy.tipIndex();

      if (reverseSearch) {
        // smooth the full track
        auto secondSmoothingResult = Acts::smoothTrack(tgContext, secondTrack, logger());
        if (!secondSmoothingResult.ok()) {
          continue;
        }

        secondTrack.reverseTrackStates(true);
      }

      addTrack(secondTrack);

      ++count;
    }

    // restore the original previous state for the first track
    firstMeasurement->previous() = originalFirstMeasurementPrevious;

    return count;
  };


  const Acts::TrackSelector::Config&
  TrackFindingAlg::getCuts (double eta) const {
    const auto &trackSelectorCfg = trackFinder().trackSelector.config();
    // return the last bin for |eta|>=4 or nan
    return (!(std::abs(eta) < trackSelectorCfg.absEtaEdges.back())) ? trackSelectorCfg.cutSets.back()
           : (std::abs(eta) < trackSelectorCfg.absEtaEdges.front()) ? trackSelectorCfg.cutSets.front()
                                                                    : trackSelectorCfg.getCuts(eta);
  };


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

  xAOD::UncalibMeasType
  TrackFindingAlg::measurementType (const detail::RecoTrackContainer::TrackStateProxy &trackState)
  {
    if (trackState.hasReferenceSurface()) {
      if (const auto *actsDetElem = dynamic_cast<const ActsDetectorElement *>(trackState.referenceSurface().associatedDetectorElement())) {
        if (const auto *detElem = dynamic_cast<const InDetDD::SiDetectorElement *>(actsDetElem->upstreamDetectorElement())) {
          if (detElem->isPixel()) {
            return xAOD::UncalibMeasType::PixelClusterType;
          } else if (detElem->isSCT()) {
            return xAOD::UncalibMeasType::StripClusterType;
          }
        }
      }
    }
    return xAOD::UncalibMeasType::Other;
  }

  void
  TrackFindingAlg::addPixelStripCounts(detail::RecoTrackContainer& tracksContainer)
  {
    tracksContainer.addColumn<unsigned int>("nPixelHits");
    tracksContainer.addColumn<unsigned int>("nStripHits");
    tracksContainer.addColumn<unsigned int>("nPixelHoles");
    tracksContainer.addColumn<unsigned int>("nStripHoles");
    tracksContainer.addColumn<unsigned int>("nPixelOutliers");
    tracksContainer.addColumn<unsigned int>("nStripOutliers");
  }

  void
  TrackFindingAlg::initPixelStripCounts(const detail::RecoTrackContainer::TrackProxy &track)
  {
    s_branchState.nPixelHits(track) = 0;
    s_branchState.nStripHits(track) = 0;
    s_branchState.nPixelHoles(track) = 0;
    s_branchState.nStripHoles(track) = 0;
    s_branchState.nPixelOutliers(track) = 0;
    s_branchState.nStripOutliers(track) = 0;
  }

  void
  TrackFindingAlg::updatePixelStripCounts(const detail::RecoTrackContainer::TrackProxy &track,
                                          Acts::ConstTrackStateType typeFlags,
                                          xAOD::UncalibMeasType detType)
  {
    if (detType == xAOD::UncalibMeasType::PixelClusterType) {
      if (typeFlags.test(Acts::TrackStateFlag::HoleFlag)) {
        s_branchState.nPixelHoles(track)++;
      } else if (typeFlags.test(Acts::TrackStateFlag::OutlierFlag)) {
        s_branchState.nPixelOutliers(track)++;
      } else if (typeFlags.test(Acts::TrackStateFlag::MeasurementFlag)) {
        s_branchState.nPixelHits(track)++;
      }
    } else if (detType == xAOD::UncalibMeasType::StripClusterType) {
      if (typeFlags.test(Acts::TrackStateFlag::HoleFlag)) {
        s_branchState.nStripHoles(track)++;
      } else if (typeFlags.test(Acts::TrackStateFlag::OutlierFlag)) {
        s_branchState.nStripOutliers(track)++;
      } else if (typeFlags.test(Acts::TrackStateFlag::MeasurementFlag)) {
        s_branchState.nStripHits(track)++;
      }
    }
  }

  void
  TrackFindingAlg::copyPixelStripCounts(const detail::RecoTrackContainer::TrackProxy &track,
                                        const detail::RecoTrackContainer::TrackProxy &other)
  {
    s_branchState.nPixelHits(track) = s_branchState.nPixelHits(other);
    s_branchState.nStripHits(track) = s_branchState.nStripHits(other);
    s_branchState.nPixelHoles(track) = s_branchState.nPixelHoles(other);
    s_branchState.nStripHoles(track) = s_branchState.nStripHoles(other);
    s_branchState.nPixelOutliers(track) = s_branchState.nPixelOutliers(other);
    s_branchState.nStripOutliers(track) = s_branchState.nStripOutliers(other);
  }

  void
  TrackFindingAlg::checkPixelStripCounts(const detail::RecoTrackContainer::TrackProxy &track) const
  {
    // This check will fail if there are other types (HGTD, MS?) of hits, holes, or outliers.
    // The check can be removed when it is no longer appropriate.
    if (track.nMeasurements() != s_branchState.nPixelHits(track) + s_branchState.nStripHits(track))
      ATH_MSG_WARNING("mismatched hit count: total (" << track.nMeasurements()
                      << ") != pixel (" << s_branchState.nPixelHits(track)
                      << ") + strip (" << s_branchState.nStripHits(track) << ")");
    if (track.nHoles() < s_branchState.nPixelHoles(track) + s_branchState.nStripHoles(track))  // allow extra HGTD holes
      ATH_MSG_WARNING("mismatched hole count: total (" << track.nHoles()
                      << ") < pixel (" << s_branchState.nPixelHoles(track)
                      << ") + strip (" << s_branchState.nStripHoles(track) << ")");
    if (track.nOutliers() != s_branchState.nPixelOutliers(track) + s_branchState.nStripOutliers(track))
      ATH_MSG_WARNING("mismatched outlier count: total (" << track.nOutliers()
                      << ") != pixel (" << s_branchState.nPixelOutliers(track)
                      << ") + strip (" << s_branchState.nStripOutliers(track) << ")");
  };

  std::array<bool, 3>
  TrackFindingAlg::selectPixelStripCounts(const detail::RecoTrackContainer::TrackProxy &track, double eta) const
  {
    bool enoughMeasurements = true, tooManyHoles = false, tooManyOutliers = false;
    const auto &trackSelectorCfg = trackFinder().trackSelector.config();
    std::size_t etaBin = (std::abs(eta) < trackSelectorCfg.absEtaEdges.front())   ? 0
                         : (std::abs(eta) >= trackSelectorCfg.absEtaEdges.back()) ? trackSelectorCfg.absEtaEdges.size() - 1
                                                                                  : trackSelectorCfg.binIndex(eta);
    auto cutMin = [etaBin](std::size_t val, const std::vector<std::size_t> &cutSet) {
      return !cutSet.empty() && (val < (etaBin < cutSet.size() ? cutSet[etaBin] : cutSet.back()));
    };
    auto cutMax = [etaBin](std::size_t val, const std::vector<std::size_t> &cutSet) {
      return !cutSet.empty() && (val > (etaBin < cutSet.size() ? cutSet[etaBin] : cutSet.back()));
    };
    enoughMeasurements = enoughMeasurements && !cutMin(s_branchState.nPixelHits(track), m_minPixelHits);
    enoughMeasurements = enoughMeasurements && !cutMin(s_branchState.nStripHits(track), m_minStripHits);
    tooManyHoles = tooManyHoles || cutMax(s_branchState.nPixelHoles(track), m_maxPixelHoles);
    tooManyHoles = tooManyHoles || cutMax(s_branchState.nStripHoles(track), m_maxStripHoles);
    tooManyOutliers = tooManyOutliers || cutMax(s_branchState.nPixelOutliers(track), m_maxPixelOutliers);
    tooManyOutliers = tooManyOutliers || cutMax(s_branchState.nStripOutliers(track), m_maxStripOutliers);
    return {enoughMeasurements, tooManyHoles, tooManyOutliers};
  }

  // === Statistics printout =================================================

  void TrackFindingAlg::initStatTables()
  {
    if (!m_statEtaBins.empty())
    {
      m_useAbsEtaForStat = (m_statEtaBins[0] > 0.);
      float last_eta = m_statEtaBins[0];
      for (float eta : m_statEtaBins)
      {
        if (eta < last_eta)
        {
          ATH_MSG_FATAL("Eta bins for statistics counter not in ascending order.");
        }
        last_eta = eta;
      }
    }
    m_stat.resize(nSeedCollections() * seedCollectionStride());
  }

  // copy statistics
  void TrackFindingAlg::copyStats(const EventStats &event_stat) const
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::size_t category_i = 0;
    for (const std::array<unsigned int, kNStat> &src_stat : event_stat)
    {
      std::array<std::size_t, kNStat> &dest_stat = m_stat[category_i++];
      for (std::size_t i = 0; i < src_stat.size(); ++i)
      {
        assert(i < dest_stat.size());
        dest_stat[i] += src_stat[i];
      }
    }
  }

  // print statistics
  void TrackFindingAlg::printStatTables() const
  {
    if (msgLvl(MSG::INFO))
    {
      std::vector<std::string> stat_labels =
          TableUtils::makeLabelVector(kNStat,
                                      {
                                          std::make_pair(kNTotalSeeds, "Input seeds"),
                                          std::make_pair(kNoTrackParam, "No track parameters"),
                                          std::make_pair(kNUsedSeeds, "Used   seeds"),
                                          std::make_pair(kNoTrack, "Cannot find track"),
                                          std::make_pair(kNDuplicateSeeds, "Duplicate seeds"),
                                          std::make_pair(kNNoEstimatedParams, "Initial param estimation failed"),
                                          std::make_pair(kNRejectedRefinedSeeds, "Rejected refined parameters"),
                                          std::make_pair(kNOutputTracks, "CKF tracks"),
                                          std::make_pair(kNSelectedTracks, "selected tracks"),
                                          std::make_pair(kNStoppedTracksMaxHoles, "Stopped tracks reaching max holes"),
                                          std::make_pair(kMultipleBranches, "Seeds with more than one branch"),
                                          std::make_pair(kNoSecond, "Tracks failing second CKF"),
                                          std::make_pair(kNStoppedTracksMinPt, "Stopped tracks below pT cut"),
                                          std::make_pair(kNStoppedTracksMaxEta, "Stopped tracks above max eta"),
                                          std::make_pair(kNTotalSharedHits, "Total shared hits")
                                      });
      assert(stat_labels.size() == kNStat);
      std::vector<std::string> categories;
      categories.reserve(m_seedLabels.size() + 1);
      categories.insert(categories.end(), m_seedLabels.begin(), m_seedLabels.end());
      categories.push_back("ALL");

      std::vector<std::string> eta_labels;
      eta_labels.reserve(m_statEtaBins.size() + 2);
      for (std::size_t eta_bin_i = 0; eta_bin_i < m_statEtaBins.size() + 2; ++eta_bin_i)
      {
        eta_labels.push_back(TableUtils::makeEtaBinLabel(m_statEtaBins, eta_bin_i, m_useAbsEtaForStat));
      }

      // vector used as 3D array stat[ eta_bin ][ stat_i ][ seed_type]
      // stat_i = [0, kNStat)
      // eta_bin = [0, m_statEtaBins.size()+2 ); eta_bin == m_statEtaBinsSize()+1 means sum of all etaBins
      // seed_type = [0, nSeedCollections()+1)  seed_type == nSeedCollections() means sum of all seed collections
      std::vector<std::size_t> stat =
          TableUtils::createCounterArrayWithProjections<std::size_t>(nSeedCollections(),
                                                                     m_statEtaBins.size() + 1,
                                                                     m_stat);

      // the extra columns and rows for the projections are addeded internally:
      std::size_t stat_stride =
          TableUtils::counterStride(nSeedCollections(),
                                    m_statEtaBins.size() + 1,
                                    kNStat);
      std::size_t eta_stride =
          TableUtils::subCategoryStride(nSeedCollections(),
                                        m_statEtaBins.size() + 1,
                                        kNStat);
      std::stringstream table_out;

      if (m_dumpAllStatEtaBins.value())
      {
        // dump for each counter a table with one row per eta bin
        std::size_t max_label_width = TableUtils::maxLabelWidth(stat_labels) + TableUtils::maxLabelWidth(eta_labels);
        for (std::size_t stat_i = 0; stat_i < kNStat; ++stat_i)
        {
          std::size_t dest_idx_offset = stat_i * stat_stride;
          table_out << makeTable(stat, dest_idx_offset, eta_stride,
                                 eta_labels,
                                 categories)
                           .columnWidth(10)
                           // only dump the footer for the last eta bin i.e. total
                           .dumpHeader(stat_i == 0)
                           .dumpFooter(stat_i + 1 == kNStat)
                           .separateLastRow(true) // separate the sum of all eta bins
                           .minLabelWidth(max_label_width)
                           .labelPrefix(stat_labels.at(stat_i));
        }
      }
      else
      {
        // dump one table with one row per counter showing the total eta range
        for (std::size_t eta_bin_i = (m_dumpAllStatEtaBins.value() ? 0 : m_statEtaBins.size() + 1);
             eta_bin_i < m_statEtaBins.size() + 2;
             ++eta_bin_i)
        {
          std::size_t dest_idx_offset = eta_bin_i * eta_stride;
          table_out << makeTable(stat, dest_idx_offset, stat_stride,
                                 stat_labels,
                                 categories,
                                 eta_labels.at(eta_bin_i))
                           .columnWidth(10)
                           // only dump the footer for the last eta bin i.e. total
                           .dumpFooter(!m_dumpAllStatEtaBins.value() || eta_bin_i == m_statEtaBins.size() + 1);
        }
      }
      ATH_MSG_INFO("statistics:\n"
                   << table_out.str());
      table_out.str("");

      // define retios first element numerator, second element denominator
      // each element contains a vector of counter and a multiplier e.g. +- 1
      // ratios are computed as  (sum_i stat[stat_i] *  multiplier_i ) / (sum_j stat[stat_j] *  multiplier_j )
      auto [ratio_labels, ratio_def] =
          TableUtils::splitRatioDefinitionsAndLabels({TableUtils::makeRatioDefinition("failed / seeds ",
                                                                                      std::vector<TableUtils::SummandDefinition>{
                                                                                          TableUtils::defineSummand(kNTotalSeeds, 1),
                                                                                          TableUtils::defineSummand(kNUsedSeeds, -1),
                                                                                          TableUtils::defineSummand(kNDuplicateSeeds, -1),
                                                                                          // no track counted  as used but want to include it as failed
                                                                                          TableUtils::defineSummand(kNoTrack, 1),
                                                                                      }, // failed seeds i.e. seeds which are not duplicates but did not produce a track
                                                                                      std::vector<TableUtils::SummandDefinition>{TableUtils::defineSummand(kNTotalSeeds, 1)}),
                                                      TableUtils::defineSimpleRatio("duplication / seeds", kNDuplicateSeeds, kNTotalSeeds),
                                                      TableUtils::defineSimpleRatio("Rejected refined params / seeds", kNRejectedRefinedSeeds, kNTotalSeeds),
                                                      TableUtils::defineSimpleRatio("selected / CKF tracks", kNSelectedTracks, kNOutputTracks),
                                                      TableUtils::defineSimpleRatio("selected tracks / used seeds", kNSelectedTracks, kNUsedSeeds),
                                                      TableUtils::defineSimpleRatio("branched tracks / used seeds", kMultipleBranches, kNUsedSeeds),
                                                      TableUtils::defineSimpleRatio("no 2nd CKF / CKF tracks", kNoSecond, kNOutputTracks),
                                                      TableUtils::defineSimpleRatio("shared hits / CKF tracks", kNTotalSharedHits, kNOutputTracks)});

      std::vector<float> ratio = TableUtils::computeRatios(ratio_def,
                                                           nSeedCollections() + 1,
                                                           m_statEtaBins.size() + 2,
                                                           stat);

      // the extra columns and rows for the projections are _not_ added internally
      std::size_t ratio_stride = TableUtils::ratioStride(nSeedCollections() + 1,
                                                         m_statEtaBins.size() + 2,
                                                         ratio_def);
      std::size_t ratio_eta_stride = TableUtils::subCategoryStride(nSeedCollections() + 1,
                                                                   m_statEtaBins.size() + 2,
                                                                   ratio_def);

      std::size_t max_label_width = TableUtils::maxLabelWidth(ratio_labels) + TableUtils::maxLabelWidth(eta_labels);
      if (m_dumpAllStatEtaBins.value())
      {
        // show for each ratio a table with one row per eta bin
        for (std::size_t ratio_i = 0; ratio_i < ratio_labels.size(); ++ratio_i)
        {
          table_out << makeTable(ratio,
                                 ratio_i * ratio_stride,
                                 ratio_eta_stride,
                                 eta_labels,
                                 categories)
                           .columnWidth(10)
                           // only dump the footer for the last eta bin i.e. total
                           .dumpHeader(ratio_i == 0)
                           .dumpFooter(ratio_i + 1 == ratio_labels.size())
                           .separateLastRow(true) // separate the sum of las
                           .minLabelWidth(max_label_width)
                           .labelPrefix(ratio_labels.at(ratio_i));
        }
      }
      else
      {
        // dump one table with one row per ratio showing  the total eta range
        table_out << makeTable(ratio,
                               (m_statEtaBins.size() + 1) * ratio_eta_stride + 0 * ratio_stride,
                               ratio_stride,
                               ratio_labels,
                               categories)
                         .columnWidth(10)
                         // only dump the footer for the last eta bin i.e. total
                         .minLabelWidth(max_label_width)
                         .dumpFooter(false);

        // also dump a table for final tracks over seeds (ratio_i==3) showing one row per eta bin
        eta_labels.erase(eta_labels.end() - 1); // drop last line of table which shows again all eta bins summed.
        constexpr std::size_t ratio_i = 3;
        table_out << makeTable(ratio,
                               ratio_i * ratio_stride,
                               ratio_eta_stride,
                               eta_labels,
                               categories)
                         .columnWidth(10)
                         .dumpHeader(false)
                         // only dump the footer for the last eta bin i.e. total
                         .dumpFooter(!m_dumpAllStatEtaBins.value() || ratio_i + 1 == ratio_labels.size())
                         .separateLastRow(false)
                         .minLabelWidth(max_label_width)
                         .labelPrefix(ratio_labels.at(ratio_i));
      }

      ATH_MSG_INFO("Ratios:\n"
                   << table_out.str());
    }
  }

  inline std::size_t TrackFindingAlg::getStatCategory(std::size_t seed_collection, float eta) const
  {
    std::vector<float>::const_iterator bin_iter = std::upper_bound(m_statEtaBins.begin(),
                                                                   m_statEtaBins.end(),
                                                                   m_useAbsEtaForStat ? std::abs(eta) : eta);
    std::size_t category_i = seed_collection * seedCollectionStride() + static_cast<std::size_t>(bin_iter - m_statEtaBins.begin());
    assert(category_i < m_stat.size());
    return category_i;
  }

  inline std::size_t TrackFindingAlg::computeStatSum(std::size_t seed_collection, EStat counter_i, const EventStats &stat) const
  {
    std::size_t out = 0u;
    for (std::size_t category_i = seed_collection * seedCollectionStride();
         category_i < (seed_collection + 1) * seedCollectionStride();
         ++category_i)
    {
      assert(category_i < stat.size());
      out += stat[category_i][counter_i];
    }
    return out;
  }


  StatusCode TrackFindingAlg::initializeMeasurementSelector() {
    std::vector<std::pair<float, float> > &chi2CutOffOutlier = m_measurementSelectorConfig.m_chi2CutOffOutlier;
    chi2CutOffOutlier .reserve( m_chi2CutOff.size() );
    if (!m_chi2OutlierCutOff.empty()) {
       if (m_chi2CutOff.size() !=  m_chi2OutlierCutOff.size()) {
          ATH_MSG_ERROR("Outlier chi2 cut off provided but number of elements does not agree with"
                        " chi2 cut off for measurements which however is required: "
                        << m_chi2CutOff.size() << " != " <<  m_chi2OutlierCutOff.size());
          return StatusCode::FAILURE;
       }
    }
    unsigned int idx=0;
    for (const auto &elm : m_chi2CutOff) {
       chi2CutOffOutlier.push_back( std::make_pair(static_cast<float>(elm),
                                                   idx < m_chi2OutlierCutOff.size()
                                                   ? static_cast<float>(m_chi2OutlierCutOff[idx])
                                                   : std::numeric_limits<float>::max()) );
       ++idx;
    }
    if (m_etaBins.size() > 2) {
      std::vector<float> &etaBinsf = m_measurementSelectorConfig.m_etaBins;
      etaBinsf.assign(m_etaBins.begin() + 1, m_etaBins.end() - 1);
    }

    return /*m_measurementSelector ?*/ StatusCode::SUCCESS /*: StatusCode::FAILURE*/;
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
