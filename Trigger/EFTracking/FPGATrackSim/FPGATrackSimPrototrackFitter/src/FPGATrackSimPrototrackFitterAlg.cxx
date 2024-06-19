/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FPGATrackSimPrototrackFitterAlg.h"


FPGATrackSim::FPGATrackSimPrototrackFitterAlg::FPGATrackSimPrototrackFitterAlg (const std::string& name, ISvcLocator* pSvcLocator ) : AthReentrantAlgorithm( name, pSvcLocator ){
}

StatusCode FPGATrackSim::FPGATrackSimPrototrackFitterAlg::initialize() {
  ATH_CHECK(m_trackContainerKey.initialize());
  ATH_CHECK(m_tracksBackendHandlesHelper.initialize(ActsTrk::prefixFromTrackContainerName(m_trackContainerKey.key())));
  ATH_CHECK(m_actsFitter.retrieve()); 
  ATH_CHECK(m_detEleCollKeys.initialize());
  ATH_CHECK(m_trackingGeometryTool.retrieve());
  ATH_CHECK(m_ATLASConverterTool.retrieve());
  ATH_CHECK(m_extrapolationTool.retrieve());
  ATH_CHECK(m_ProtoTrackCollectionFromFPGAKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode FPGATrackSim::FPGATrackSimPrototrackFitterAlg::execute(const EventContext & ctx) const
{
  
  SG::WriteHandle<ActsTrk::TrackContainer> trackContainerHandle (m_trackContainerKey, ctx);
  SG::ReadHandle<ActsTrk::ProtoTrackCollection> myProtoTracks(m_ProtoTrackCollectionFromFPGAKey,ctx);
  
  if (!myProtoTracks.isValid()){
    ATH_MSG_WARNING("no Prototrack collections"); 
    return StatusCode::SUCCESS;
  }
  ATH_MSG_DEBUG("I received " <<myProtoTracks->size()<<" proto-tracks");


  /// ----------------------------------------------------------
  /// The following block has nothing to do with EF tracking 
  /// directly - it helps us translate the ATLAS surfaces associated
  /// to our clusters to ACTS
  /// For pure EF logic, feel free to ignore until the next divider! 
  ///
  /// The block is borrowed from the ACTS TrackFindingAlg and 
  /// should eventually be retired when this is no longer needed / 
  /// automated. 

  std::vector<const InDetDD::SiDetectorElementCollection *> detEleColl;
  detEleColl.reserve(m_detEleCollKeys.size());
  for (const auto &detEleCollKey : m_detEleCollKeys)
  {
    ATH_MSG_DEBUG("Reading input condition data with key " << detEleCollKey.key());
    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> detEleCollHandle(detEleCollKey, ctx);
    ATH_CHECK(detEleCollHandle.isValid());
    detEleColl.push_back(detEleCollHandle.retrieve());
    if (detEleColl.back() == nullptr)
    {
      ATH_MSG_FATAL(detEleCollKey.fullKey() << " is not available.");
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Retrieved " << detEleColl.back()->size() << " input condition elements from key " << detEleCollKey.key());
  }

  TrackingSurfaceHelper trackingSurfaceHelper;
  for (auto & coll : detEleColl)
  {
    for (const auto *det_el : *coll){
      const Acts::Surface &surface =
          m_ATLASConverterTool->trkSurfaceToActsSurface(det_el->surface());
      xAOD::UncalibMeasType type = xAOD::UncalibMeasType::Other;
      if (det_el->isPixel()) type = xAOD::UncalibMeasType::PixelClusterType;
      else if (det_el->isSCT()) type = xAOD::UncalibMeasType::StripClusterType;
      trackingSurfaceHelper.actsSurfaces(type).push_back(&surface);
    }
  }
  for (const auto & coll : detEleColl)
  {
    xAOD::UncalibMeasType measType = xAOD::UncalibMeasType::Other;
    if (coll->front()->isPixel()) measType = xAOD::UncalibMeasType::PixelClusterType;
    else measType = xAOD::UncalibMeasType::StripClusterType;
    trackingSurfaceHelper.setSiDetectorElements(measType, coll);
  }
  Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  // CalibrationContext converter not implemented yet.
  Acts::CalibrationContext calContext = Acts::CalibrationContext();

  /// ----------------------------------------------------------
  /// and we are back to EF tracking! 
  ActsTrk::MutableTrackContainer trackContainer;

  // now we fit each of the proto tracks
  for (auto & proto : *myProtoTracks){
    auto res = m_actsFitter->fit(ctx, proto.measurements, *proto.parameters,
      m_trackingGeometryTool->getGeometryContext(ctx).context(),
      m_extrapolationTool->getMagneticFieldContext(ctx),
      Acts::CalibrationContext(),
      trackingSurfaceHelper);

    if(!res) continue;
    if (res->size() == 0 ) continue;
    if(proto.measurements.empty()) continue;
    ATH_MSG_DEBUG(".......Done track with size "<< proto.measurements.size());
    const auto trackProxy = res->getTrack(0);
    if (not trackProxy.hasReferenceSurface()) {
      ATH_MSG_INFO("There is not reference surface for this track");
      continue;
    }
    auto destProxy = trackContainer.getTrack(trackContainer.addTrack());
    destProxy.copyFrom(trackProxy, true); // make sure we copy track states!
  }
  std::unique_ptr<ActsTrk::TrackContainer> constTracksContainer = m_tracksBackendHandlesHelper.moveToConst(std::move(trackContainer), 
    m_trackingGeometryTool->getGeometryContext(ctx).context(), ctx);  
  ATH_CHECK(trackContainerHandle.record(std::move(constTracksContainer)));

  return StatusCode::SUCCESS;
}


