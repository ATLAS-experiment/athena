/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONREADOUTGEOMETRY_SurfaceCache_H
#define MUONREADOUTGEOMETRY_SurfaceCache_H
#ifndef SIMULATIONBASE
#include <ActsGeoUtils/TransformCache.h>
#include <Acts/Geometry/DetectorElementBase.hpp>
#include <Acts/Surfaces/Surface.hpp>

namespace ActsTrk {

    /** @brief: Helper class to connect the aligned transformations of each active sensor(layer) with the Acts::Surfaces.
      * It's actively used in the surface factories of the MuonReadoutElement. The SurfaceCache takes a pointer to the
      * TransformCache managing the local to global transformation of the corresponding layer and the DetectorType. 
      * The latter is needed to pass the proper AlignmentStore from the Geometry context to the transform cache. 
      * Once constructed, the TransformCache is parsed to the surface construction mechanism of the Acts::Surface. The resulting shared_ptr is then parsed to the SurfaceCache.
    */

  class SurfaceCache: public Acts::DetectorElementBase {

    public:
        /** @brief: Standard constructor taking the tranasform cache of the element and the detector type. 
        **/
      SurfaceCache(const TransformCache* transformCache);

      /// Returns the transformation stored in the TransformCache.
      const Acts::Transform3& transform(const Acts::GeometryContext& gctx) const override final;

      /// Returns the dereferenced pointer cache.
      const Acts::Surface& surface() const override final;
      Acts::Surface& surface() override final;

      /// Dummy override to satisfy the interface of the Acts::DetElementBase
      double thickness() const override final;

      /// Cache the pointer to the surface that's constructed from this cache
      void setSurface(const std::shared_ptr<Acts::Surface> surface);

      /// Returns the pointer to the cached surface.
      std::shared_ptr<Acts::Surface> getSurface() const;

      /// Hash of the SurfaceCache which is the same as the one of the TransformCache.
      IdentifierHash hash() const;
      /// Returns the associated transform cache
      const TransformCache* transformCache() const;
    private:
        const TransformCache* m_transformCache{nullptr};
        std::shared_ptr<Acts::Surface> m_surface{nullptr};
  };
  /// Comparison operators
  inline bool operator<(const std::unique_ptr<SurfaceCache>& a,
                        const std::unique_ptr<SurfaceCache>& b) {
      return a->hash() < b->hash();
  }
  inline bool operator<(const IdentifierHash& a, const std::unique_ptr<SurfaceCache>& b) {
      return a < b->hash();
  }
  inline bool operator<(const std::unique_ptr<SurfaceCache>& a, const IdentifierHash& b) {
      return a->hash() < b;
  }
  using SurfaceCacheSet = std::set<std::unique_ptr<SurfaceCache>, std::less<>>;
}
#endif
#endif
