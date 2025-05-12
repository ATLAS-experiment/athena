/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FPGATrackSimPrototrackFitterAlg.h"

constexpr bool enableBenchmark = 
#ifdef BENCHMARK_FPGATRACKSIM
    true;
#else
    false;
#endif

FPGATrackSim::FPGATrackSimPrototrackFitterAlg::FPGATrackSimPrototrackFitterAlg (const std::string& name, ISvcLocator* pSvcLocator ) : AthReentrantAlgorithm( name, pSvcLocator ){
}

StatusCode FPGATrackSim::FPGATrackSimPrototrackFitterAlg::initialize() {
  ATH_CHECK(m_trackContainerKey.initialize());
  ATH_CHECK(m_tracksBackendHandlesHelper.initialize(ActsTrk::prefixFromTrackContainerName(m_trackContainerKey.key())));
  ATH_CHECK(m_actsFitter.retrieve()); 
  ATH_CHECK(m_trackingGeometryTool.retrieve());
  ATH_CHECK(m_extrapolationTool.retrieve());
  ATH_CHECK(m_ProtoTrackCollectionFromFPGAKey.initialize());
  ATH_CHECK(m_detectorElementToGeometryIdMapKey.initialize());

  ATH_CHECK(m_chrono.retrieve());
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
  SG::ReadCondHandle<ActsTrk::DetectorElementToActsGeometryIdMap>
     detectorElementToGeometryIdMap{m_detectorElementToGeometryIdMapKey, ctx};
  ATH_CHECK(detectorElementToGeometryIdMap.isValid());

  Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  // CalibrationContext converter not implemented yet.
  Acts::CalibrationContext calContext = Acts::CalibrationContext();

  /// ----------------------------------------------------------
  /// and we are back to EF tracking! 
  ActsTrk::MutableTrackContainer trackContainer;
  if constexpr (enableBenchmark) m_chrono->chronoStart("FPGATrackSimPrototrackFitterAlg: ACTS KF");
  // now we fit each of the proto tracks
  for (auto & proto : *myProtoTracks){
    auto res = m_actsFitter->fit(ctx, proto.measurements, *proto.parameters,
      m_trackingGeometryTool->getGeometryContext(ctx).context(),
      m_extrapolationTool->getMagneticFieldContext(ctx),
      Acts::CalibrationContext(),
      **detectorElementToGeometryIdMap);

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
  if constexpr (enableBenchmark) m_chrono->chronoStop("FPGATrackSimPrototrackFitterAlg: ACTS KF");
  std::unique_ptr<ActsTrk::TrackContainer> constTracksContainer = m_tracksBackendHandlesHelper.moveToConst(std::move(trackContainer), 
    m_trackingGeometryTool->getGeometryContext(ctx).context(), ctx);  
  ATH_CHECK(trackContainerHandle.record(std::move(constTracksContainer)));

  return StatusCode::SUCCESS;
}


