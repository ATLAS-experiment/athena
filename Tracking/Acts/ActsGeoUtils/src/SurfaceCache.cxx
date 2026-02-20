
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include <ActsGeoUtils/SurfaceCache.h>
#ifndef SIMULATIONBASE
#include <Acts/Surfaces/Surface.hpp>
#include <GeoModelKernel/throwExcept.h>

namespace ActsTrk{

  SurfaceCache::SurfaceCache(const TransformCache* transformCache): 
      m_transformCache{transformCache}{}  

  const TransformCache* SurfaceCache::transformCache() const { return m_transformCache; }
  const Acts::Transform3& SurfaceCache::localToGlobalTransform(const Acts::GeometryContext& anygctx) const  {
    return m_transformCache->localToGlobalTransform(anygctx);
  }
  const Acts::Surface& SurfaceCache::surface() const  { 
    if (!m_surface) THROW_EXCEPTION("Surface has not been set before");
    return *m_surface; 
  }
  Acts::Surface& SurfaceCache::surface() { 
      if (!m_surface) THROW_EXCEPTION("Surface has not been set before");
      return *m_surface; 
  }
  std::shared_ptr<Acts::Surface> SurfaceCache::getSurface() const { return m_surface; }
  void SurfaceCache::setSurface(std::shared_ptr<Acts::Surface> surface) { m_surface = surface; }
  IdentifierHash SurfaceCache::hash() const { return m_transformCache->hash(); }
  Identifier SurfaceCache::identify() const { return m_transformCache->identify(); }
  DetectorType SurfaceCache::detectorType() const { return m_transformCache->detectorType(); }
  bool SurfaceCache::isSensitive() const { return m_transformCache->parent()->isSensitive(); }
}
#endif