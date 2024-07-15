/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackExtensionAlg.h"
#include <memory>
#include "Acts/Definitions/Units.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/TrackFinding/MeasurementSelector.hpp"
#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Utilities/TrackHelpers.hpp"

#include "ActsEvent/ProtoTrack.h"
#include "ActsEvent/TrackContainer.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "StoreGate/ReadCondHandle.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"

#include "TrackFindingData.h"

namespace ActsTrk{

  TrackExtensionAlg::TrackExtensionAlg(const std::string& name,
                                      ISvcLocator* pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator) {}

  StatusCode TrackExtensionAlg::initialize() {
    ATH_CHECK(m_PixelClusters.initialize());
    ATH_CHECK(m_protoTrackCollectionKey.initialize());
    ATH_CHECK(m_trackContainerKey.initialize());
    ATH_CHECK(m_tracksBackendHandlesHelper.initialize(
        ActsTrk::prefixFromTrackContainerName(m_trackContainerKey.key())));
    ATH_CHECK(m_ATLASConverterTool.retrieve());
    ATH_CHECK(m_trackingGeometryTool.retrieve());

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

    m_ckfConfig.reset(new detail::CKF_config{std::move(extrapolator), {std::move(propagator), m_logger->cloneWithSuffix("CKF")}, measurementSelector, {}, {}, {}, trackSelectorCfg});
    // TODO replace these arbitrary settings by config
    m_ckfConfig->pOptions.maxSteps = 100;
    m_ckfConfig->pOptions.direction= Acts::Direction::Backward;


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


    for (const ActsTrk::ProtoTrack& protTrack : *protoTracksHandle) {
      auto result = m_ckfConfig->ckf.findTracks(*protTrack.parameters, buildCKFOptions(context, perigeeSurface.get()),
                                                       tracksContainerTemp);

      for (detail::RecoTrackContainer::TrackProxy tempTrackProxy : tracksContainerTemp) {
        ActsTrk::MutableTrackContainer::TrackProxy destTrackProxy = trackContainer.makeTrack();
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


  detail::TrackFindingMeasurements TrackExtensionAlg::collectMeasurements(const EventContext& context) const {
    SG::ReadHandle<xAOD::PixelClusterContainer> pixelClustersHandle(m_PixelClusters, context);
    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleHandle(m_pixelDetEleCollKey, context);

    detail::TrackFindingMeasurements measurements(pixelClustersHandle->size());
    // potential TODO: filtering only certain layers
    measurements.addMeasurements(0, *pixelClustersHandle, **pixelDetEleHandle,
                                m_ATLASConverterTool);
    return measurements;
  }
  TrackExtensionAlg::CKFOptions TrackExtensionAlg::buildCKFOptions(const EventContext& context, const Acts::PerigeeSurface* perigeeSurface) const {
    Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(context).context();
    Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(context);

    detail::TrackFindingMeasurements measurements = collectMeasurements(context);


    ActsTrk::UncalibSourceLinkAccessor slAccessor(measurements.orderedGeoIds(),
                                                  measurements.measurementRanges());
    Acts::SourceLinkAccessorDelegate<ActsTrk::UncalibSourceLinkAccessor::Iterator> slAccessorDelegate;
    slAccessorDelegate.connect<&ActsTrk::UncalibSourceLinkAccessor::range>(&slAccessor);


    return CKFOptions(tgContext,
                      mfContext,
                      m_calibrationContext,
                      slAccessorDelegate,
                      m_ckfConfig->ckfExtensions,
                      m_ckfConfig->pOptions,
                      perigeeSurface);
  }


} // EOF namespace
