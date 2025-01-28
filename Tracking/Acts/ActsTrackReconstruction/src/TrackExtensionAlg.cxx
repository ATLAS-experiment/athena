/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "src/TrackExtensionAlg.h"
#include "src/TrackFindingAlg.h"
#include "Acts/Propagator/PropagatorOptions.hpp"
#include "src/detail/FitterHelperFunctions.h"

// Athena
#include "AsgTools/ToolStore.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkTrackSummary/TrackSummary.h"
#include "InDetPrepRawData/PixelClusterCollection.h"
#include "InDetPrepRawData/SCT_ClusterCollection.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "InDetRIO_OnTrack/PixelClusterOnTrack.h"
#include "InDetRIO_OnTrack/SCT_ClusterOnTrack.h"

// ACTS
#include "Acts/Definitions/Units.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/TrackFinding/MeasurementSelector.hpp"
#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Utilities/TrackHelpers.hpp"

// ActsTrk
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsInterop/Logger.h"
#include "ActsInterop/TableUtils.h"
#include "src/detail/AtlasMeasurementSelector.h"
#include "src/detail/OnTrackCalibrator.h"
#include "src/detail/TrackFindingMeasurements.h"
#include "src/detail/SharedHitCounter.h"
#include "ActsGeometry/SurfaceOfMeasurementUtil.h"

// STL
#include <initializer_list>
#include <sstream>
#include <functional>
#include <tuple>
#include <utility>
#include <algorithm>


namespace ActsTrk{

  TrackExtensionAlg::TrackExtensionAlg(const std::string& name,
                                      ISvcLocator* pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator) {}

  StatusCode TrackExtensionAlg::initialize() {
    ATH_CHECK(m_pixelClusters.initialize());
    ATH_CHECK(m_protoTrackCollectionKey.initialize());
    ATH_CHECK(m_trackContainerKey.initialize());
    ATH_CHECK(m_tracksBackendHandlesHelper.initialize(
        ActsTrk::prefixFromTrackContainerName(m_trackContainerKey.key())));
    ATH_CHECK(m_detectorElementToGeometryIdMapKey.initialize());
    ATH_CHECK(m_trackingGeometryTool.retrieve());
    ATH_CHECK(m_extrapolationTool.retrieve());
    ATH_CHECK(m_pixelCalibTool.retrieve(EnableTool{not m_pixelCalibTool.empty()}));
    ATH_CHECK(m_stripCalibTool.retrieve(EnableTool{not m_stripCalibTool.empty()}));
    ATH_CHECK(m_hgtdCalibTool.retrieve(EnableTool{not m_hgtdCalibTool.empty()}));
    ATH_CHECK(m_truthParticlesKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_trackStatePrinter.retrieve(EnableTool{not m_trackStatePrinter.empty()}));
    ATH_CHECK(m_actsFitter.retrieve());
    m_logger = makeActsAthenaLogger(this, name());

    auto magneticField = std::make_unique<ATLASMagneticFieldWrapper>();
    std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry = m_trackingGeometryTool->trackingGeometry();

    detail::Stepper stepper(std::move(magneticField));
    detail::Navigator::Config cfg{trackingGeometry};
    cfg.resolvePassive = false;
    cfg.resolveMaterial = true;
    cfg.resolveSensitive = true;
    detail::Navigator navigator(cfg, m_logger->cloneWithSuffix("Navigator"));
    detail::Propagator propagator(std::move(stepper), std::move(navigator), m_logger->cloneWithSuffix("Prop"));

    // Using the CKF propagator as extrapolator
    detail::Extrapolator extrapolator = propagator;

    // most trivial measurement selector
    Acts::MeasurementSelectorCuts measurementSelectorCuts({-4.0, 4.0});
    Acts::MeasurementSelector measurementSelector(measurementSelectorCuts);


    // update once shared code for configuring this is available
    Acts::TrackSelector::EtaBinnedConfig trackSelectorCfg(std::vector<double>({0, 4}));
    trackSelectorCfg.cutSets[0].ptMin = 1000;
    trackSelectorCfg.cutSets[0].ptMax = 1000000;
    trackSelectorCfg.cutSets[0].minMeasurements = 3;
    trackSelectorCfg.cutSets[0].maxHoles = 1;
    trackSelectorCfg.cutSets[0].maxOutliers = 1;
    trackSelectorCfg.cutSets[0].maxSharedHits = 1;
    trackSelectorCfg.cutSets[0].maxChi2 = 25.;

    detail::CKF_config ckfConfig{
        std::move(extrapolator),
        {std::move(propagator), m_logger->cloneWithSuffix("CKF")},
        measurementSelector,
        {},
        trackSelectorCfg};

    m_ckfConfig = std::make_unique<detail::CKF_config>(std::move(ckfConfig));
    return StatusCode::SUCCESS;
  }




  StatusCode TrackExtensionAlg::execute(const EventContext& context) const {
    SG::ReadHandle<ActsTrk::ProtoTrackCollection> protoTracksHandle(m_protoTrackCollectionKey, context);

    // track finding goes here
    ActsTrk::MutableTrackContainer trackContainer;
    Acts::VectorTrackContainer trackBackend;
    Acts::VectorMultiTrajectory trackStateBackend;
    detail::RecoTrackContainer tracksContainerTemp(trackBackend, trackStateBackend);
    std::shared_ptr<Acts::PerigeeSurface> perigeeSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());

    Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(context).context();
    Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(context);

    SG::ReadCondHandle<ActsTrk::DetectorElementToActsGeometryIdMap>
       detectorElementToGeometryIdMap{m_detectorElementToGeometryIdMapKey, context};
    ATH_CHECK(detectorElementToGeometryIdMap.isValid());

    const Acts::TrackingGeometry *
       acts_tracking_geometry = m_trackingGeometryTool->trackingGeometry().get();
    ATH_CHECK( acts_tracking_geometry != nullptr);

    const auto ms = collectMeasurements(context, **detectorElementToGeometryIdMap);
    const auto& [measurements, sharedHits] = ms;

    ActsTrk::detail::UncalibSourceLinkAccessor slAccessor(measurements.measurementRanges());
    Acts::SourceLinkAccessorDelegate<ActsTrk::detail::UncalibSourceLinkAccessor::Iterator> slAccessorDelegate;
    slAccessorDelegate.connect<&ActsTrk::detail::UncalibSourceLinkAccessor::range>(&slAccessor);

    Acts::PropagatorPlainOptions plainOptions(tgContext, mfContext);
    plainOptions.maxSteps = 1000;
    plainOptions.direction= m_propagateForward ? Acts::Direction::Forward : Acts::Direction::Backward;


    TrackExtensionAlg::CKFOptions options(tgContext,
                      mfContext,
                      m_calibrationContext,
                      slAccessorDelegate,
                      m_ckfConfig->ckfExtensions,
                      plainOptions,
                      perigeeSurface.get());

    auto calibrator = detail::OnTrackCalibrator<detail::RecoTrackStateContainer>(
       *acts_tracking_geometry,
       **detectorElementToGeometryIdMap,
       m_pixelCalibTool,
       m_stripCalibTool,
       m_hgtdCalibTool);
    options.extensions.calibrator.connect<&detail::OnTrackCalibrator<detail::RecoTrackStateContainer>::calibrate>(&calibrator);

    if ( not m_truthParticlesKey.empty() ) {
      auto truthHandle = SG::ReadHandle(m_truthParticlesKey, context);
      for ( auto truthParticle: *truthHandle ) {
        ATH_MSG_DEBUG("truth: eta: " << truthParticle->eta() << " phi: " << truthParticle->phi() << " pt: " << truthParticle->pt());
      }
    }
    ATH_MSG_DEBUG("Size of proto tracks collection " << protoTracksHandle->size());
    for (const ActsTrk::ProtoTrack& protoTrack : *protoTracksHandle) {
      if(protoTrack.measurements.empty()) continue;

      const Acts::Surface* refSurface = ActsTrk::getSurfaceOfMeasurement(*acts_tracking_geometry, **detectorElementToGeometryIdMap, *protoTrack.measurements[0]);
//        ActsTrk::getSurfaceOfMeasurement( *m_trackingGeometryTool->trackingGeometry(), **detectorElementToGeometryIdMap, *protoTrack.measurements[0]);

      auto res = m_actsFitter->fit(context, protoTrack.measurements,*protoTrack.parameters,
                                  m_trackingGeometryTool->getGeometryContext(context).context(),
                                  m_extrapolationTool->getMagneticFieldContext(context),
                                  Acts::CalibrationContext(),
                                  **detectorElementToGeometryIdMap, 
                                  refSurface);
      if(!res) continue;
      if (res->size() == 0 ) continue;
      ATH_MSG_DEBUG(".......Done fit of track with "<< protoTrack.measurements.size() << " measurements");
      const auto trackProxy = res->getTrack(0);
      if (not trackProxy.hasReferenceSurface()) {
        ATH_MSG_INFO("There is not reference surface for this track");
        continue;
      }
      Acts::BoundTrackParameters parametersAtRefSurface( trackProxy.referenceSurface().getSharedPtr(), 
                                                          trackProxy.parameters(), 
                                                          trackProxy.covariance(),
                                                          trackProxy.particleHypothesis());



      ATH_MSG_DEBUG("proto track: eta: " <<  -1 * log(tan( parametersAtRefSurface.theta() * 0.5)) << " phi: " << parametersAtRefSurface.phi() << " pt:" << abs(1./protoTrack.parameters->qOverP() * sin(protoTrack.parameters->theta())));
      ATH_MSG_DEBUG("Extending proto track of " << protoTrack.measurements.size() << " measurements");
      auto result = m_ckfConfig->ckf.findTracks(parametersAtRefSurface, options,
                                                       tracksContainerTemp);


      ATH_MSG_DEBUG("Built " << tracksContainerTemp.size() << " tracks from it");
      for (detail::RecoTrackContainer::TrackProxy tempTrackProxy : tracksContainerTemp) {
        ActsTrk::MutableTrackContainer::TrackProxy destTrackProxy = trackContainer.makeTrack();
        ATH_MSG_DEBUG("Reco MTJ size " << trackStateBackend.size() );
        for ( size_t stateIndex=0; stateIndex < trackStateBackend.size(); ++stateIndex) {
          auto state = trackStateBackend.getTrackState(stateIndex);
          if (m_trackStatePrinter.isSet()) m_trackStatePrinter->printTrackState(tgContext, state, sharedHits, false);
        }
        ATH_MSG_DEBUG("Track has: " << tempTrackProxy.nMeasurements() << " measurements ");
        ATH_MSG_DEBUG("track: eta: " <<  -1 * log(tan( tempTrackProxy.theta() * 0.5)) << " phi: " << tempTrackProxy.phi() << " pt:" << abs(1./tempTrackProxy.qOverP() * sin(protoTrack.parameters->theta())));

        destTrackProxy.copyFrom(tempTrackProxy);
      }
    }

    std::unique_ptr<ActsTrk::TrackContainer> constTracksContainer =
        m_tracksBackendHandlesHelper.moveToConst(
            std::move(trackContainer),
            m_trackingGeometryTool->getGeometryContext(context).context(),
            context);
    SG::WriteHandle<ActsTrk::TrackContainer> trackContainerHandle(m_trackContainerKey, context);
    ATH_CHECK(trackContainerHandle.record(std::move(constTracksContainer)));

    return StatusCode::SUCCESS;
  }


  std::pair<detail::TrackFindingMeasurements, detail::SharedHitCounter> TrackExtensionAlg::collectMeasurements(
       const EventContext& context,
       const ActsTrk::DetectorElementToActsGeometryIdMap &detectorElementToGeometryIdMap) const {
    SG::ReadHandle<xAOD::PixelClusterContainer> pixelClustersHandle(m_pixelClusters, context);

    // setup to return both objects with RVO. Note that SharedHitCounter is only used to get hit offsets needed by TrackStatePrinter.
    std::pair<detail::TrackFindingMeasurements, detail::SharedHitCounter> res{{}, {1u /* only one measurement collection: pixel clusters*/}};
    auto& [measurements, sharedHits] = res;
    ATH_MSG_DEBUG("Measurements (pixels only) size: " << pixelClustersHandle->size());
    // potential TODO: filtering only certain layers
    measurements.addMeasurements(0, *pixelClustersHandle, detectorElementToGeometryIdMap);
    sharedHits.addMeasurements(0, *pixelClustersHandle);
    return res;
  }
} // EOF namespace
