/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsGeometry/ActsTrackingGeometryTool.h"

ActsTrackingGeometryTool::ActsTrackingGeometryTool(const std::string& type, const std::string& name,
    const IInterface* parent)
  : base_class(type, name, parent)
{
}

StatusCode
ActsTrackingGeometryTool::initialize()
{
  ATH_MSG_INFO(name() << " initializing");

  ATH_CHECK( m_trackingGeometrySvc.retrieve() );

  ATH_CHECK( m_rchk.initialize() );

  return StatusCode::SUCCESS;
}

std::shared_ptr<const Acts::TrackingGeometry>
ActsTrackingGeometryTool::trackingGeometry() const
{
  return m_trackingGeometrySvc->trackingGeometry();
}

const ActsGeometryContext&
ActsTrackingGeometryTool::getGeometryContext(const EventContext& ctx) const
{
  ATH_MSG_DEBUG("Creating alignment context for event");
  SG::ReadHandle<ActsGeometryContext> rch(m_rchk, ctx);

  if(!rch.isValid()) {
    ATH_MSG_ERROR("Creating alignment context failed: read cond handle invalid!");
  }

  return *rch;
}

const ActsGeometryContext&
ActsTrackingGeometryTool::getNominalGeometryContext() const {

  return m_trackingGeometrySvc->getNominalContext();
}
const ActsGeometryContext& ActsTrackingGeometryTool::getGeometryContext() const {
    return getGeometryContext(Gaudi::Hive::currentContext());
}

