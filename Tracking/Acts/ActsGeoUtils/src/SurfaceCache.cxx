
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <ActsGeoUtils/SurfaceCache.h>
#include <GeoModelHelpers/throwExcept.h>
#ifndef SIMULATIONBASE

namespace ActsTrk{

  SurfaceCache::SurfaceCache(const TransformCache* transformCache): 
      m_transformCache{transformCache}{}  

  const TransformCache* SurfaceCache::transformCache() const { return m_transformCache; }
  const Acts::Transform3& SurfaceCache::transform(const Acts::GeometryContext& anygctx) const  {
    const ActsGeometryContext* gctx = anygctx.get<const ActsGeometryContext*>();    
    // unpack the alignment store from the context
    return m_transformCache->getTransform(gctx->getStore(m_transformCache->parent()->detectorType()).get());
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
  double SurfaceCache::thickness() const { return 0.; }
  void SurfaceCache::setSurface(std::shared_ptr<Acts::Surface> surface) { m_surface = surface; }
  IdentifierHash SurfaceCache::hash() const { return m_transformCache->hash(); }
}
#endif