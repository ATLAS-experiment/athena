/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackingGeometryTool.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometryInterfaces/IDetectorElement.h"

#include "Acts/Geometry/TrackingGeometry.hpp"


namespace ActsTrk {
StatusCode TrackingGeometryTool::initialize() {
    ATH_MSG_DEBUG(name() << " initializing");
    if (parent() != toolSvc()) {
        ATH_MSG_ERROR("The tool is initialized as a private tool but should be public");
        return StatusCode::FAILURE;
    }
    ATH_CHECK(m_trackingGeometrySvc.retrieve());
    
    return StatusCode::SUCCESS;
}

std::shared_ptr<const Acts::TrackingGeometry> TrackingGeometryTool::trackingGeometry() const {
    return m_trackingGeometrySvc->trackingGeometry();
}
const Acts::TrackingVolume* TrackingGeometryTool::getEnvelope(const ActsTrk::SystemEnvelope envType) const {
    return m_trackingGeometrySvc->getEnvelope(envType);
}

const ActsTrk::DetectorElementToActsGeometryIdMap* TrackingGeometryTool::surfaceIdMap() const {
    return m_trackingGeometrySvc->surfaceIdMap();
}

const GeometryContext& TrackingGeometryTool::getNominalGeometryContext() const {
     return m_trackingGeometrySvc->getNominalContext();
}



}