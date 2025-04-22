/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PlaneLayer.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkGeometry/PlaneLayer.h"

#include "TrkGeometry/LayerMaterialProperties.h"
#include "TrkGeometry/MaterialProperties.h"
#include "TrkParameters/TrackParameters.h"
// CLHEP
#include "GeoPrimitives/GeoPrimitives.h"

Trk::PlaneLayer::PlaneLayer(const Amg::Transform3D & transform,
                            const Trk::SurfaceBounds* tbounds,
                            const Trk::LayerMaterialProperties& laymatprop,
                            double thickness, std::unique_ptr<Trk::OverlapDescriptor> olap,
                            int laytyp)
    : PlaneSurface(transform, tbounds),
      Layer(laymatprop, thickness, std::move(olap), laytyp) {}

Trk::PlaneLayer::PlaneLayer(Trk::PlaneSurface* plane,
                            const Trk::LayerMaterialProperties& laymatprop,
                            double thickness, std::unique_ptr<Trk::OverlapDescriptor> olap,
                            int laytyp)
    : PlaneSurface(*plane), Layer(laymatprop, thickness, std::move(olap), laytyp) {}

Trk::PlaneLayer::PlaneLayer(
    const Amg::Transform3D & transform,
    const std::shared_ptr<const Trk::SurfaceBounds>& tbounds,
    const Trk::LayerMaterialProperties& laymatprop, double thickness,
    std::unique_ptr<Trk::OverlapDescriptor> olap, int laytyp)
    : PlaneSurface(transform, tbounds),
      Layer(laymatprop, thickness, std::move(olap), laytyp) {}

Trk::PlaneLayer::PlaneLayer(const Trk::PlaneLayer& play) = default;

Trk::PlaneLayer::PlaneLayer(const Trk::PlaneLayer& play,
                            const Amg::Transform3D& transf)
    : PlaneSurface(play, transf), Layer(play) {}

Trk::PlaneLayer& Trk::PlaneLayer::operator=(const PlaneLayer& play) {
  if (this != &play) {
    // call the assignments of the base classes
    Trk::PlaneSurface::operator=(play);
    Trk::Layer::operator=(play);
    m_index = play.m_index;
  }
  return (*this);
}

const Trk::PlaneSurface&
Trk::PlaneLayer::surfaceRepresentation() const
{
  return (*this);
}
Trk::PlaneSurface&
Trk::PlaneLayer::surfaceRepresentation()
{
  return (*this);
}

double Trk::PlaneLayer::preUpdateMaterialFactor(
    const Trk::TrackParameters& parm, Trk::PropDirection dir) const {
  if (!Trk::Layer::m_layerMaterialProperties) return 0.;
  if (Trk::PlaneSurface::normal().dot(dir * parm.momentum().normalized()) > 0.)
    return Trk::Layer::m_layerMaterialProperties->alongPreFactor();
  return Trk::Layer::m_layerMaterialProperties->oppositePreFactor();
}

double Trk::PlaneLayer::postUpdateMaterialFactor(
    const Trk::TrackParameters& parm, Trk::PropDirection dir) const {
  if (!Trk::Layer::m_layerMaterialProperties) return 0.;
  if (Trk::PlaneSurface::normal().dot(dir * parm.momentum().normalized()) > 0.)
    return Trk::Layer::m_layerMaterialProperties->alongPostFactor();
  return Trk::Layer::m_layerMaterialProperties->oppositePostFactor();
}

void Trk::PlaneLayer::moveLayer(Amg::Transform3D& shift) {
  m_transforms =
      std::make_unique<Transforms>(shift * (m_transforms->transform));
}

