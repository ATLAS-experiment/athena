/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ProtoTrackCreationAndFitAlg.h"

#include "xAODEventInfo/EventInfo.h"
#include <stdlib.h>


ActsTrk::ProtoTrackCreationAndFitAlg::ProtoTrackCreationAndFitAlg (const std::string& name, ISvcLocator* pSvcLocator ) : AthReentrantAlgorithm( name, pSvcLocator ){
}

StatusCode ActsTrk::ProtoTrackCreationAndFitAlg::initialize() {
  ATH_CHECK(m_trackContainerKey.initialize()); 
  ATH_CHECK(m_PixelClusters.initialize()); 
  ATH_CHECK(m_StripClusters.initialize()); 
  ATH_CHECK(m_tracksBackendHandlesHelper.initialize(ActsTrk::prefixFromTrackContainerName(m_trackContainerKey.key())));
  ATH_CHECK(m_actsFitter.retrieve()); 
  ATH_CHECK(m_patternBuilder.retrieve()); 
  ATH_CHECK(m_detEleCollKeys.initialize());
  ATH_CHECK(m_trackingGeometryTool.retrieve());
  ATH_CHECK(m_ATLASConverterTool.retrieve());
  ATH_CHECK(m_extrapolationTool.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode ActsTrk::ProtoTrackCreationAndFitAlg::execute(const EventContext & ctx) const {  

  // Read the pixel and strip cluster list
  SG::ReadHandle<xAOD::PixelClusterContainer> thePixelClusters(m_PixelClusters,ctx); 
  SG::ReadHandle<xAOD::StripClusterContainer> theStripClusters(m_StripClusters,ctx); 
  if (!thePixelClusters.isValid()){
    ATH_MSG_FATAL("no Pixel clusters"); 
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("I found " <<thePixelClusters->size()<<" pix clusters");

  if (!theStripClusters.isValid()){
    ATH_MSG_FATAL("no strip clusters"); 
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("I found " <<theStripClusters->size()<<" strip clusters");

  // book the output tracks
  auto trackContainerHandle = SG::makeHandle(m_trackContainerKey, ctx);

  // call the user-provided track finder 
  std::vector<ActsTrk::ProtoTrack> myProtoTracks; 
  ATH_CHECK(m_patternBuilder->findProtoTracks(ctx,
                  *thePixelClusters,
                  *theStripClusters,
                  myProtoTracks )); 
  ATH_MSG_INFO("I received " <<myProtoTracks.size()<<" proto-tracks");
  

  /// ----------------------------------------------------------
  /// The following block has nothing to do with EF tracking 
  /// directly - it helps us translate the ATLAS surfaces associated
  /// to our clusters to ACTS
  /// For pure EF logic, feel free to ignore until the nex divider! 
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

  std::array<std::vector<const Acts::Surface *>, TrackingSurfaceHelper::s_NMeasTypes> acts_surfaces;
  for (auto & coll : detEleColl)
  {
    for (const auto *det_el : *coll){
      const Acts::Surface &surface =
          m_ATLASConverterTool->trkSurfaceToActsSurface(det_el->surface());
          xAOD::UncalibMeasType type = xAOD::UncalibMeasType::Other; 
          if (det_el->isPixel()) type = xAOD::UncalibMeasType::PixelClusterType;  
          else if (det_el->isSCT()) type = xAOD::UncalibMeasType::StripClusterType;  
          acts_surfaces[static_cast<std::size_t>(type)].push_back(&surface);
    }
  }
  TrackingSurfaceHelper tracking_surface_helper(std::move(acts_surfaces));
  for (const auto & coll : detEleColl)
  {
    xAOD::UncalibMeasType measType = xAOD::UncalibMeasType::Other;
    if (coll->front()->isPixel()) measType = xAOD::UncalibMeasType::PixelClusterType;
    else measType = xAOD::UncalibMeasType::StripClusterType;
    tracking_surface_helper.setSiDetectorElements(measType, coll);
  }
  Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  // CalibrationContext converter not implemented yet.
  Acts::CalibrationContext calContext = Acts::CalibrationContext();

  /// ----------------------------------------------------------
  /// and we are back to EF tracking! 
  ActsTrk::MutableTrackContainer trackContainer;

  // now we fit each of the proto tracks
  for (auto & proto : myProtoTracks){
    auto res = m_actsFitter->fit(ctx, proto.measurements, *proto.parameters, 
											  m_trackingGeometryTool->getGeometryContext(ctx).context(),  m_extrapolationTool->getMagneticFieldContext(ctx), Acts::CalibrationContext(),
											  tracking_surface_helper);

    if(!res) continue;
    ATH_MSG_DEBUG(".......Done track with size "<< proto.measurements.size());
    const auto trackProxy = res->getTrack(0);
    auto destProxy = trackContainer.getTrack(trackContainer.addTrack());
    destProxy.copyFrom(trackProxy, true); // make sure we copy track states!
  }
  std::unique_ptr<ActsTrk::TrackContainer> constTracksContainer = m_tracksBackendHandlesHelper.moveToConst(std::move(trackContainer), ctx);  
  ATH_CHECK(trackContainerHandle.record(std::move(constTracksContainer)));


  return StatusCode::SUCCESS;
}


