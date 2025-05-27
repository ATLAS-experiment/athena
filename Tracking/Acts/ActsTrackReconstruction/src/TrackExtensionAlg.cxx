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
#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Utilities/TrackHelpers.hpp"
#include "Acts/TrackFinding/TrackStateCreator.hpp"

// ActsTrk
#include "ActsCalibration/CalibrationContext.h"
#include "ActsCalibration/xAODUncalibMeasSurfAcc.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsInterop/Logger.h"
#include "ActsInterop/TableUtils.h"
#include "src/detail/AtlasMeasurementSelector.h"
#include "src/detail/OnTrackCalibrator.h"
#include "src/detail/TrackFindingMeasurements.h"
#include "src/detail/MeasurementIndex.h"
#include "ActsGeometry/SurfaceOfMeasurementUtil.h"

// STL
#include <initializer_list>
#include <sstream>
#include <functional>
#include <tuple>
#include <utility>
#include <algorithm>


namespace ActsTrk{

  StatusCode TrackExtensionAlg::initialize() {
    ATH_CHECK(m_pixelClusters.initialize());
    ATH_CHECK(m_protoTrackCollectionKey.initialize());
    ATH_CHECK(m_trackContainerKey.initialize());
    ATH_CHECK(m_tracksBackendHandlesHelper.initialize(
        ActsTrk::prefixFromTrackContainerName(m_trackContainerKey.key())));
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
        detail::CKF{std::move(propagator), m_logger->cloneWithSuffix("CKF")},
        {},
        Acts::TrackSelector{trackSelectorCfg}};

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

    const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(context).context();
    const Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(context);
    const Acts::CalibrationContext calContext{getCalibrationContext(context)};
   
    detail::xAODUncalibMeasSurfAcc surfAcc{m_trackingGeometryTool.get()};

    const auto* detectorElementToGeometryIdMap = m_trackingGeometryTool->surfaceIdMap();
    
    SG::ReadHandle<xAOD::PixelClusterContainer> pixelClustersHandle(m_pixelClusters, context);
    ATH_MSG_DEBUG("Measurements (pixels only) size: " << pixelClustersHandle->size());
    // potential TODO: filtering only certain layers
    detail::TrackFindingMeasurements measurements(1ul /* number of measurement containers*/);
    measurements.addMeasurements(0, *pixelClustersHandle, *detectorElementToGeometryIdMap);
    std::optional<detail::MeasurementIndex> measurementIndex;
    if (m_trackStatePrinter.isSet()) {
      measurementIndex.emplace(1ul);
      measurementIndex->addMeasurements(*pixelClustersHandle);
    }

    using DefaultTrackStateCreator = Acts::TrackStateCreator<ActsTrk::detail::UncalibSourceLinkAccessor::Iterator,detail::RecoTrackContainer>;

    ActsTrk::detail::UncalibSourceLinkAccessor slAccessor(measurements.measurementRanges());
    DefaultTrackStateCreator::SourceLinkAccessor slAccessorDelegate;
    slAccessorDelegate.connect<&ActsTrk::detail::UncalibSourceLinkAccessor::range>(&slAccessor);

    Acts::PropagatorPlainOptions plainOptions(tgContext, mfContext);
    plainOptions.maxSteps = 1000;
    plainOptions.direction= m_propagateForward ? Acts::Direction::Forward() : Acts::Direction::Backward();



    TrackExtensionAlg::CKFOptions options(tgContext,
                      mfContext,
                      calContext,
                      m_ckfConfig->ckfExtensions,
                      plainOptions,
                      perigeeSurface.get());

    options.targetSurface = perigeeSurface.get();                  

    auto calibrator = detail::OnTrackCalibrator<detail::RecoTrackStateContainer>(
       m_trackingGeometryTool.get(),
       m_pixelCalibTool,
       m_stripCalibTool,
       m_hgtdCalibTool);
    DefaultTrackStateCreator defaultTrackStateCreator{};
    defaultTrackStateCreator.sourceLinkAccessor = slAccessorDelegate;
    defaultTrackStateCreator.calibrator.template connect<&detail::OnTrackCalibrator<detail::RecoTrackStateContainer>::calibrate>(&calibrator);

    options.extensions.createTrackStates.template connect<
       &DefaultTrackStateCreator
       ::createTrackStates>(&defaultTrackStateCreator);
    if ( not m_truthParticlesKey.empty() ) {
      auto truthHandle = SG::ReadHandle(m_truthParticlesKey, context);
      for ( auto truthParticle: *truthHandle ) {
        ATH_MSG_DEBUG("truth: eta: " << truthParticle->eta() << " phi: " << truthParticle->phi() << " pt: " << truthParticle->pt());
      }
    }
    ATH_MSG_DEBUG("Size of proto tracks collection " << protoTracksHandle->size());
    for (const ActsTrk::ProtoTrack& protoTrack : *protoTracksHandle) {
      if(protoTrack.measurements.empty()) continue;

      const Acts::Surface* refSurface = surfAcc.get(protoTrack.measurements[0]);

      auto res = m_actsFitter->fit(context, protoTrack.measurements, *protoTrack.parameters,
                                   tgContext, mfContext, calContext, *detectorElementToGeometryIdMap, refSurface);
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
          if (m_trackStatePrinter.isSet()) m_trackStatePrinter->printTrackState(tgContext, state, *measurementIndex, false);
        }
        ATH_MSG_DEBUG("Track has: " << tempTrackProxy.nMeasurements() << " measurements ");
        ATH_MSG_DEBUG("track: eta: " <<  -1 * log(tan( tempTrackProxy.theta() * 0.5)) << " phi: " << tempTrackProxy.phi() << " pt:" << abs(1./tempTrackProxy.qOverP() * sin(protoTrack.parameters->theta())));

        destTrackProxy.copyFrom(tempTrackProxy);
      }
    }

    std::unique_ptr<ActsTrk::TrackContainer> constTracksContainer =
        m_tracksBackendHandlesHelper.moveToConst(
            std::move(trackContainer),
            tgContext,
            context);
    SG::WriteHandle<ActsTrk::TrackContainer> trackContainerHandle(m_trackContainerKey, context);
    ATH_CHECK(trackContainerHandle.record(std::move(constTracksContainer)));

    return StatusCode::SUCCESS;
  }

} // EOF namespace
