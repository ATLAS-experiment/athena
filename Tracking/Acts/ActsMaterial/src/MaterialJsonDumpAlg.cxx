/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MaterialJsonDumpAlg.h"
#include "Acts/Geometry/TrackingGeometry.hpp"

ActsTrk::MaterialJsonDumpAlg::MaterialJsonDumpAlg(const std::string& name, ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode ActsTrk::MaterialJsonDumpAlg::initialize()
{
    ATH_CHECK(m_trackingGeometrySvc.retrieve());
    ATH_CHECK(m_materialDumper.retrieve());
    return StatusCode::SUCCESS;
}

StatusCode ActsTrk::MaterialJsonDumpAlg::execute(const EventContext& /*ctx*/) const
{
    return StatusCode::SUCCESS;
}

StatusCode ActsTrk::MaterialJsonDumpAlg::finalize()
{
    const std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry = m_trackingGeometrySvc->trackingGeometry();
    if (!trackingGeometry) {
        ATH_MSG_ERROR("No tracking geometry available to dump the material from");
        return StatusCode::FAILURE;
    }

    m_materialDumper->dumpGeometryMaterial(m_trackingGeometrySvc->getNominalContext(), *trackingGeometry);

    return StatusCode::SUCCESS;
}
