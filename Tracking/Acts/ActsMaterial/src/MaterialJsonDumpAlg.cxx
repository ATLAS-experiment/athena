/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MaterialJsonDumpAlg.h"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "ActsPlugins/Json/TrackingGeometryMaterialJsonConverter.hpp"
#include "Acts/Material/TrackingGeometryMaterial.hpp"

ActsTrk::MaterialJsonDumpAlg::MaterialJsonDumpAlg(const std::string& name, ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode ActsTrk::MaterialJsonDumpAlg::initialize()
{
    ATH_CHECK(m_trackingGeometrySvc.retrieve());
    if (m_materialFractionBits.value() < 0 || m_materialFractionBits.value() > 23) {
        ATH_MSG_ERROR("MaterialFractionBits must be between 0 and 23");
        return StatusCode::FAILURE;
    }
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

    Acts::TrackingGeometryMaterialJsonConverter::Options options;
    options.compressionLevel = m_compressionLevel.value();
    options.materialFractionBits = m_materialFractionBits.value();
    ATH_MSG_INFO("Writing material map to '" << m_fileName.value() << "'");
    Acts::TrackingGeometryMaterialJsonConverter{}.toFile(
        Acts::TrackingGeometryMaterial::fromGeometry(*trackingGeometry), m_fileName.value(), options);

    return StatusCode::SUCCESS;
}
