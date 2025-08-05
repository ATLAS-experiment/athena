/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "Acts/Geometry/TrackingGeometry.hpp"

#include "TrackContainerReader.h"

namespace ActsTrk{
  
StatusCode TrackContainerReader::initialize()
{
  ATH_CHECK(m_trackingGeometryTool.retrieve());
  ATH_CHECK(m_tracksKey.initialize());
  ATH_CHECK(m_tracksKey.key().find("Tracks") != std::string::npos);
  ATH_CHECK(m_tracksBackendHandlesHelper.initialize(ActsTrk::prefixFromTrackContainerName(m_tracksKey.key())));

  return StatusCode::SUCCESS;
}

StatusCode TrackContainerReader::execute(const EventContext& context) const
{
  std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry = m_trackingGeometryTool->trackingGeometry();
  Acts::GeometryContext geoContext = m_trackingGeometryTool->getGeometryContext(context).context();  

  // Create persistent (i.e. xAOD backended) track collection
  std::unique_ptr<ActsTrk::PersistentTrackContainer> trackContainer = m_tracksBackendHandlesHelper.build(trackingGeometry.get(), geoContext, context);
  ATH_MSG_DEBUG("read track container size " << trackContainer->size());

  // We convert to non-xAOD backend for StoreGate
  // Transient declination
  Acts::VectorTrackContainer trackBackend;
  Acts::VectorMultiTrajectory trackStateBackend;
  ActsTrk::MutableTrackContainer tc( std::move(trackBackend),
                                     std::move(trackStateBackend) );

  // copy
  for ( auto track : *trackContainer ) {
    auto destProxy = tc.makeTrack();
    destProxy.copyFrom(track);
  }
  
  // Constant declination
  Acts::ConstVectorTrackContainer ctrackBackend( std::move(tc.container()) );
  Acts::ConstVectorMultiTrajectory ctrackStateBackend( std::move(tc.trackStateContainer()) );
  std::unique_ptr<ActsTrk::TrackContainer> ctc = std::make_unique<ActsTrk::TrackContainer>( std::move(ctrackBackend),
                                                                                            std::move(ctrackStateBackend) );
  ATH_MSG_DEBUG("store track container size " << ctc->size());
  
  // Store
  auto handle = SG::makeHandle(m_tracksKey, context);
  ATH_CHECK(handle.record(std::move(ctc)));
  return StatusCode::SUCCESS;
}
  
}
