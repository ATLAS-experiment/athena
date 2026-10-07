/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MaterialDumperTool.h"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/TrackingVolume.hpp"
#include "Acts/Surfaces/Surface.hpp"

StatusCode ActsTrk::MaterialDumperTool::initialize()
{
    ATH_CHECK(m_materialMapWriters.retrieve());
    return StatusCode::SUCCESS;
}

void ActsTrk::MaterialDumperTool::dumpMaterial(const ActsTrk::GeometryContext& gctx,
                                               const Acts::TrackingGeometryMaterial& material) const
{
    for (const auto& materialWriter : m_materialMapWriters) {
        materialWriter->writeMaterial(gctx, material);
    }
}

void ActsTrk::MaterialDumperTool::dumpGeometryMaterial(const ActsTrk::GeometryContext& gctx,
                                                       const Acts::TrackingGeometry& geometry) const
{
    // Collect the material assigned to the geometry, keyed by geometry ID,
    // in the same container the material mapping produces
    Acts::TrackingGeometryMaterial geometryMaterial;
    geometry.visitSurfaces([&geometryMaterial](const Acts::Surface* surface) {
        if (surface->surfaceMaterialSharedPtr()) {
            geometryMaterial.surfaceMaterials[surface->geometryId()] = surface->surfaceMaterialSharedPtr();
        }
    }, false);
    geometry.visitVolumes([&geometryMaterial](const Acts::TrackingVolume* volume) {
        if (volume->hasMaterial()) {
            geometryMaterial.volumeMaterials[volume->geometryId()] = volume->volumeMaterialPtr();
        }
    });

    dumpMaterial(gctx, geometryMaterial);
}
