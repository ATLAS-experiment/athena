/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FPGATrackSimPrototrackFitterAlg.h"

#include "ActsCalibBase/CalibrationContext.h"

constexpr bool enableBenchmark = 
#ifdef BENCHMARK_FPGATRACKSIM
    true;
#else
    false;
#endif

StatusCode FPGATrackSim::FPGATrackSimPrototrackFitterAlg::initialize() {
  ATH_CHECK(m_trackContainerKey.initialize());
  ATH_CHECK(m_tracksBackendHandlesHelper.initialize(ActsTrk::prefixFromTrackContainerName(m_trackContainerKey.key())));
  ATH_CHECK(m_actsFitter.retrieve()); 
  ATH_CHECK(m_trackingGeometryTool.retrieve());
  ATH_CHECK(m_extrapolationTool.retrieve());
  ATH_CHECK(m_ProtoTrackCollectionFromFPGAKey.initialize());
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
  const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
  const Acts::MagneticFieldContext mfContext = m_extrapolationTool->getMagneticFieldContext(ctx);
  const Acts::CalibrationContext calContext{ActsTrk::getCalibrationContext(ctx)};

  /// ----------------------------------------------------------
  /// and we are back to EF tracking! 
  ActsTrk::MutableTrackContainer trackContainer;
  if constexpr (enableBenchmark) m_chrono->chronoStart("FPGATrackSimPrototrackFitterAlg: ACTS KF");
  // now we fit each of the proto tracks
  for (auto & proto : *myProtoTracks){
    auto res = m_actsFitter->fit(proto.measurements, *proto.parameters,
                                 tgContext, mfContext, calContext);

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


