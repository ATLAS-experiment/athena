/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTDTrackExtensionAlg.cxx
 *
 * @brief Extends tracks from the inner tracker to the HGTD using the ACTS framework.
 *
 * @details This algorithm retrieves tracks from the inner detector, accesses their last measurement parameters,
 * and extends them into the HGTD using the Combinatorial Kalman Filter (CKF) in the ACTS framework.
 */

#include "src/HGTDTrackExtensionAlg.h"

// Athena
#include "AsgTools/ToolStore.h"
#include "AthenaMonitoringKernel/Monitored.h"

// ACTS
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometry/SurfaceOfMeasurementUtil.h"
#include "ActsCalibrators/SourceLinkHash.h"
#include "ActsInterop/Logger.h"

// ActsTrk
#include "ActsCalibBase/CalibrationContext.h"
#include "ActsEvent/TrackContainer.h"
#include "src/detail/AtlasMeasurementSelector.h"
#include "src/detail/OnTrackCalibrator.h"
#include "src/detail/TrackFindingMeasurements.h"
#include "src/detail/MeasurementIndex.h"
#include "src/detail/SharedHitCounter.h"
#include "ActsEvent/ExpectedHitUtils.h"
#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "ActsEvent/Decoration.h"
#include "ActsInterop/UnitConverters.h"

// STL
#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"
#include "Acts/TrackFinding/TrackStateCreator.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Utilities/VectorHelpers.hpp"
#include <optional>
#include "xAODTracking/TrackParticleContainer.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "GaudiKernel/PhysicalConstants.h"  // for Gaudi::Units::c_light
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"



namespace ActsTrk{

struct TrackFindingBaseAlg::CKF_pimpl : public detail::CKF_config {};

namespace {

/// Collector action for the ActionList of the Propagator
/// 
/// Adds the expcted layer pattern of the surfaces reached 
//  by the propagator 

struct Collector {
  using result_type = HGTDTrackExtensionAlg::ExpectedLayerPattern*;

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

    if (currentSurface->surfacePlacement() != nullptr) {
      const auto* detElem = dynamic_cast<const ActsDetectorElement*>(currentSurface->surfacePlacement());
      if(detElem != nullptr) {
        detail::addToExpectedLayerPattern(*result, *detElem);
      }
    }

    return Acts::Result<void>::success();
  }
};
}

// ---------------- Initialize ----------------
StatusCode HGTDTrackExtensionAlg::initialize()
{
  ATH_MSG_DEBUG("Initializing " << name() << "...");

  ATH_CHECK(TrackFindingBaseAlg::initialize());
  ATH_MSG_DEBUG("     " << m_absEtaMin);
  ATH_MSG_DEBUG("     " << m_absEtaMax);

  ATH_CHECK(m_trackParticleContainerName.initialize());
  ATH_CHECK(m_HGTDClusterContainerName.initialize());
  ATH_CHECK(m_actsTrackLinkKey.initialize());

  ATH_CHECK(m_uncalibratedMeasurementContainerKeys.initialize());

  // Initialize all WriteDecorHandleKeys
  ATH_CHECK(m_layerHasExtensionKey.initialize());
  ATH_CHECK(m_layerExtensionChi2Key.initialize());
  ATH_CHECK(m_layerClusterRawTimeKey.initialize());
  ATH_CHECK(m_layerClusterTimeKey.initialize());
  ATH_CHECK(m_extrapXKey.initialize());
  ATH_CHECK(m_extrapYKey.initialize());
  ATH_CHECK(m_numHGTDHitsKey.initialize());
  ATH_CHECK(m_hgtdTrackLinkKey.initialize());

  // Initialize surface accessor
  m_surfAcc = ActsTrk::detail::xAODUncalibMeasSurfAcc{m_trackingGeometrySvc.get()};

  //Retreive HGTD ID helper
  ATH_CHECK(detStore()->retrieve(m_id_helper, "HGTD_ID"));

  return StatusCode::SUCCESS;
}

// ---------------- Execute ----------------

StatusCode HGTDTrackExtensionAlg::execute(const EventContext& ctx) const
{
  ATH_MSG_DEBUG("Executing " << name() << "...");

  auto timer = Monitored::Timer<std::chrono::milliseconds>("TIME_execute");
  auto mon_nTracks = Monitored::Scalar<int>("nTracks");
  auto mon = Monitored::Group(m_monTool, timer, mon_nTracks);

  // ================================================== //
  // ========= RETRIEVE TRACK PARTICLES =============== //
  // ================================================== //

  const xAOD::TrackParticleContainer* trackParticles{nullptr};
  ATH_CHECK(SG::get(trackParticles, m_trackParticleContainerName, ctx));

  ATH_MSG_DEBUG("Size of trackParticles collection " << trackParticles->size());
  
  // Create WriteDecorHandles for all decorations
  SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<char>> layerHasExtensionHandle(m_layerHasExtensionKey, ctx);
  SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> layerExtensionChi2Handle(m_layerExtensionChi2Key, ctx);
  SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> layerClusterRawTimeHandle(m_layerClusterRawTimeKey, ctx);
  SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> layerClusterTimeHandle(m_layerClusterTimeKey, ctx);
  SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> extrapXHandle(m_extrapXKey, ctx);
  SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> extrapYHandle(m_extrapYKey, ctx);
  SG::WriteDecorHandle<xAOD::TrackParticleContainer, uint8_t> numHGTDHitsHandle(m_numHGTDHitsKey, ctx);
  SG::WriteDecorHandle<xAOD::TrackParticleContainer, ElementLink<ActsTrk::TrackContainer>> hgtdTrackLink(m_hgtdTrackLinkKey, ctx);
  
  // ================================================== //
  // ============ RETRIEVE MEASUREMENTS =============== //
  // ================================================== //
  const xAOD::HGTDClusterContainer* hgtdClusters{nullptr};
  ATH_CHECK(SG::get(hgtdClusters, m_HGTDClusterContainerName, ctx));


  std::vector<const xAOD::UncalibratedMeasurementContainer *> uncalibratedMeasurementContainers;
  std::size_t total_measurements = 0;
  ATH_CHECK(getContainersFromKeys(ctx, m_uncalibratedMeasurementContainerKeys, uncalibratedMeasurementContainers, total_measurements));


  detail::MeasurementIndex measurementIndex(uncalibratedMeasurementContainers.size());
  for (std::size_t icontainer = 0; icontainer < uncalibratedMeasurementContainers.size(); ++icontainer) {
    measurementIndex.addMeasurements(*uncalibratedMeasurementContainers[icontainer]);
  }

  detail::TrackFindingMeasurements measurements(uncalibratedMeasurementContainers.size());
  for (std::size_t icontainer = 0; icontainer < uncalibratedMeasurementContainers.size(); ++icontainer) {
    ATH_MSG_DEBUG("Create " << uncalibratedMeasurementContainers[icontainer]->size() << 
                  " source links from measurements in " << m_uncalibratedMeasurementContainerKeys[icontainer].key());
    measurements.addMeasurements(icontainer,
                                  *uncalibratedMeasurementContainers[icontainer],
                                  *m_trackingGeometrySvc->surfaceIdMap(),
                                  &measurementIndex);
  }

  ATH_MSG_DEBUG("measurement index size = " << measurementIndex.size());


  if (m_trackStatePrinter.isSet()) {
    m_trackStatePrinter->printMeasurements(ctx, uncalibratedMeasurementContainers, measurements.measurementOffsets());
  }

  // ================================================== //
  // ===================== COMPUTATION ================ //
  // ================================================== //

  EventStats event_stat;
  event_stat.resize(m_stat.size());

  DetectorContextHolder detContext {
    .geometry = m_ctxProvider.getGeometryContext(ctx),
    .magField = m_ctxProvider.getMagneticFieldContext(ctx),
    // CalibrationContext converter not implemented yet.
    .calib = m_ctxProvider.getCalibrationContext(ctx)
  };
  
  Acts::VectorTrackContainer actsTrackBackend;
  Acts::VectorMultiTrajectory actsTrackStateBackend;

  auto atomicMax=[](std::size_t new_val, std::atomic<std::size_t> &dest) -> void {
    std::size_t is_value;
    do {
       is_value = dest;
       if (is_value>=new_val) return;
    } while (!dest.compare_exchange_weak(is_value, new_val));
  };
  atomicMax(actsTrackBackend.size(), m_nTrackReserve); 
  atomicMax(actsTrackStateBackend.size(), m_nTrackStateReserve);

  detail::RecoTrackContainer actsTracksContainer(actsTrackBackend,
                                                  actsTrackStateBackend);

  
  addCountsAndProperties(actsTracksContainer, m_addCounts.value());

  detail::ExpectedLayerPatternHelper::add(actsTracksContainer);

  int extension_index{0};
  // Loop over each track particle and decorate it with various information
  std::unordered_map<uint32_t, uint32_t> extensions;
  for (const xAOD::TrackParticle* trackParticle : *trackParticles) {
    // Default to empty track data
    TrackExtensionData trackData;

    std::optional<ActsTrk::TrackContainer::ConstTrackProxy> optional_track = getActsTrack(*trackParticle);
    if (!optional_track.has_value()) {
      ATH_MSG_ERROR("No valid ACTS track associated with TrackParticle " << trackParticle->index());
      return StatusCode::FAILURE;
    }

    const ActsTrk::TrackContainer::ConstTrackProxy& track = optional_track.value();

    // Retrieve quantities for ACTS track
    float trackEta = Acts::VectorHelpers::eta(track.momentum());

    // Check eta coverage first
    if (std::abs(trackEta) < m_minEtaAcceptance or std::abs(trackEta) > m_maxEtaAcceptance) {
      ATH_MSG_DEBUG("!!!!! ------  Track eta " << trackEta
		    << " outside eta range [" << m_minEtaAcceptance.value() << ", " << m_maxEtaAcceptance.value()
		    << "], skipping extension  ------ !!!!!");

      // set default values
      trackData.hasClusterVec = {false, false, false, false};
      trackData.numHGTDHits = 0;
      layerHasExtensionHandle(*trackParticle) = trackData.hasClusterVec;
      layerExtensionChi2Handle(*trackParticle) = trackData.chi2Vec;
      layerClusterRawTimeHandle(*trackParticle) = trackData.rawTimeVec;
      layerClusterTimeHandle(*trackParticle) = trackData.timeVec;
      extrapXHandle(*trackParticle) = trackData.extrapX;
      extrapYHandle(*trackParticle) = trackData.extrapY;
      numHGTDHitsHandle(*trackParticle) = trackData.numHGTDHits;

      continue;
    }

    float trackpT = track.transverseMomentum();
    float trackPhi = track.phi();
    float trackNmeasurements = track.nMeasurements();
    
    ATH_MSG_DEBUG("TrackParticle " << trackParticle->index() << 
                  " has ACTS track with eta: " << trackEta << 
                  ", phi: " << trackPhi << 
                  ", pT: " << trackpT << 
                  " and nMeasurements: " << trackNmeasurements);

    // Parameters at last measurement state
    const auto lastMeasurementState = Acts::findLastMeasurementState(track);
    if (not lastMeasurementState.ok()) {
      ATH_MSG_ERROR("Problem finding last measurement state for acts track");
      return StatusCode::FAILURE;
    }
    const Acts::BoundTrackParameters lastMeasurementStateParameters = track.createParametersFromState(*lastMeasurementState);
    
    // Parameters at reference state of track - not necessarily a measurement state!!!                        
    const Acts::Surface& refSurface = track.referenceSurface();
    const Acts::BoundTrackParameters parametersAtRefSurface(refSurface.getSharedPtr(), 
							    track.parameters(), 
							    track.covariance(),
							    track.particleHypothesis());

    ATH_MSG_DEBUG("Initial track parameters for extension - lastMeasurementStateParameters:");
    ATH_MSG_DEBUG(" - eta: " << -1 * log(tan(lastMeasurementStateParameters.theta() * 0.5)));
    ATH_MSG_DEBUG(" - phi: " << lastMeasurementStateParameters.phi());
    ATH_MSG_DEBUG(" - pT: " << std::abs(1./lastMeasurementStateParameters.qOverP() * std::sin(lastMeasurementStateParameters.theta())));
    ATH_MSG_DEBUG(" - theta: " << lastMeasurementStateParameters.theta());
    ATH_MSG_DEBUG(" - qOverP: " << lastMeasurementStateParameters.qOverP());
    ATH_MSG_DEBUG(" - covariance exists: " << (lastMeasurementStateParameters.covariance().has_value() ? "yes" : "no"));


    // ActsTrk::MutableTrackContainer tracksContainerTemp;
    Acts::VectorTrackContainer trackBackend;
    Acts::VectorMultiTrajectory trackStateBackend;
    detail::RecoTrackContainer tracksContainerTemp(trackBackend, trackStateBackend);

    addCountsAndProperties(tracksContainerTemp, m_addCounts.value());
  
    detail::ExpectedLayerPatternHelper::add(tracksContainerTemp);

    // Now use the *last measurement parameters* parameters for the CKF
    if(findExtension(ctx,
                      detContext,
                      measurements,
                      measurementIndex,
                      lastMeasurementStateParameters,
                      tracksContainerTemp,
                      actsTracksContainer,
                      event_stat,
                      refSurface,
                      extension_index))
    {
      const detail::RecoTrackContainer::TrackProxy&  trackProxy = tracksContainerTemp.at(extension_index);
      trackData = processTrackExtension(ctx, trackParticle, trackProxy, hgtdClusters);     
      extensions.insert(std::make_pair(trackParticle->index(), actsTracksContainer.size() - 1));

    }
    else{
      trackData.hasClusterVec = {false, false, false, false};
      trackData.numHGTDHits = 0;
    }
      // Apply decorations from the track data
      layerHasExtensionHandle(*trackParticle) = trackData.hasClusterVec;
      layerExtensionChi2Handle(*trackParticle) = trackData.chi2Vec;
      layerClusterRawTimeHandle(*trackParticle) = trackData.rawTimeVec;
      layerClusterTimeHandle(*trackParticle) = trackData.timeVec;
      extrapXHandle(*trackParticle) = trackData.extrapX;
      extrapYHandle(*trackParticle) = trackData.extrapY;
      numHGTDHitsHandle(*trackParticle) = trackData.numHGTDHits;
  } // loop on tracks

  // ================================================== //
  // ===================== OUTPUTS ==================== //
  // ================================================== //

  ATH_MSG_DEBUG("    \\__ Found " << actsTracksContainer.size() << " extensions");

  // update the reserve space
  if (actsTrackBackend.size() > m_nTrackReserve) {
    m_nTrackReserve = static_cast<std::size_t>( std::ceil(m_memorySafetyMargin * actsTrackBackend.size()) );
  }
  if (actsTrackStateBackend.size() > m_nTrackStateReserve) {
    m_nTrackStateReserve = static_cast<std::size_t>( std::ceil(m_memorySafetyMargin * actsTrackStateBackend.size()) );
  }

  // convert to const
  Acts::ConstVectorTrackContainer constTrackBackend( std::move(actsTrackBackend) );
  Acts::ConstVectorMultiTrajectory constTrackStateBackend( std::move(actsTrackStateBackend) );
  std::unique_ptr< ActsTrk::TrackContainer> constTracksContainer = std::make_unique< ActsTrk::TrackContainer >( std::move(constTrackBackend),
                                                                                                                std::move(constTrackStateBackend) );

  SG::WriteHandle<ActsTrk::TrackContainer> trackContainerHandle = SG::makeHandle(m_trackContainerKey, ctx);

  ATH_MSG_DEBUG("    \\__ Tracks Container `" << m_trackContainerKey.key() << "` created ...");
  ATH_CHECK(trackContainerHandle.record(std::move(constTracksContainer)));

  const  ActsTrk::TrackContainer *const_track_container_ptr = trackContainerHandle.cptr();
  
  for ( const std::pair< const uint32_t,uint32_t> &ext : extensions) {

    hgtdTrackLink(*trackParticles->at(ext.first))
         = ElementLink<ActsTrk::TrackContainer>( *const_track_container_ptr,
                                                    ext.second );

}

  return StatusCode::SUCCESS;
}

bool HGTDTrackExtensionAlg::findExtension(
  const EventContext &ctx,
  const DetectorContextHolder& detContext,
  const detail::TrackFindingMeasurements &measurements,
  const detail::MeasurementIndex& measurementIndex,
  const Acts::BoundTrackParameters & initialParameters,
  detail::RecoTrackContainer &tracksContainerTemp,
  detail::RecoTrackContainer &actsTracksContainer,
  EventStats &event_stat,
  const Acts::Surface& refSurface,
  int& extension_index) const{

  //Setting pSurface to nullptr
  auto [options, secondOptions, measurementSelector] = getDefaultOptions(ctx, detContext, measurements, nullptr);


  std::size_t category_i = 0;
  const auto &trackSelectorCfg = trackFinder().trackSelector.config();
  auto stopBranchProxy = [&](const detail::RecoTrackContainer::TrackProxy &track,
                            const detail::RecoTrackContainer::TrackStateProxy &trackState) -> BranchStopperResult {
    return stopBranch(track, trackState, trackSelectorCfg, detContext.geometry, measurementIndex, 0, event_stat[category_i]);
  };
  options.extensions.branchStopper.connect(stopBranchProxy); 

  Acts::PropagatorOptions<detail::Stepper::Options, detail::Navigator::Options,
                          Acts::ActorList<Acts::MaterialInteractor>>
    extrapolationOptions(detContext.geometry, detContext.magField);

  Acts::TrackExtrapolationStrategy extrapolationStrategy =
    Acts::TrackExtrapolationStrategy::first;

  // Get the Acts tracks, given the initial parameters from last hit of itk track
  Acts::Result<std::vector<TrkProxy> > result =
    trackFinder().ckf.findTracks(initialParameters, options, tracksContainerTemp);

  // Track finding result
  if (not result.ok()) {
    ATH_MSG_WARNING("Track finding failed with error" << result.error());
    return false;
  }

  ATH_MSG_DEBUG("Built " << tracksContainerTemp.size() << " extensions from it");
  auto &foundTracks = result.value();

  // loop on the tracks we have just found
  int best_track_index = -1;
  float best_track_chi2 = 1000;
  TrkProxy &best_track_proxy = foundTracks.at(0);

  for (TrkProxy &firstTrack : foundTracks) {
    if((firstTrack.chi2() > 0) and (firstTrack.chi2() < best_track_chi2)){
      best_track_index = firstTrack.index();
      best_track_chi2  = firstTrack.chi2();
      best_track_proxy = firstTrack;
    }
  }

  if(best_track_index == -1) return false;

  ATH_MSG_DEBUG("Best extension index " << best_track_proxy.index() <<
                " nMeas " << best_track_proxy.nMeasurements() <<
                " chi2 " << best_track_proxy.chi2());

  if(addTrack(detContext,
      best_track_proxy,
      refSurface,
      extrapolationStrategy,
      actsTracksContainer,
      measurementIndex,
      tracksContainerTemp)){

      extension_index = best_track_index;
      return true;
    }
  else { 
    ATH_MSG_DEBUG("Track failed selection, not adding it");
    return false; 
  }
  
}

HGTDTrackExtensionAlg::TrackExtensionData HGTDTrackExtensionAlg::processTrackExtension(
  const EventContext& ctx,
  const xAOD::TrackParticle* trackParticle,
  const detail::RecoTrackContainer::TrackProxy& trackProxy,
  const xAOD::HGTDClusterContainer* hgtdClusters) const {

  TrackExtensionData data;

  // Modern approach uses surface accessor instead of detector element map
  
  // Apply track smoothing before trying to access chi2 values
  Acts::GeometryContext geoContext = m_ctxProvider.getGeometryContext(ctx);
  const Acts::TrackingGeometry* acts_tracking_geometry = m_trackingGeometrySvc->trackingGeometry().get();

  
  // Count measurements, holes, and HGTD hits specifically
  std::size_t nMeasurements = 0;
  std::size_t nHoles = 0;
  std::size_t nOutliers = 0;
  std::size_t nHGTDHits = 0;
  
  std::vector<char> hasHitInLayer = {false, false, false, false};
  std::vector<float> chi2PerLayer = {-1.0, -1.0, -1.0, -1.0};
  std::vector<float> timePerLayer = {-1.0, -1.0, -1.0, -1.0};
  std::vector<float> rawTimePerLayer = {-1.0, -1.0, -1.0, -1.0};

  // Extrapolated position - get the position at the first HGTD surface encountered
  float extrapX = 0.0;
  float extrapY = 0.0;
  float extrapZ = 0.0;
  bool foundExtrapolation = false;

  for (auto state : trackProxy.trackStatesReversed()) {
    auto flags = state.typeFlags();
    if (flags.isHole()) {
        nHoles++;
    } else if (flags.isOutlier()) {
        nOutliers++;
    } else if (flags.isMeasurement()) {
        nMeasurements++;
        
        // Check if this is an HGTD hit                
        const auto& surface = state.referenceSurface();
        const auto* detElem = getActsDetectorElement(surface);
        
        // Check if measurement is at a valid HGTD layer
        if (detElem == nullptr || detElem->detectorType() != DetectorType::Hgtd) {
          continue;
        }
        const std::size_t layerIndex = m_id_helper->layer(detElem->identify());
                
        nHGTDHits++;
        hasHitInLayer[layerIndex] = true;
        chi2PerLayer[layerIndex] = state.chi2();
              
        // Get the measured time from the calibrated 3D measurement (local x, y, time)
        float rawTime = 0.0f;
        float calibratedTime = 0.0f;

        if (state.hasCalibrated()) {
          // Extract time from calibrated data
          try {
            const auto& calibrated = state.template calibrated<3>();
            calibratedTime = ActsTrk::timeToAthena(calibrated(2));
            ATH_MSG_DEBUG("Got time from calibrated<3>: " << calibratedTime);
          } catch (const std::exception& e) {
            ATH_MSG_WARNING("Failed to extract time from calibrated<3>: " << e.what());                    
          }
        }
    
        // Extract raw time from HGTD clusters
        const xAOD::HGTDCluster* cluster = getHGTDClusterFromState(ctx, state, hgtdClusters);

        if (cluster) {
          rawTime = cluster->time();
          ATH_MSG_DEBUG("Got raw time from cluster: " << rawTime);
        } else {
          ATH_MSG_WARNING("Could not get cluster from state");
        }

        // Store the raw time
        rawTimePerLayer[layerIndex] = calibratedTime;
        if (cluster) {
          auto [correctedTime, timeErr] = correctTOF(
              trackParticle,
              cluster,
              calibratedTime,
              0.0, // time error set to zero for now!
              acts_tracking_geometry,
              geoContext);
          timePerLayer[layerIndex] = correctedTime;
          ATH_MSG_DEBUG("Applied TOF correction: " << calibratedTime << " -> " << correctedTime);
        } else {
          // No cluster or time, use raw time
          timePerLayer[layerIndex] = calibratedTime;
          ATH_MSG_DEBUG("No cluster found for TOF correction, using calibrated time: " << calibratedTime);
        }
        
        // For extrapolation: use the first HGTD hit's surface position.
        if (!foundExtrapolation) {
          foundExtrapolation = true;
          if (state.hasPredicted()) {
            // Get the local predicted position
            const auto& predicted = state.predicted();
            Acts::Vector2 localPos(predicted[Acts::eBoundLoc0], predicted[Acts::eBoundLoc1]);
              
            // Transform to global coordinates
            Acts::Vector3 globalPos = surface.localToGlobal(
                geoContext,
                localPos,
                Acts::Vector3::Zero());
                  
            extrapX = globalPos.x();
            extrapY = globalPos.y();
            extrapZ = globalPos.z();
              
            ATH_MSG_DEBUG("Extrapolated position (predicted) at HGTD: x=" << extrapX 
                        << ", y=" << extrapY << ", z=" << extrapZ);
          } else {
              // Fallback to surface center
              Acts::Vector3 globalPos = surface.center(geoContext);
              extrapX = globalPos.x();
              extrapY = globalPos.y();
              extrapZ = globalPos.z();
              
              ATH_MSG_DEBUG("Extrapolated position (surface center) at HGTD: x=" << extrapX 
                          << ", y=" << extrapY << ", z=" << extrapZ);
          }
        }
        ATH_MSG_DEBUG("Found HGTD hit on layer " << layerIndex 
                  << ", chi2=" << chi2PerLayer[layerIndex]
                  << ", time=" << timePerLayer[layerIndex]);                                    
    }
  }
  
  ATH_MSG_DEBUG("Extension Statistics: "
              << " nMeasurements=" << nMeasurements
              << " nHGTDHits=" << nHGTDHits
              << " nHoles=" << nHoles 
              << " nOutliers=" << nOutliers
              << " extrapolation found: " << (foundExtrapolation ? "yes" : "no")); 
  

  // Fill the data structure with results
  data.hasClusterVec = std::move(hasHitInLayer);
  data.chi2Vec    = std::move(chi2PerLayer);
  data.timeVec    = std::move(timePerLayer);
  data.rawTimeVec = std::move(rawTimePerLayer);
  data.extrapX = extrapX;
  data.extrapY = extrapY;
  data.extrapZ = extrapZ;
  data.numHGTDHits = nHGTDHits;

  return data;
}

std::pair<float, float> HGTDTrackExtensionAlg::correctTOF(
  const xAOD::TrackParticle* trackParticle,
  const xAOD::HGTDCluster* cluster,
  float measuredTime,
  float measuredTimeErr,
  const Acts::TrackingGeometry*,
  const Acts::GeometryContext& geoContext) const {

  ATH_MSG_DEBUG("Correcting input time: " << measuredTime);

  if (!trackParticle || !cluster) {
    ATH_MSG_WARNING("Null pointer provided to correctTOF");
    return {measuredTime, measuredTimeErr}; // Return uncorrected values
  }

  // Get the surface for this HGTD cluster
  const Acts::Surface* surface = nullptr;
  try {
    surface = m_surfAcc.get(cluster);
  } catch (const std::exception& e) {
    ATH_MSG_WARNING("Exception getting surface: " << e.what());
    return {measuredTime, measuredTimeErr}; // Return uncorrected values
  }
  
  if (!surface) {
    ATH_MSG_WARNING("Could not determine surface for HGTD cluster with id " 
                    << cluster->identifier());
    return {measuredTime, measuredTimeErr}; // Return uncorrected values
  }

  // Get the global position of the hit
  Acts::Vector3 globalHitPos;
  try {
    // Try to get the cluster's local position
    auto localPos = cluster->localPosition<3>();
    // Transform to global coordinates
    globalHitPos = surface->localToGlobal(
        geoContext,
        Acts::Vector2(localPos[0], localPos[1]),
        Acts::Vector3::Zero());
  } catch (const std::exception& e) {
    ATH_MSG_WARNING("Failed to transform position: " << e.what());
    // Fall back to surface center
    globalHitPos = surface->center(geoContext);
  }
  
  // Get track origin (vertex position)
  //option 1 - use beamspot
  //Amg::Vector3D trackOrigin(trackParticle->vx(), trackParticle->vy(), trackParticle->vz());
  
  //option 2 - use perigee - this is what is done in legacy code: 
  // https://gitlab.cern.ch/atlas/athena/-/blob/main/HighGranularityTimingDetector/HGTD_Reconstruction/HGTD_RecTools/src/StraightLineTOFcorrectionTool.cxx
  // Get track origin from perigee parameters instead of vertex
  // In ACTS, this is the d0 and z0 parameter with reference to the beamline
  
  // Get the perigee position (the point of closest approach to the beamline)
  double d0 = trackParticle->d0();
  double z0 = trackParticle->z0();
  double phi0 = trackParticle->phi0();
  
  Amg::Vector3D trackOrigin(-d0 * std::sin(phi0), d0 * std::cos(phi0), z0);
  ATH_MSG_DEBUG("Track perigee: d0=" << d0 << ", z0=" << z0 << ", phi0=" << phi0);
  ATH_MSG_DEBUG("Track origin (perigee): (" << trackOrigin.x() << ", " 
                << trackOrigin.y() << ", " << trackOrigin.z() << ")");
  
  // Calculate distance components
  float dx = globalHitPos.x() - trackOrigin.x();
  float dy = globalHitPos.y() - trackOrigin.y();
  float dz = globalHitPos.z() - trackOrigin.z();
  
  // Calculate distance and time of flight
  float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
  float tof = distance / Gaudi::Units::c_light;
  
  // Apply TOF correction
  float correctedTime = measuredTime - tof;
  
  ATH_MSG_DEBUG("Track origin: (" << trackOrigin.x() << ", " 
                << trackOrigin.y() << ", " << trackOrigin.z() << ")");
  ATH_MSG_DEBUG("Hit position: (" << globalHitPos.x() << ", " 
                << globalHitPos.y() << ", " << globalHitPos.z() << ")");
  ATH_MSG_DEBUG("Distance = " << distance << " mm, TOF = " << tof 
                << " ns, Corrected time = " << correctedTime);
  
  return {correctedTime, measuredTimeErr};
}

const xAOD::HGTDCluster* HGTDTrackExtensionAlg::getHGTDClusterFromState(
  const EventContext& ctx, 
  const ActsTrk::detail::RecoConstTrackStateContainerProxy& state,
  const xAOD::HGTDClusterContainer* hgtdClusters) const {

  if (state.hasUncalibratedSourceLink()) {
    auto uncalib_cluster = detail::xAODUncalibMeasCalibrator::unpack(state.getUncalibratedSourceLink());
    assert( uncalib_cluster != nullptr);   
    xAOD::UncalibMeasType clusterType = uncalib_cluster->type();

    if (clusterType == xAOD::UncalibMeasType::HGTDClusterType) {
      ATH_MSG_DEBUG("Found HGTD cluster in source link");
      auto hgtdCluster = static_cast<const xAOD::HGTDCluster *>(uncalib_cluster);
      return hgtdCluster;
    } 
    else {
      ATH_MSG_DEBUG("Source link contains non-HGTD measurement type: " << static_cast<int>(clusterType));
    }

    // If we have a reference surface, try to match by position
    if (state.hasReferenceSurface()) {
      const auto& surface = state.referenceSurface();
      Acts::GeometryIdentifier geoID = surface.geometryId();

      const auto *acts_detector_element = getActsDetectorElement(surface);
      
      // Check if this is an HGTD surface
      if (acts_detector_element->detectorType() == DetectorType::Hgtd) {
        ATH_MSG_DEBUG("This is an HGTD surface with ID: " << geoID.volume() << ":" << geoID.layer());
        
        // Modern approach uses surface accessor instead of detector element map
        
        // Get global position of the state surface
        const Acts::GeometryContext& geoContext = m_ctxProvider.getGeometryContext(ctx);
        Acts::Vector3 statePos = surface.center(geoContext);
        
        // Find the closest cluster to this state position
        const xAOD::HGTDCluster* closestCluster = nullptr;
        double minDistance = 100.0; // Use a reasonable threshold (in mm)
        
        for (const xAOD::HGTDCluster* cluster : *hgtdClusters) {
          // Get the cluster's surface
          const Acts::Surface* clusterSurface = m_surfAcc.get(cluster);
              
          if (!clusterSurface) continue;
          
          // Check if it's on the same surface by comparing geometry IDs
          Acts::GeometryIdentifier clusterGeoID = clusterSurface->geometryId();
          if (clusterGeoID.volume() == geoID.volume() && clusterGeoID.layer() == geoID.layer()) {
            // Get cluster position
            Acts::Vector3 clusterPos = clusterSurface->center(geoContext);
            
            // Calculate 2D distance (x,y only, since z is fixed for a layer)
            double dx = clusterPos.x() - statePos.x();
            double dy = clusterPos.y() - statePos.y();
            double distance = std::sqrt(dx*dx + dy*dy);
            
            // Update closest if this is better
            if (distance < minDistance) {
              minDistance = distance;
              closestCluster = cluster;
              ATH_MSG_DEBUG("Found possible cluster match at distance " << distance << " mm");
            }
          }
        }
        
        if (closestCluster) {
          ATH_MSG_DEBUG("Found closest cluster at distance " << minDistance << " mm");
          return closestCluster;
        } else {
          ATH_MSG_DEBUG("No matching cluster found on this surface");
        }
      }
    }
  } 
  else {
    ATH_MSG_DEBUG("State doesn't have uncalibrated source link");
  }
  return nullptr;
}

bool HGTDTrackExtensionAlg::addTrack(const DetectorContextHolder& detContext,
                                           detail::RecoTrackContainerProxy &track,
                                           const Acts::Surface& refSurface,
                                           const Acts::TrackExtrapolationStrategy& extrapolationStrategy,
                                           detail::RecoTrackContainer &actsTracksContainer,
                                           const detail::MeasurementIndex& measurementIndex,
                                           const detail::RecoTrackContainer& tracksContainerTemp) const{

  std::array<unsigned int, 4> expectedLayerPattern{};

  // if the the perigeeSurface was not hit (in particular the case for the inside-out pass,
  // the track has no reference surface and the extrapolation to the perigee has not been done
  // yet.
  if (not track.hasReferenceSurface()) {
    auto extrapolationResult =
    extrapolateTrackToReferenceSurface(detContext, track,
                                      refSurface,
                                      trackFinder().extrapolator,
                                      extrapolationStrategy,
                                      expectedLayerPattern);
    if (not extrapolationResult.ok()) {
      ATH_MSG_WARNING("Extrapolation for "
                      << track.index()
                      << " failed with error " << extrapolationResult.error()
                      << " dropping track candidate.");
      return false;
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

  if ( not trackFinder().trackSelector.isValidTrack(track)) {
    ATH_MSG_WARNING("Track " << track.index() << " failed track selection");
    if ( m_trackStatePrinter.isSet() ) {
      m_trackStatePrinter->printTrack(detContext.geometry, tracksContainerTemp, track, measurementIndex, true);
    }
    return false;
  }

  auto actsDestProxy   = actsTracksContainer.makeTrack();
  actsDestProxy.copyFrom(track);  // make sure we copy track states!
  detail::ExpectedLayerPatternHelper::set(actsDestProxy, expectedLayerPattern);

  ATH_MSG_DEBUG("Added Track " << track.index() << " into container");
  return true;                                                                               
}

Acts::Result<void> HGTDTrackExtensionAlg::extrapolateTrackToReferenceSurface(
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
  ExpectedLayerPattern*& collectorResult = state.get<HGTDTrackExtensionAlg::ExpectedLayerPattern*>();
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
  track.covariance() = propagateResult->endParameters.value().covariance().value();

  return Acts::Result<void>::success();
}

} // End namespace ActsTrk
