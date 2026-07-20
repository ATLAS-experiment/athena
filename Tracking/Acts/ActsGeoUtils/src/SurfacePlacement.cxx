
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include <ActsGeoUtils/SurfacePlacement.h>
#ifndef SIMULATIONBASE
#include <Acts/Surfaces/Surface.hpp>
#include <GeoModelKernel/throwExcept.h>

namespace ActsTrk{
  SurfacePlacement::SurfacePlacement(IReadoutSurfacePositioning* transformCache): 
      m_transformCache{transformCache}{}  

  const IDetectorElement* SurfacePlacement::detectorElement() const { return m_transformCache->parent(); }
  const Acts::Transform3& SurfacePlacement::localToGlobalTransform(const Acts::GeometryContext& tgContext) const  {
    return m_transformCache->getTransform(tgContext);
  }
  const Acts::Surface& SurfacePlacement::surface() const  { 
    return *m_transformCache->m_surface; 
  }
  Acts::Surface& SurfacePlacement::surface() { 
      return *m_transformCache->m_surface; 
  }
  std::shared_ptr<Acts::Surface> SurfacePlacement::getSurface() const { return m_transformCache->m_surface; }
  IdentifierHash SurfacePlacement::hash() const { return m_transformCache->hash(); }
  Identifier SurfacePlacement::identify() const { return m_transformCache->identify(); }
  DetectorType SurfacePlacement::detectorType() const { return m_transformCache->detectorType(); }
}
#endif