/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTDTrackExtensionAlg.cxx
 *
 * @brief Extends tracks from the inner tracker to the HGTD using the ACTS framework.
 *
 * @details This algorithm retrieves tracks from the inner detector, accesses their last measurement parameters,
 * and extends them into the HGTD using the Combinatorial Kalman Filter (CKF) in the ACTS framework.
 */

#include "src/HGTDTrackExtensionAlg.h"
#include "AsgTools/ToolStore.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsInterop/Logger.h"
#include "src/detail/AtlasMeasurementSelector.h"
#include "src/detail/OnTrackCalibrator.h"
#include "src/detail/TrackFindingMeasurements.h"
#include "src/detail/MeasurementIndex.h"
#include "ActsGeometry/SurfaceOfMeasurementUtil.h"
#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"
#include "ActsEvent/TrackContainerHandlesHelper.h"
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

// ---------------- Initialize ----------------
StatusCode HGTDTrackExtensionAlg::initialize()
{
  ATH_MSG_DEBUG("Initializing " << name() << "...");

  ATH_CHECK(m_trackParticleContainerName.initialize());
  ATH_CHECK(m_HGTDClusterContainerName.initialize());

  // Initialize all WriteDecorHandleKeys
  ATH_CHECK(m_layerHasExtensionKey.initialize());
  ATH_CHECK(m_layerExtensionChi2Key.initialize());
  ATH_CHECK(m_layerClusterRawTimeKey.initialize());
  ATH_CHECK(m_layerClusterTimeKey.initialize());
  ATH_CHECK(m_extrapXKey.initialize());
  ATH_CHECK(m_extrapYKey.initialize());
  ATH_CHECK(m_numHGTDHitsKey.initialize());

  ATH_CHECK(m_actsTrackLinkKey.initialize());
  
  // Initialize HGTD-extension related keys
  ATH_CHECK(m_uncalibratedMeasurementContainerKey_HGTD.initialize());

  // Checks for logger
  ATH_CHECK(m_trackingGeometryTool.retrieve());
  ATH_CHECK(m_extrapolationTool.retrieve());
  
  ATH_CHECK(m_monTool.retrieve(EnableTool{not m_monTool.empty()}));
  ATH_CHECK(m_trackStatePrinter.retrieve(EnableTool{not m_trackStatePrinter.empty()}));
  ATH_CHECK(m_pixelCalibTool.retrieve(EnableTool{not m_pixelCalibTool.empty()}));
  ATH_CHECK(m_stripCalibTool.retrieve(EnableTool{not m_stripCalibTool.empty()}));
  ATH_CHECK(m_hgtdCalibTool.retrieve(EnableTool{not m_hgtdCalibTool.empty()}));

  //Initialize logger
  m_logger = makeActsAthenaLogger(this, "Acts");

  // Initialize surface accessor
  m_surfAcc = ActsTrk::detail::xAODUncalibMeasSurfAcc{m_trackingGeometryTool.get()};


  auto magneticField = std::make_unique<ATLASMagneticFieldWrapper>();
  std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry = m_trackingGeometryTool->trackingGeometry();

  detail::Stepper stepper(std::move(magneticField));
  detail::Navigator::Config cfg{trackingGeometry};
  cfg.resolvePassive = true;
  cfg.resolveMaterial = true;
  cfg.resolveSensitive = true;
  detail::Navigator navigator(cfg, logger().cloneWithSuffix("Navigator"));
  detail::Propagator propagator(std::move(stepper), std::move(navigator), logger().cloneWithSuffix("Prop"));

  // Using the CKF propagator as extrapolator
  detail::Extrapolator extrapolator = propagator;

  // Configure track selector for extension
  Acts::TrackSelector::EtaBinnedConfig trackSelectorCfg(std::vector<double>({0, 4}));  //abs value here
  trackSelectorCfg.cutSets[0].ptMin = 500;        
  trackSelectorCfg.cutSets[0].ptMax = 1000000;    
  trackSelectorCfg.cutSets[0].minMeasurements = 1; 
  trackSelectorCfg.cutSets[0].maxHoles = 4;     
  trackSelectorCfg.cutSets[0].maxOutliers = 4;    
  trackSelectorCfg.cutSets[0].maxHolesAndOutliers = 4; 
  trackSelectorCfg.cutSets[0].maxSharedHits = 4; 
  trackSelectorCfg.cutSets[0].maxChi2 = 10000000.0;     

  ATH_MSG_DEBUG(trackSelectorCfg);

  detail::CKF_config ckfConfig{
      std::move(extrapolator),
      detail::CKF{std::move(propagator), logger().cloneWithSuffix("CKF")},
      {},
      Acts::TrackSelector{trackSelectorCfg}};

  m_trackFinder = std::make_unique<detail::CKF_config>(std::move(ckfConfig));

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
  SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<bool>> layerHasExtensionHandle(m_layerHasExtensionKey, ctx);
  SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> layerExtensionChi2Handle(m_layerExtensionChi2Key, ctx);
  SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> layerClusterRawTimeHandle(m_layerClusterRawTimeKey, ctx);
  SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> layerClusterTimeHandle(m_layerClusterTimeKey, ctx);
  SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> extrapXHandle(m_extrapXKey, ctx);
  SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> extrapYHandle(m_extrapYKey, ctx);
  SG::WriteDecorHandle<xAOD::TrackParticleContainer, int> numHGTDHitsHandle(m_numHGTDHitsKey, ctx);

  SG::ReadDecorHandle<xAOD::TrackParticleContainer, ElementLink<ActsTrk::TrackContainer>> actsTrackLink( m_actsTrackLinkKey, ctx );
  ATH_CHECK( actsTrackLink.isValid() );
  // ================================================== //
  // ============ RETRIEVE MEASUREMENTS =============== //
  // ================================================== //

  ATH_MSG_DEBUG("Reading input collection with key " << m_uncalibratedMeasurementContainerKey_HGTD.key());

  const xAOD::UncalibratedMeasurementContainer* uncalibratedMeasurementContainer{nullptr};
  ATH_CHECK(SG::get(uncalibratedMeasurementContainer ,m_uncalibratedMeasurementContainerKey_HGTD, ctx));
  ATH_MSG_DEBUG("Retrieved " << uncalibratedMeasurementContainer->size()
                  << " input elements from key " << m_uncalibratedMeasurementContainerKey_HGTD.key());

  // Modern approach uses surface accessor instead of detector element map

  ATH_MSG_DEBUG("Investigating HGTD geometry structure");

  Acts::GeometryContext geoContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);

   
  // Get a list of all identifiers associated with HGTD clusters
  if (msgLvl(MSG::DEBUG)) { // DEBUG
    const xAOD::HGTDClusterContainer* hgtdInitHandle{nullptr};
    ATH_CHECK(SG::get(hgtdInitHandle, m_HGTDClusterContainerName, ctx));
    
    std::unordered_set<uint32_t> hgtdVolumes {};
    std::unordered_map<uint32_t, std::unordered_set<uint32_t>> hgtdLayers {}; // volume -> layers
        
    for (const xAOD::HGTDCluster* cluster : *hgtdInitHandle) {
      const Acts::Surface* surface = m_surfAcc.get(cluster);
      
      if (surface) {
	Acts::GeometryIdentifier geoID = surface->geometryId();
	uint32_t vol = geoID.volume();
	uint32_t layer = geoID.layer();
	
	hgtdVolumes.insert(vol);
	hgtdLayers[vol].insert(layer);
	
      }
      else {
	ATH_MSG_ERROR("Failed to retrieve surface for cluster " << cluster->index());
	return StatusCode::FAILURE;
      }
    }
    
    ATH_MSG_DEBUG("Found " << hgtdVolumes.size() << " different HGTD volumes");
    for (std::uint32_t vol : hgtdVolumes) {
      ATH_MSG_DEBUG("HGTD Volume " << vol << " has " << hgtdLayers[vol].size() << " layers:");
      for (std::uint32_t lyr : hgtdLayers[vol]) {
      	ATH_MSG_DEBUG(" - Layer " << lyr);
      }
    }
  } // DEBUG
  
  detail::TrackFindingMeasurements measurements(1ul); // only one measurement collection: HGTD clusters
  ATH_CHECK( collectMeasurements(ctx, measurements) );

  using DefaultTrackStateCreator = Acts::TrackStateCreator<ActsTrk::detail::UncalibSourceLinkAccessor::Iterator,detail::RecoTrackContainer>;

  ActsTrk::detail::UncalibSourceLinkAccessor slAccessor(measurements.measurementRanges());  
  DefaultTrackStateCreator::SourceLinkAccessor slAccessorDelegate;
  slAccessorDelegate.connect<&ActsTrk::detail::UncalibSourceLinkAccessor::range>(&slAccessor);

  // acts_tracking_geometry is already declared in processTrackExtension function

  ATH_MSG_DEBUG("Create " << uncalibratedMeasurementContainer->size()
  << " source links from measurements in "
  << m_uncalibratedMeasurementContainerKey_HGTD.key());

  if (m_trackStatePrinter.isSet()) {
    m_trackStatePrinter->printMeasurements(ctx, 
    {uncalibratedMeasurementContainer}, //wrap in a braced initializer list to make a vector
    measurements.measurementOffsets());
  }

  Acts::PropagatorPlainOptions plainOptions(geoContext, mfContext);
  plainOptions.direction = Acts::Direction::Forward(); 
  
  HGTDTrackExtensionAlg::CKFOptions options(geoContext,
					    mfContext,
					    m_calibrationContext,
					    //slAccessorDelegate,
					    m_trackFinder->ckfExtensions,
					    plainOptions);

  auto calibrator = detail::OnTrackCalibrator<detail::RecoTrackStateContainer>(m_trackingGeometryTool.get(),
									       m_pixelCalibTool,
									       m_stripCalibTool,
									       m_hgtdCalibTool);
  
  DefaultTrackStateCreator defaultTrackStateCreator{};
  defaultTrackStateCreator.sourceLinkAccessor = slAccessorDelegate;
  defaultTrackStateCreator.calibrator.template connect<&detail::OnTrackCalibrator<detail::RecoTrackStateContainer>::calibrate>(&calibrator);
  
  options.extensions.createTrackStates.template connect<
    &DefaultTrackStateCreator::createTrackStates>(&defaultTrackStateCreator);
  
  // ================================================== //
  // ============ LOOP OVER TRACKPARTICLES ============ //
  // ================================================== //

  // Loop over each track particle and decorate it with various information
  for (const xAOD::TrackParticle* trackParticle : *trackParticles) {
    // Default to empty track data
    TrackExtensionData trackData;
    // Check if the TrackParticle has a link to an ACTS track
    ElementLink<ActsTrk::TrackContainer> link_to_track = actsTrackLink(*trackParticle);
    if (!link_to_track.isValid()) {
      ATH_MSG_ERROR("Invalid ACTS track link for TrackParticle " << trackParticle->index());
      return StatusCode::FAILURE;
    }

    std::optional<ActsTrk::TrackContainer::ConstTrackProxy> optional_track = *link_to_track;
    if (!optional_track.has_value()) {
      ATH_MSG_ERROR("No valid ACTS track associated with TrackParticle " << trackParticle->index());
      return StatusCode::FAILURE;
    }

    const ActsTrk::TrackContainer::ConstTrackProxy& track = optional_track.value();

    Acts::VectorTrackContainer trackBackend;
    Acts::VectorMultiTrajectory trackStateBackend;
    detail::RecoTrackContainer tracksContainerTemp(trackBackend, trackStateBackend);

    // Retrieve quantities for ACTS track
    float trackEta = Acts::VectorHelpers::eta(track.momentum());

    // Check eta coverage first
    if (std::abs(trackEta) < m_minEtaAcceptance or
	std::abs(trackEta) > m_maxEtaAcceptance) {
      ATH_MSG_DEBUG("!!!!! ------  Track eta " << trackEta
		    << " outside eta range [" << m_minEtaAcceptance.value() << ", " << m_maxEtaAcceptance.value()
		    << "], skipping extension  ------ !!!!!");

      // set default values
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
    
    ATH_MSG_DEBUG("TrackParticle " << trackParticle->index() << " has ACTS track with eta: " << trackEta << ", phi = " << trackPhi << "  pT: " << trackpT << " and nMeasurements: " << trackNmeasurements);

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

    // Now use the *last measurement parameters* parameters for the CKF
    auto result = m_trackFinder->ckf.findTracks(lastMeasurementStateParameters, options,tracksContainerTemp);

    ATH_MSG_DEBUG("Built " << tracksContainerTemp.size() << " tracks from it");

    for (const detail::RecoTrackContainer::TrackProxy trackProxy : tracksContainerTemp) {
      trackData = processTrackExtension(ctx, trackParticle, trackProxy);
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

  
  return StatusCode::SUCCESS;
}


StatusCode
HGTDTrackExtensionAlg::collectMeasurements(const EventContext& context,
					   detail::TrackFindingMeasurements& measurements) const {
 
  SG::ReadHandle<xAOD::HGTDClusterContainer> HGTDClustersHandle = SG::makeHandle(m_HGTDClusterContainerName, context);
  ATH_CHECK( HGTDClustersHandle.isValid() );
  const xAOD::HGTDClusterContainer* HGTDClusters = HGTDClustersHandle.cptr();
  
  ATH_MSG_DEBUG("Measurements (HGTD only) size: " << HGTDClustersHandle->size());

  if (not m_monTool.empty()) {
    {
      auto mon_nclusters = Monitored::Scalar("n_hgtd_clusters", HGTDClusters->size());
      auto mon = Monitored::Group(m_monTool, mon_nclusters);
    }
    
    // Add cluster position to monitoring tool
    for (const xAOD::HGTDCluster* cluster : *HGTDClustersHandle) {
      const Acts::Surface* surface = m_surfAcc.get(cluster);      
      if (not surface) continue;
      
      Acts::Vector3 globalPos = surface->center(m_trackingGeometryTool->getGeometryContext(context).context());
      
      // Get the time from local position (3rd coordinate)
      auto localPosition = cluster->localPosition<3>();  // Get 3D local position
      double clusterTime = localPosition[2];  // Time is in the third coordinate
      
      ATH_MSG_DEBUG("HGTD Cluster: "<< Amg::toString(globalPos) );

      auto mon_cluster_x = Monitored::Scalar("cluster_x", globalPos.x());
      auto mon_cluster_y = Monitored::Scalar("cluster_y", globalPos.y());
      auto mon_cluster_z = Monitored::Scalar("cluster_z", globalPos.z());
      auto mon_cluster_t = Monitored::Scalar("cluster_t", clusterTime);
      auto mon = Monitored::Group(m_monTool,
				  mon_cluster_x, mon_cluster_y, mon_cluster_z,
				  mon_cluster_t);
    }
  } // MONITORING

  measurements.addMeasurements(0,
			       *HGTDClusters,
			       *m_trackingGeometryTool->surfaceIdMap());

  return StatusCode::SUCCESS;
}

HGTDTrackExtensionAlg::TrackExtensionData HGTDTrackExtensionAlg::processTrackExtension(
  const EventContext& ctx,
  const xAOD::TrackParticle* trackParticle,
  const detail::RecoTrackContainer::TrackProxy& trackProxy) const {

    TrackExtensionData data;

    // Modern approach uses surface accessor instead of detector element map
    
    // Apply track smoothing before trying to access chi2 values
    Acts::GeometryContext geoContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
    const Acts::TrackingGeometry* acts_tracking_geometry = m_trackingGeometryTool->trackingGeometry().get();
  
    
    // Count measurements, holes, and HGTD hits specifically
    std::size_t nMeasurements = 0;
    std::size_t nHoles = 0;
    std::size_t nOutliers = 0;
    std::size_t nHGTDHits = 0;
    
    std::vector<bool> hasHitInLayer = {false, false, false, false};
    std::vector<float> chi2PerLayer = {0.0, 0.0, 0.0, 0.0};
    std::vector<float> timePerLayer = {0.0, 0.0, 0.0, 0.0};
    std::vector<float> rawTimePerLayer = {0.0, 0.0, 0.0, 0.0};

    std::vector<int> truthClassPerLayer = {-1, -1, -1, -1};
    std::vector<bool> isShadowedPerLayer = {false, false, false, false};
    std::vector<bool> isMergedPerLayer = {false, false, false, false};
    std::vector<bool> primaryExpectedPerLayer = {false, false, false, false};
    
    // Extrapolated position - get the position at the first HGTD surface encountered
    float extrapX = 0.0;
    float extrapY = 0.0;
    float extrapZ = 0.0;
    bool foundExtrapolation = false;
    
    trackProxy.container().trackStateContainer().visitBackwards(
      trackProxy.tipIndex(),
        [&](const auto& state) {
            auto flags = state.typeFlags();
            if (flags.test(Acts::TrackStateFlag::HoleFlag)) {
                nHoles++;
            } else if (flags.test(Acts::TrackStateFlag::OutlierFlag)) {
                nOutliers++;
            } else if (flags.test(Acts::TrackStateFlag::MeasurementFlag)) {
                nMeasurements++;
                
                // Check if this is an HGTD hit
                if (state.hasReferenceSurface() && flags.test(Acts::TrackStateFlag::MeasurementFlag)) {
                    const auto& surface = state.referenceSurface();
                    Acts::GeometryIdentifier geoID = surface.geometryId();
                    
                    if (isHGTDSurface(geoID)) {
		        std::size_t layerIndex = getHGTDLayerIndex(geoID);

                        const auto& calibrated = state.template calibrated<3>(); //x,y,time
                        const auto& predicted = state.predicted(); //6D
                        const auto& calibCov = state.template calibratedCovariance<3>(); // Full 3D covariance



			Eigen::Vector2d residual2d;
                        residual2d(0) = calibrated(0) - predicted(Acts::eBoundLoc0);
                        residual2d(1) = calibrated(1) - predicted(Acts::eBoundLoc1);

                        // Extract the top-left 2x2 from the 3x3 measurement covariance
                        AmgSymMatrix(2) cov_2d{calibCov.template block<2,2>(0,0)};
                        
                        // Get the predicted covariance for residual calculation
                        const auto& predictedCov = state.predictedCovariance();
                        AmgSymMatrix(2) predicted_cov_2d{predictedCov.template block<2,2>(0,0)};
                        
                        // Total residual covariance is measurement + predicted covariances
                        AmgSymMatrix(2) residual_cov = cov_2d + predicted_cov_2d;
                                            


                        double chi2=0.0;
                        double ndf = 2.0;
                        if (residual_cov.determinant() != 0) 
                        {
                          chi2 = residual2d.transpose() * residual_cov.inverse() * residual2d;
                        }
                        else
                        {
                          chi2=-99.9;
                        }

                        if (layerIndex < 4) { 
                            nHGTDHits++;
                            hasHitInLayer[layerIndex] = true;
                            chi2PerLayer[layerIndex] =chi2/ndf; //state.chi2();
                            
              // Get the measured time from the calibrated 3D measurement (local x, y, time)
              
                float rawTime = 0.0f;
                float calibratedTime = 0.0f;
                
                if (state.hasCalibrated()) {
                // Extract time from calibrated data
                  try {
                    const auto& calibrated = state.template calibrated<3>();
                    calibratedTime = static_cast<float>(calibrated(2));
                    ATH_MSG_DEBUG("Got time from calibrated<3>: " << calibratedTime);
                  } catch (const std::exception& e) {
                    ATH_MSG_WARNING("Failed to extract time from calibrated<3>: " << e.what());
                  
                  }
                }
                
                // Extract raw time from HGTD clusters
                const xAOD::HGTDCluster* cluster = getHGTDClusterFromState(state);

                if (cluster) {
                  auto localPos = cluster->localPosition<3>();
                  rawTime = static_cast<float>(localPos[2]);
                  ATH_MSG_DEBUG("Got raw time from cluster local position: " << rawTime);
                } else {
                  ATH_MSG_WARNING("Could not get cluster from state");
                }

                // Store the raw time
                rawTimePerLayer[layerIndex] = rawTime;
                

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
              }
              else
              {
                ATH_MSG_DEBUG("State does not have calibrated data");
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
    }
      
    });
    
    // If we didn't find any HGTD hits but we still have a valid bestTrack,
    // we should try to predict where the track would intersect HGTD
    if (nHGTDHits == 0 && !foundExtrapolation) {
        
        // Try to calculate the extrapolation position using your existing method
        if (getExtrapolationPosition(ctx, trackProxy, extrapX, extrapY, extrapZ)) {
            ATH_MSG_DEBUG("!!! Didn't find any HGTD hits but calculated extrapolation position: x=" << extrapX 
                          << ", y=" << extrapY << ", z=" << extrapZ);
            foundExtrapolation = true;
        }
    }
    
    ATH_MSG_DEBUG("Track Statistics: "
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

std::size_t HGTDTrackExtensionAlg::getHGTDLayerIndex(const Acts::GeometryIdentifier& geoID) const {
  // Get volume and layer ID
  std::uint32_t volume = geoID.volume();
  std::uint32_t layer = geoID.layer();
  
  // Check if we're in the positive or negative endcap 
  bool isPositiveEndcap = (volume == 25); 
  bool isNegativeEndcap = (volume == 2); 
  
  // Different mapping for different sides to maintain consistent physical ordering
  if (isPositiveEndcap) {
    // Mapping for positive endcap
    switch(layer) {
      case 2: return 0;  // First HGTD layer (closest to IP)
      case 4: return 1;  // Second HGTD layer
      case 6: return 2;  // Third HGTD layer
      case 8: return 3;  // Fourth HGTD layer (farthest from IP)
      default: return 99; // Invalid layer
    }
  } else if (isNegativeEndcap) {
    // Mapping for negative endcap - potentially different ordering
    switch(layer) {
      case 2: return 3; 
      case 4: return 2;  
      case 6: return 1;
      case 8: return 0;
      default: return 99; // Invalid layer
    }
  } else {
    return 99; // Not an HGTD volume
  }
}

bool HGTDTrackExtensionAlg::getExtrapolationPosition(
  const EventContext& ctx,
  const detail::RecoTrackContainer::TrackProxy& track, 
  float& x, float& y, float& z) const {
  
  // Default values
  x = 0.0;
  y = 0.0;
  z = 0.0;
  
  // Get the position at the first HGTD layer
  bool foundHGTDSurface = false;
  
  // Explicitly capture ctx and geoContext in the lambda
  Acts::GeometryContext geoContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  track.container().trackStateContainer().visitBackwards(
      track.tipIndex(),
      [this, &foundHGTDSurface, &x, &y, &z, geoContext](const auto& state) {
          // Skip if already found or no reference surface
          if (foundHGTDSurface || !state.hasReferenceSurface()) {
              return;
          }
          
          const auto& surface = state.referenceSurface();
          Acts::GeometryIdentifier geoID = surface.geometryId();
          
          // Check if this is an HGTD surface
          if (isHGTDSurface(geoID)) {
              // Instead of getting the surface center, use the predicted parameters
              if (state.hasPredicted()) {
                  // Get the local predicted parameters
                  const auto& predicted = state.predicted();
                  
                  // Get the local predicted position (first two components of the predicted vector)
                  // Local coordinates: [loc0, loc1, phi, theta, q/p, time]
                  Acts::Vector2 localPos(predicted[Acts::eBoundLoc0], predicted[Acts::eBoundLoc1]);
                  
                  // Transform to global coordinates
                  Acts::Vector3 globalPos = surface.localToGlobal(
                      geoContext,
                      localPos,
                      Acts::Vector3::Zero());  // Direction doesn't matter for position
                  
                  // Store coordinates
                  x = globalPos.x();
                  y = globalPos.y();
                  z = globalPos.z();
                  
                  ATH_MSG_DEBUG("Predicted position on HGTD surface: (" << x << ", " << y << ", " << z << ")");
                  foundHGTDSurface = true;
              } else {
                  // Fallback to surface center if predicted parameters not available
                  Acts::Vector3 globalPos = surface.center(geoContext);
                  x = globalPos.x();
                  y = globalPos.y();
                  z = globalPos.z();
                  
                  ATH_MSG_DEBUG("Fallback to surface center for HGTD: (" << x << ", " << y << ", " << z << ")");
                  foundHGTDSurface = true;
              }
          }
      });
  
  return foundHGTDSurface;
}

// Helper function to check if a surface is in HGTD
bool HGTDTrackExtensionAlg::isHGTDSurface(const Acts::GeometryIdentifier& geoID) const {
  // Get volume and layer
  std::uint32_t volume = geoID.volume();
  std::uint32_t layer = geoID.layer();
  
  // Check if it's one of the known HGTD volumes
  if (volume == 2 || volume == 25) {
    // Check if it's one of the HGTD layers
    if (layer == 2 || layer == 4 || layer == 6 || layer == 8) {
      return true;
    }
  }
  
  return false;
}



std::pair<float, float> HGTDTrackExtensionAlg::correctTOF(
  const xAOD::TrackParticle* trackParticle,
  const xAOD::HGTDCluster* cluster,
  float measuredTime,
  float measuredTimeErr,
  const Acts::TrackingGeometry* /*trackingGeometry*/,
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
  
  //option 2 - use perigee - this is what is done in legacy code: https://gitlab.cern.ch/atlas/athena/-/blob/main/HighGranularityTimingDetector/HGTD_Reconstruction/HGTD_RecTools/src/StraightLineTOFcorrectionTool.cxx
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

const xAOD::HGTDCluster* HGTDTrackExtensionAlg::getHGTDClusterFromState(const ActsTrk::detail::RecoConstTrackStateContainerProxy& state) const {
  if (state.hasUncalibratedSourceLink()) {
    auto sl = state.getUncalibratedSourceLink().template get<ATLASUncalibSourceLink>();
    assert( sl != nullptr);
    const xAOD::UncalibratedMeasurement &uncalib_cluster = getUncalibratedMeasurement(sl);
    xAOD::UncalibMeasType clusterType = uncalib_cluster.type();

    if (clusterType == xAOD::UncalibMeasType::HGTDClusterType) {
      ATH_MSG_DEBUG("Found HGTD cluster in source link");
      auto hgtdCluster = static_cast<const xAOD::HGTDCluster *>(&uncalib_cluster);
      return hgtdCluster;
    } else {
      ATH_MSG_DEBUG("Source link contains non-HGTD measurement type: " << static_cast<int>(clusterType));
    }

      // If we have a reference surface, try to match by position
      if (state.hasReferenceSurface()) {
        const auto& surface = state.referenceSurface();
        Acts::GeometryIdentifier geoID = surface.geometryId();
        
        // Check if this is an HGTD surface
        if (isHGTDSurface(geoID)) {
          ATH_MSG_DEBUG("This is an HGTD surface with ID: " << geoID.volume() << ":" << geoID.layer());
          
          // Get the current context
          EventContext ctx = Gaudi::Hive::currentContext();
          
          // Modern approach uses surface accessor instead of detector element map
          
          // Get the HGTD clusters
          SG::ReadHandle<xAOD::HGTDClusterContainer> hgtdClusters(m_HGTDClusterContainerName, ctx);
          if (!hgtdClusters.isValid()) {
            ATH_MSG_WARNING("Failed to retrieve HGTD clusters");
            return nullptr;
          }
          
          // Get global position of the state surface
          const Acts::GeometryContext& geoContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
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
    } else {
      ATH_MSG_DEBUG("State doesn't have uncalibrated source link");
    }
  
  return nullptr;
}

} // End namespace ActsTrk
