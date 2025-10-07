/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/TrackFindingBaseAlg.h"

// ActsTrk
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsInterop/Logger.h"
#include "src/detail/AtlasMeasurementSelector.h"
#include "src/detail/FitterHelperFunctions.h"
#include "src/detail/TrackFindingMeasurements.h"
#include "ActsInterop/TableUtils.h"
#include "Acts/Surfaces/PerigeeSurface.hpp"

namespace ActsTrk {
  struct TrackFindingBaseAlg::CKF_pimpl : public detail::CKF_config {};

  TrackFindingBaseAlg::CKF_pimpl &TrackFindingBaseAlg::trackFinder() { return *m_trackFinder; }
  const TrackFindingBaseAlg::CKF_pimpl &TrackFindingBaseAlg::trackFinder() const { return *m_trackFinder; }

  TrackFindingBaseAlg::TrackFindingBaseAlg(const std::string &name, ISvcLocator *pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator) {}

  TrackFindingBaseAlg::~TrackFindingBaseAlg() = default;

  StatusCode TrackFindingBaseAlg::initialize() {
    ATH_MSG_DEBUG("Properties Summary:");
    ATH_MSG_DEBUG("   " << m_maxPropagationStep);
    ATH_MSG_DEBUG("   " << m_etaBins);
    ATH_MSG_DEBUG("   " << m_chi2CutOff);
    ATH_MSG_DEBUG("   " << m_chi2OutlierCutOff);
    ATH_MSG_DEBUG("   " << m_numMeasurementsCutOff);
    ATH_MSG_DEBUG("   " << m_ptMinMeasurements);
    ATH_MSG_DEBUG("   " << m_absEtaMaxMeasurements);
    ATH_MSG_DEBUG("   " << m_doBranchStopper);
    ATH_MSG_DEBUG("   " << m_addCounts);
    ATH_MSG_DEBUG("   " << m_checkCounts);
    ATH_MSG_DEBUG("   " << m_doTwoWay);
    ATH_MSG_DEBUG("   " << m_phiMin);
    ATH_MSG_DEBUG("   " << m_phiMax);
    ATH_MSG_DEBUG("   " << m_etaMin);
    ATH_MSG_DEBUG("   " << m_etaMax);
    ATH_MSG_DEBUG("   " << m_absEtaMin);
    ATH_MSG_DEBUG("   " << m_absEtaMax);
    ATH_MSG_DEBUG("   " << m_ptMin);
    ATH_MSG_DEBUG("   " << m_ptMax);
    ATH_MSG_DEBUG("   " << m_d0Min);
    ATH_MSG_DEBUG("   " << m_d0Max);
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
    ATH_MSG_DEBUG("   " << m_branchStopperPtMinFactor);
    ATH_MSG_DEBUG("   " << m_branchStopperAbsEtaMaxExtra);
    ATH_MSG_DEBUG("   " << m_branchStopperMeasCutReduce);
    ATH_MSG_DEBUG("   " << m_branchStopperAbsEtaMeasCut);
    ATH_MSG_DEBUG("   " << m_endOfWorldVolumeIds);

    m_logger = makeActsAthenaLogger(this, "Acts");

    // Read and Write handles
    ATH_CHECK(m_trackContainerKey.initialize());
    ATH_CHECK(m_tracksBackendHandlesHelper.initialize(ActsTrk::prefixFromTrackContainerName(m_trackContainerKey.key())));

    ATH_CHECK(m_monTool.retrieve(EnableTool{not m_monTool.empty()}));
    ATH_CHECK(m_trackingGeometryTool.retrieve());
    ATH_CHECK(m_extrapolationTool.retrieve());
    ATH_CHECK(m_trackStatePrinter.retrieve(EnableTool{not m_trackStatePrinter.empty()}));
    ATH_CHECK(m_ATLASConverterTool.retrieve());
    ATH_CHECK(m_fitterTool.retrieve());
    ATH_CHECK(m_pixelCalibTool.retrieve(EnableTool{not m_pixelCalibTool.empty()}));
    ATH_CHECK(m_stripCalibTool.retrieve(EnableTool{not m_stripCalibTool.empty()}));
    ATH_CHECK(m_hgtdCalibTool.retrieve(EnableTool{not m_hgtdCalibTool.empty()}));

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

    ATH_MSG_DEBUG(trackSelectorCfg);

    // initializer measurement selector and connect it to the delegates of the track finder optins
    ATH_CHECK(initializeMeasurementSelector());

    detail::CKF_config ckfConfig{
        std::move(extrapolator),
        detail::CKF{std::move(propagator), logger().cloneWithSuffix("CKF")},
        {},
        Acts::TrackSelector{trackSelectorCfg}};

    m_trackFinder = std::make_unique<CKF_pimpl>(std::move(ckfConfig));

    trackFinder().ckfExtensions.updater.connect<&ActsTrk::detail::FitterHelperFunctions::gainMatrixUpdate<detail::RecoTrackStateContainer>>();

    m_unalibMeasSurfAcc = detail::xAODUncalibMeasSurfAcc {m_trackingGeometryTool.get()};

    initStatTables();

    return StatusCode::SUCCESS;
  }

  StatusCode TrackFindingBaseAlg::execute(const EventContext &) const {
    ATH_MSG_FATAL("execute() method from the base class was called! Implement proper execute() method in the derived class!");

    return StatusCode::FAILURE;
  }

  StatusCode TrackFindingBaseAlg::finalize() {
    printStatTables();

    return StatusCode::SUCCESS;
  }

  std::unique_ptr<ActsTrk::IMeasurementSelector> TrackFindingBaseAlg::setMeasurementSelector(
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

  TrackFindingBaseAlg::TrackFindingDefaultOptions TrackFindingBaseAlg::getDefaultOptions(
      const DetectorContextHolder &detContext,
      const detail::TrackFindingMeasurements &measurements,
      const Acts::PerigeeSurface* pSurface) const {
    Acts::PropagatorPlainOptions plainOptions{detContext.geometry, detContext.magField};
    plainOptions.maxSteps = m_maxPropagationStep;
    plainOptions.direction = Acts::Direction::Forward();
    plainOptions.endOfWorldVolumeIds = m_endOfWorldVolumeIds;

    // Set the CombinatorialKalmanFilter options
    TrackFinderOptions options(detContext.geometry, detContext.magField, detContext.calib,
                               trackFinder().ckfExtensions, plainOptions, pSurface);

    std::unique_ptr<ActsTrk::IMeasurementSelector> measurementSelector = setMeasurementSelector(measurements, options);

    Acts::PropagatorPlainOptions plainSecondOptions{detContext.geometry, detContext.magField};
    plainSecondOptions.maxSteps = m_maxPropagationStep;
    plainSecondOptions.direction = plainOptions.direction.invert();

    TrackFinderOptions secondOptions(detContext.geometry, detContext.magField, detContext.calib,
                                     options.extensions, plainSecondOptions, pSurface);
    secondOptions.targetSurface = pSurface;
    secondOptions.skipPrePropagationUpdate = true;

    return {std::move(options), std::move(secondOptions), std::move(measurementSelector)};
  };

  const Acts::TrackSelector::Config& TrackFindingBaseAlg::getCuts (double eta) const {
    const auto &trackSelectorCfg = trackFinder().trackSelector.config();
    // return the last bin for |eta|>=4 or nan
    return (!(std::abs(eta) < trackSelectorCfg.absEtaEdges.back())) ? trackSelectorCfg.cutSets.back()
           : (std::abs(eta) < trackSelectorCfg.absEtaEdges.front()) ? trackSelectorCfg.cutSets.front()
                                                                    : trackSelectorCfg.getCuts(eta);
  };

  std::vector<typename detail::RecoTrackContainer::TrackProxy>
  TrackFindingBaseAlg::doTwoWayTrackFinding(const detail::RecoTrackStateContainerProxy& firstMeasurement,
                                            const TrkProxy &trackProxy,
                                            detail::RecoTrackContainer &tracksContainerTemp,
                                            const TrackFinderOptions &options) const {
    if (not m_doTwoWay) return {};

    // Create initial parameters for the propagation
    Acts::BoundTrackParameters secondInitialParameters = trackProxy.createParametersFromState(detail::RecoConstTrackStateContainerProxy{firstMeasurement});
    if (!secondInitialParameters.referenceSurface().insideBounds(secondInitialParameters.localPosition())) {  // #3751
      return {};
    }

    auto rootBranch = tracksContainerTemp.makeTrack();
    rootBranch.copyFromWithoutStates(trackProxy);  // #3534

    // perform track finding
    auto secondResult =
      trackFinder().ckf.findTracks(secondInitialParameters, options, tracksContainerTemp, rootBranch);
    if (not secondResult.ok()) {
      return {};
    }
    return secondResult.value();
  }

  xAOD::UncalibMeasType TrackFindingBaseAlg::measurementType (const detail::RecoTrackContainer::TrackStateProxy &trackState) {
    if (trackState.hasReferenceSurface()) {
      if (const auto *actsDetElem = dynamic_cast<const IDetectorElementBase *>(trackState.referenceSurface().associatedDetectorElement())) {
        switch (actsDetElem->detectorType()) {
        case DetectorType::Pixel:
          return xAOD::UncalibMeasType::PixelClusterType;
        case DetectorType::Sct:
          return xAOD::UncalibMeasType::StripClusterType;
        case DetectorType::Hgtd:
          return xAOD::UncalibMeasType::HGTDClusterType;
        default:
          break;
        }
      }
    }
    return xAOD::UncalibMeasType::Other;
  }

  TrackFindingBaseAlg::BranchStopperResult TrackFindingBaseAlg::stopBranch(
      const detail::RecoTrackContainer::TrackProxy &track,
      const detail::RecoTrackContainer::TrackStateProxy &trackState,
      const Acts::TrackSelector::EtaBinnedConfig &trackSelectorCfg,
      const Acts::GeometryContext &tgContext,
      const detail::MeasurementIndex &measurementIndex,
      const std::size_t typeIndex,
      EventStats::value_type &event_stat_category_i) const {
    if (m_addCounts) {
      updateCounts(track, trackState.typeFlags(),
                   measurementType(trackState));
      if (m_checkCounts) {
        checkCounts(track);
      }
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


    // In the pixel endcap regions relax the requirement for minMeasurements before cutting the branch off
    auto minMeasurementsBranchStop = std::abs(eta) > m_branchStopperAbsEtaMeasCut ? cutSet.minMeasurements - m_branchStopperMeasCutReduce : cutSet.minMeasurements;
    bool enoughMeasurements = (track.nMeasurements() >= minMeasurementsBranchStop);
    bool tooManyHoles = (track.nHoles() > cutSet.maxHoles);
    bool tooManyOutliers = (track.nOutliers() > cutSet.maxOutliers);

    if (m_addCounts) {
      auto [enoughMeasurementsPS, tooManyHolesPS, tooManyOutliersPS] =
          selectCounts(track, eta);
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

    if (m_addCounts) {
      ATH_MSG_DEBUG("CkfBranchStopper: stop and "
                    << (enoughMeasurements ? "keep" : "drop")
                    << " branch with nHoles=" << track.nHoles() << " ("
                    << s_branchState.nPixelHoles(track) << " pixel+"
                    << s_branchState.nStripHoles(track) << " strip+"
                    << s_branchState.nHgtdHoles(track)
                    << " hgtd), nOutliers=" << track.nOutliers() << " ("
                    << s_branchState.nPixelOutliers(track) << "+"
                    << s_branchState.nStripOutliers(track) << "+"
                    << s_branchState.nHgtdOutliers(track)
                    << "), nMeasurements=" << track.nMeasurements() << " ("
                    << s_branchState.nPixelHits(track) << "+"
                    << s_branchState.nStripHits(track) << "+"
                    << s_branchState.nHgtdHits(track) << ")");
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


  void TrackFindingBaseAlg::addCounts(detail::RecoTrackContainer& tracksContainer)
  {
    tracksContainer.addColumn<unsigned int>("nPixelHits");
    tracksContainer.addColumn<unsigned int>("nStripHits");
    tracksContainer.addColumn<unsigned int>("nHgtdHits");
    tracksContainer.addColumn<unsigned int>("nPixelHoles");
    tracksContainer.addColumn<unsigned int>("nStripHoles");
    tracksContainer.addColumn<unsigned int>("nHgtdHoles");
    tracksContainer.addColumn<unsigned int>("nPixelOutliers");
    tracksContainer.addColumn<unsigned int>("nStripOutliers");
    tracksContainer.addColumn<unsigned int>("nHgtdOutliers");
  }

  void TrackFindingBaseAlg::initCounts(const detail::RecoTrackContainer::TrackProxy &track)
  {
    s_branchState.nPixelHits(track) = 0;
    s_branchState.nStripHits(track) = 0;
    s_branchState.nHgtdHits(track) = 0;
    s_branchState.nPixelHoles(track) = 0;
    s_branchState.nStripHoles(track) = 0;
    s_branchState.nHgtdHoles(track) = 0;
    s_branchState.nPixelOutliers(track) = 0;
    s_branchState.nStripOutliers(track) = 0;
    s_branchState.nHgtdOutliers(track) = 0;
  }

  void TrackFindingBaseAlg::updateCounts(
      const detail::RecoTrackContainer::TrackProxy &track,
      Acts::ConstTrackStateType typeFlags, xAOD::UncalibMeasType detType) {
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
    } else if (detType == xAOD::UncalibMeasType::HGTDClusterType) {
      if (typeFlags.test(Acts::TrackStateFlag::HoleFlag)) {
        s_branchState.nHgtdHoles(track)++;
      } else if (typeFlags.test(Acts::TrackStateFlag::OutlierFlag)) {
        s_branchState.nHgtdOutliers(track)++;
      } else if (typeFlags.test(Acts::TrackStateFlag::MeasurementFlag)) {
        s_branchState.nHgtdHits(track)++;
      }
    }
  }
  
  void TrackFindingBaseAlg::checkCounts(const detail::RecoTrackContainer::TrackProxy &track) const {
    // This check will fail if there are other types (HGTD, MS?) of hits, holes, or outliers.
    // The check can be removed when it is no longer appropriate.
    if (track.nMeasurements() != s_branchState.nPixelHits(track) + s_branchState.nStripHits(track) + s_branchState.nHgtdHits(track))
      ATH_MSG_WARNING("mismatched hit count: total (" << track.nMeasurements()
                      << ") != pixel (" << s_branchState.nPixelHits(track)
                      << ") + strip (" << s_branchState.nStripHits(track)
                      << ") + hgtd (" << s_branchState.nHgtdHits(track)
                      << ")");
    if (track.nHoles() != s_branchState.nPixelHoles(track) + s_branchState.nStripHoles(track) + s_branchState.nHgtdHoles(track))
      ATH_MSG_WARNING("mismatched hole count: total (" << track.nHoles()
                      << ") < pixel (" << s_branchState.nPixelHoles(track)
                      << ") + strip (" << s_branchState.nStripHoles(track)
                      << ") + hgtd (" << s_branchState.nHgtdHoles(track)
                      << ")");
    if (track.nOutliers() != s_branchState.nPixelOutliers(track) + s_branchState.nStripOutliers(track) + s_branchState.nHgtdOutliers(track))
      ATH_MSG_WARNING("mismatched outlier count: total (" << track.nOutliers()
                      << ") != pixel (" << s_branchState.nPixelOutliers(track)
                      << ") + strip (" << s_branchState.nStripOutliers(track)
                      << ") + hgtd (" << s_branchState.nHgtdOutliers(track)
                      << ")");
  };

  std::array<bool, 3> TrackFindingBaseAlg::selectCounts(const detail::RecoTrackContainer::TrackProxy &track, double eta) const {
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
    enoughMeasurements = enoughMeasurements && !cutMin(s_branchState.nHgtdHits(track), m_minHgtdHits);
    tooManyHoles = tooManyHoles || cutMax(s_branchState.nPixelHoles(track), m_maxPixelHoles);
    tooManyHoles = tooManyHoles || cutMax(s_branchState.nStripHoles(track), m_maxStripHoles);
    tooManyHoles = tooManyHoles || cutMax(s_branchState.nHgtdHoles(track), m_maxHgtdHoles);
    tooManyOutliers = tooManyOutliers || cutMax(s_branchState.nPixelOutliers(track), m_maxPixelOutliers);
    tooManyOutliers = tooManyOutliers || cutMax(s_branchState.nStripOutliers(track), m_maxStripOutliers);
    tooManyOutliers = tooManyOutliers || cutMax(s_branchState.nHgtdOutliers(track), m_maxHgtdOutliers);

    return {enoughMeasurements, tooManyHoles, tooManyOutliers};
  }

  StatusCode TrackFindingBaseAlg::initializeMeasurementSelector() {
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

  // === Statistics printout =================================================

  void TrackFindingBaseAlg::initStatTables() {
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
  void TrackFindingBaseAlg::copyStats(const EventStats &event_stat) const {
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
  void TrackFindingBaseAlg::printStatTables() const {
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
                                          std::make_pair(kNTotalSharedHits, "Total shared hits"),
                                          std::make_pair(kNForcedSeedMeasurements, "Total forced measurements")
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
                                                      TableUtils::defineSimpleRatio("shared hits / CKF tracks", kNTotalSharedHits, kNOutputTracks),
                                                      TableUtils::defineSimpleRatio("forced measurements / used seeds", kNForcedSeedMeasurements, kNUsedSeeds)});

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

  std::size_t TrackFindingBaseAlg::getStatCategory(std::size_t seed_collection, float eta) const {
    std::vector<float>::const_iterator bin_iter = std::upper_bound(m_statEtaBins.begin(),
                                                                   m_statEtaBins.end(),
                                                                   m_useAbsEtaForStat ? std::abs(eta) : eta);
    std::size_t category_i = seed_collection * seedCollectionStride() + static_cast<std::size_t>(bin_iter - m_statEtaBins.begin());
    assert(category_i < m_stat.size());
    return category_i;
  }

  std::size_t TrackFindingBaseAlg::computeStatSum(std::size_t seed_collection, EStat counter_i, const EventStats &stat) const {
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

  bool TrackFindingBaseAlg::selectCountsFinal(const detail::RecoTrackContainer::TrackProxy &track) const {
    if (not m_addCounts) return true;
    double eta = -std::log(std::tan(0.5 * track.theta()));
    auto [enoughMeasurementsPS, tooManyHolesPS, tooManyOutliersPS] = selectCounts(track, eta);
    return enoughMeasurementsPS && !tooManyHolesPS && !tooManyOutliersPS;
  }

}  // namespace ActsTrk
