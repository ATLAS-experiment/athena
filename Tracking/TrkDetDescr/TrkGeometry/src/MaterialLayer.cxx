/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MaterialLayer.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "TrkGeometry/MaterialLayer.h"

#include "TrkDetDescrUtils/SharedDoNoDelete.h"
// No ownership of the surface representation
// Expressed via shared_ptr with custom deleter
Trk::MaterialLayer::MaterialLayer(
    Surface& surfaceRepresentation,
    std::unique_ptr<LayerMaterialProperties> mlprop)
    : Trk::Layer(),
    m_surfaceRepresentation(std::shared_ptr<Surface>(&surfaceRepresentation, do_not_delete<Surface>))
{
  m_layerMaterialProperties = std::move(mlprop);
  m_layerThickness = 1.;
}

// Keep ownership of the surface representation
Trk::MaterialLayer::MaterialLayer(
    std::shared_ptr<Surface>&& surfaceRepresentation,
    std::unique_ptr<LayerMaterialProperties> mlprop)
    : Trk::Layer(),
    m_surfaceRepresentation(std::move(surfaceRepresentation)) {
  m_layerMaterialProperties = std::move(mlprop);
  m_layerThickness = 1.;
}

bool Trk::MaterialLayer::isOnLayer(const Amg::Vector3D& gp,
                                   const BoundaryCheck& bcheck) const {
  return m_surfaceRepresentation.get()->isOnSurface(gp, bcheck);
}

