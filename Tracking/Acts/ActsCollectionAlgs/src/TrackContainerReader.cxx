/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "Acts/Geometry/TrackingGeometry.hpp"

#include "TrackContainerReader.h"

namespace ActsTrk{
TrackContainerReader::TrackContainerReader(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator)
{
}


StatusCode TrackContainerReader::initialize()
{
  ATH_CHECK(m_trackingGeometryTool.retrieve());
  ATH_CHECK(m_tracksKey.initialize());
  ATH_CHECK(m_tracksKey.key().find("Tracks") != std::string::npos);
  ATH_CHECK(m_tracksBackendHandlesHelper.initialize(ActsTrk::prefixFromTrackContainerName(m_tracksKey.key())));

  return StatusCode::SUCCESS;
}

StatusCode TrackContainerReader::finalize()
{
  return StatusCode::SUCCESS;
}

StatusCode TrackContainerReader::execute(const EventContext& context) const
{
  std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry = m_trackingGeometryTool->trackingGeometry();
  Acts::GeometryContext geoContext = m_trackingGeometryTool->getGeometryContext(context).context();  

  std::unique_ptr<ActsTrk::TrackContainer> trackContainer = m_tracksBackendHandlesHelper.build(trackingGeometry.get(), geoContext, context);
  ATH_MSG_DEBUG("track container size " << trackContainer->size());
  auto handle = SG::makeHandle(m_tracksKey, context);
  ATH_CHECK(handle.record(std::move(trackContainer)));
  return StatusCode::SUCCESS;
}
}
