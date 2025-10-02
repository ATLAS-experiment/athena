/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// NavigationLayer.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "TrkGeometry/NavigationLayer.h"

// constructor with arguments
Trk::NavigationLayer::NavigationLayer
    (std::unique_ptr<Trk::Surface> surfaceRepresentation)
    : Trk::Layer(),
      m_surfaceRepresentation(std::move(surfaceRepresentation)) {
      Trk::Layer::m_layerType = Trk::passive;
    }

// constructor with arguments
Trk::NavigationLayer::NavigationLayer(
  std::unique_ptr<Trk::Surface> surfaceRepresentation, double thickness)
  : Trk::Layer(),
  m_surfaceRepresentation(std::move(surfaceRepresentation)) {
    Trk::Layer::m_layerThickness = thickness;
  }

// copy constructor
Trk::NavigationLayer::NavigationLayer(const Trk::NavigationLayer& lay)
    : Trk::Layer(lay),
      m_surfaceRepresentation(lay.m_surfaceRepresentation->uniqueClone()) {
  Trk::Layer::m_previousLayer = lay.m_previousLayer;
  Trk::Layer::m_nextLayer = lay.m_nextLayer;
  Trk::Layer::m_binUtility = lay.m_binUtility;
}

Trk::NavigationLayer& Trk::NavigationLayer::operator=(
    const Trk::NavigationLayer& lay) {
  if (this != &lay) {
    Trk::Layer::operator=(lay);
    m_surfaceRepresentation = lay.m_surfaceRepresentation->uniqueClone();
  }
  return (*this);
}

bool Trk::NavigationLayer::isOnLayer(const Amg::Vector3D& gp,
                                     const BoundaryCheck& bcheck) const {
  return m_surfaceRepresentation->isOnSurface(gp, bcheck);
}

