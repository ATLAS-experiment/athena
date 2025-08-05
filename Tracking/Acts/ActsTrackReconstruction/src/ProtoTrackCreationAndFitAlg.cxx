/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ProtoTrackCreationAndFitAlg.h"

#include "ActsCalibBase/CalibrationContext.h"
#include "xAODEventInfo/EventInfo.h"
#include "src/detail/Definitions.h"
#include <stdlib.h>



StatusCode ActsTrk::ProtoTrackCreationAndFitAlg::initialize() {
  ATH_CHECK(m_trackContainerKey.initialize()); 
  ATH_CHECK(m_protoTrackCollectionKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_PixelClusters.initialize()); 
  ATH_CHECK(m_StripClusters.initialize()); 
  ATH_CHECK(m_tracksBackendHandlesHelper.initialize(ActsTrk::prefixFromTrackContainerName(m_trackContainerKey.key())));
  ATH_CHECK(m_actsFitter.retrieve()); 
  ATH_CHECK(m_patternBuilder.retrieve());
  ATH_CHECK(m_trackingGeometryTool.retrieve());
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
  auto myProtoTracks = std::make_unique<ActsTrk::ProtoTrackCollection>(); 
  ATH_CHECK(m_patternBuilder->findProtoTracks(ctx,
                  *thePixelClusters,
                  *theStripClusters,
                  *myProtoTracks )); 
  ATH_MSG_INFO("I received " << myProtoTracks->size() << " proto-tracks");

  /// ----------------------------------------------------------
  /// The following block has nothing to do with EF tracking 
  /// directly - it helps us translate the ATLAS surfaces associated
  /// to our clusters to ACTS
  /// For pure EF logic, feel free to ignore until the nex divider! 
  ///
  /// The block is borrowed from the ACTS TrackFindingAlg and 
  /// should eventually be retired when this is no longer needed / 
  /// automated. 
  const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  const Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  const Acts::CalibrationContext calContext{getCalibrationContext(ctx)};
  
  /// ----------------------------------------------------------
  /// and we are back to EF tracking!
  Acts::VectorTrackContainer trackBackend;
  Acts::VectorMultiTrajectory trackStateBackend;
  ActsTrk::detail::RecoTrackContainer trackContainer( trackBackend, trackStateBackend );

  // now we fit each of the proto tracks
  for (auto & proto : *myProtoTracks){
    auto res = m_actsFitter->fit(proto.measurements,*proto.parameters,
                                 tgContext, mfContext, calContext);

    if(!res) continue;
    if (res->size() == 0 ) continue;
    if(not proto.measurements.size()) continue;
    ATH_MSG_DEBUG(".......Done track with size "<< proto.measurements.size());
    const auto trackProxy = res->getTrack(0);
    if (not trackProxy.hasReferenceSurface()) {
      ATH_MSG_INFO("There is not reference surface for this track");
      continue;
    }
    auto destProxy = trackContainer.getTrack(trackContainer.addTrack());
    destProxy.copyFrom(trackProxy, true); // make sure we copy track states!
    if ( m_copyParametersFromFit ) {
      ATH_MSG_VERBOSE("original " << proto.parameters->parameters());
      proto.parameters = 
      std::make_unique<Acts::BoundTrackParameters>( trackProxy.referenceSurface().getSharedPtr(), 
                                                    trackProxy.parameters(), 
                                                    trackProxy.covariance(),
                                                    trackProxy.particleHypothesis());
      ATH_MSG_VERBOSE("corrected" << proto.parameters->parameters());
    }

  }

  ActsTrk::TrackBackend constTrackBackend( std::move(trackBackend) );
  ActsTrk::TrackStateBackend constTrackStateBackend( std::move(trackStateBackend) );
  std::unique_ptr<ActsTrk::TrackContainer> constTracksContainer = std::make_unique<ActsTrk::TrackContainer>( std::move(constTrackBackend),
                                                                                                             std::move(constTrackStateBackend) );
  ATH_CHECK(trackContainerHandle.record(std::move(constTracksContainer)));

  if (not m_protoTrackCollectionKey.empty()) {
    auto handle =  SG::makeHandle(m_protoTrackCollectionKey, ctx);
    ATH_CHECK(handle.record(std::move(myProtoTracks)));
  }

  return StatusCode::SUCCESS;
}


