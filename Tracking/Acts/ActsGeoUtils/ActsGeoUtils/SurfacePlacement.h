/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONREADOUTGEOMETRY_SurfacePlacement_H
#define MUONREADOUTGEOMETRY_SurfacePlacement_H
#ifndef SIMULATIONBASE

#include <GeoPrimitives/GeoPrimitivesHelpers.h>
///
#include <ActsGeoUtils/TransformCache.h>
#include <ActsGeometryInterfaces/ISurfacePlacement.h>

#include "Acts/Surfaces/SurfacePlacementBase.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Surfaces/SurfaceBounds.hpp"

namespace Acts{
    class Surface;
}
namespace ActsTrk {

    /** @brief: Helper class to assign a @Acts::SurfacePlacementBase to the Acts::Surfaces in order
     *          to make them alignable. The class implements the ISurfacePlacement extension
     *          to make the surface's ATLAS identifier available to the user. It uses the 
     *          IReadoutElementPositioning to forward the handling of the alignment. Also, it is used
     *          to take shared ownership of the associated Surface and of the SurfacePlacement 
     *          instance itself.
     * 
     *          The class constructor is protected. Instead the factory method `makeShared` shall
     *          be used to create the SurfacePlacement together with the Acts::Surface to align  */
  class SurfacePlacement final: public ISurfacePlacement {
    public:
      /** @brief Factory method to create a SurfacePlacement together with an Acts::Surface.
       *         The method is templated over the concrete Surface type which is to be instantiated
       *         e.g. Acts::PlaneSurface, and the surface bounds type that's passed to the surface
       *         at its construction. E.g.
       *        
       *            auto bounds = std::make_shared<Acts::RectangleBounds>(5._cm, 5._cm);
       *            auto placement = SurfacePlacement::makeShared<Acts::RectangleSurface>(transformCache, std::move(bounds));
       *        creates the placement, together with the bounds. The passed transform cache automatically takes over ownership.
       *        over the surface and the SurfacePlacement
       *         
       * @param transformCache: The mutable reference taking ownership over the constructed placement and surface
       * @param bounds: The surface bounds with which the surface is instantiated. */
      template <typename Surface_t,
                typename Bounds_t>
      static std::shared_ptr<SurfacePlacement> makeShared(IReadoutSurfacePositioning& transformCache,
                                                          std::shared_ptr<Bounds_t>&& bounds)
                requires(std::is_base_of_v<Acts::SurfaceBounds, Bounds_t>);
 
      virtual ~SurfacePlacement() = default;

      /** @brief Delete the copy consturctor */
      SurfacePlacement(const SurfacePlacement& other) = delete;
      /** @brief The copy assignment operator */
      SurfacePlacement& operator=(const SurfacePlacement& other) = delete;
      /** @brief Delete the move constructor */
      SurfacePlacement(SurfacePlacement&& other) = delete;
      /** @brief Delete the move assignment operator */
      SurfacePlacement& operator=(SurfacePlacement&& other) = delete;
      /** @copydoc Acts::SurfacePlacementBase::localToGlobalTransform */
      const Acts::Transform3& localToGlobalTransform(const Acts::GeometryContext& tgContext) const override final;

      /** @copydoc Acts::SurfacePlacementBase::surface */
      const Acts::Surface& surface() const override final;

      /** @copydoc Acts::SurfacePlacementBase::surface */
      Acts::Surface& surface() override final;

      /** @brief Returns a mutable shared pointer to the surface held by the placement */
      std::shared_ptr<Acts::Surface> getSurface() const;

      /** @brief Identifierhash of the associated IReadoutSurfacePositioning */
      IdentifierHash hash() const;

      /** @copydoc ISurfacePlacement::identify */
      Identifier identify() const override final;

      /** @copydoc ISurfacePlacement::detectorType */
      DetectorType detectorType() const override final;
 
      /** @copydoc ISurfacePlacement::detectorElement */
      const IDetectorElement* detectorElement() const override final;
    private:
      /** @brief Constructor taking the SurfacePositioning object */
      explicit SurfacePlacement(IReadoutSurfacePositioning* transformCache);
      /** @brief Pointer to the parent */
      const IReadoutSurfacePositioning* m_transformCache{nullptr};
  }; 

  template <typename Surface_t, typename Bounds_t>  
    std::shared_ptr<SurfacePlacement> 
        SurfacePlacement::makeShared(IReadoutSurfacePositioning& transformCache,
                                    std::shared_ptr<Bounds_t>&& bounds)
                requires(std::is_base_of_v<Acts::SurfaceBounds, Bounds_t>){
      if (transformCache.m_surface || transformCache.m_placement) {
          return nullptr;
      } 
      transformCache.m_placement.reset(new SurfacePlacement(&transformCache));
      transformCache.m_surface = Acts::Surface::makeShared<Surface_t>(std::move(bounds), *transformCache.m_placement);
      return transformCache.m_placement;
    }
}

#endif
#endif
