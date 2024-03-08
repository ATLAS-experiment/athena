
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <ActsGeoUtils/SurfaceCache.h>
#ifndef SIMULATIONBASE
#define THROW_RUNTIME(message)                         \
    {                                                  \
      std::stringstream except{};                      \
      except<<__FILE__<<":"<<__LINE__<<" ";            \
      except<<message<<std::endl;                      \
      throw std::runtime_error(except.str());          \
    }

namespace ActsTrk{

  SurfaceCache::SurfaceCache(const TransformCache* transformCache, 
                             DetectorType type): 
      m_transformCache{transformCache},
      m_type{type} {}  

  const TransformCache* SurfaceCache::transformCache() const { return m_transformCache; }
  const Acts::Transform3& SurfaceCache::transform(const Acts::GeometryContext& anygctx) const  {
    const ActsGeometryContext* gctx = anygctx.get<const ActsGeometryContext*>();    
    // unpack the alignment store from the context
    ActsGeometryContext::SubDetAlignments::const_iterator itr = gctx->alignmentStores.find(m_type);
    /// If no alignment for Detector technology x is found, parse a nullptr which is equivalent to
    /// invoking the internal cache store
    const AlignmentStore* store{itr == gctx->alignmentStores.end() ? nullptr : itr->second.get()};   
    return m_transformCache->getTransform(store);
  }
  const Acts::Surface& SurfaceCache::surface() const  { 
    if (!m_surface) THROW_RUNTIME("Surface has not been set before");
    return *m_surface; 
  }
  Acts::Surface& SurfaceCache::surface() { 
      if (!m_surface) THROW_RUNTIME("Surface has not been set before");
      return *m_surface; 
  }
  std::shared_ptr<Acts::Surface> SurfaceCache::getSurface() const { return m_surface; }
  double SurfaceCache::thickness() const { return 0.; }
  void SurfaceCache::setSurface(std::shared_ptr<Acts::Surface> surface) { m_surface = surface; }
  IdentifierHash SurfaceCache::hash() const { return m_transformCache->hash(); }
}
#undef THROW_RUNTIME
#endif