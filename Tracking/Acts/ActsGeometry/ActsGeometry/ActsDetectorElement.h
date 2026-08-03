/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSDETECTORELEMENT_H
#define ACTSGEOMETRY_ACTSDETECTORELEMENT_H

// Amg Eigen plugin includes
#include "GeoPrimitives/GeoPrimitives.h"

#include "GeoModelKernel/GeoVDetectorElement.h"
#include "ActsGeometryInterfaces/IDetectorElement.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
// ATHENA INCLUDES
#include "ActsGeoUtils/TransformCache.h"
#include "ActsGeoUtils/SurfacePlacement.h"

// ACTS
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Utilities/PointerTraits.hpp"

// STL
#include <memory>

namespace InDetDD {
class TRT_BaseElement;
class SiDetectorElement;
class HGTD_DetectorElement;
}

namespace Trk {
class Surface;
}

namespace Acts {
class SurfaceBounds;
}

class IdentityHelper;


class ActsDetectorElement;
/** @brief Attempts to retrieve the ActsDetectorElement associated to the
 *         passed ActsSurface
 * @param surf: The surface for which the detector element shall be retrieved */
const ActsDetectorElement* getActsDetectorElement(const Acts::Surface& surf);
template <Acts::PointerConcept ptr_t>
const ActsDetectorElement* getActsDetectorElement(const ptr_t& surf) 
  requires(std::is_base_of_v<Acts::Surface, Acts::RemovePointer_t<ptr_t>>) {
      if (!surf) {
         return nullptr;
      }
      return getActsDetectorElement(*surf);
  }
/// @class ActsDetectorElement
///

class ActsDetectorElement : public ActsTrk::IDetectorElement, public GeoVDetectorElement {
public:
  using DetectorType = ActsTrk::DetectorType;
  using AlignmentStore = ActsTrk::GeometryContext::AlignmentStore;


  ActsDetectorElement(const InDetDD::SiDetectorElement &detElem);

  /// Constructor for a straw surface.
  /// @param transform Transform to the straw system
  ActsDetectorElement(
      const Acts::Transform3 &trf, const InDetDD::TRT_BaseElement &detElem,
      const Identifier &id // we need explicit ID here b/c of straws
  );

  /// Constructor for an HGTD surface.
  ActsDetectorElement(
      const InDetDD::HGTD_DetectorElement &detElem,
      const Identifier &id // explicit id is needed for HGTD
  );

  ///  Destructor
  virtual ~ActsDetectorElement() = default;

  /// Identifier
  Identifier identify() const override final;
   
  /// Detector type
  DetectorType detectorType() const override final;

  /// Identifier hash
  IdentifierHash identifyHash() const { return m_idHash; }

  virtual unsigned int storeAlignedTransforms(ActsTrk::DetectorAlignStore& alignStore) const override;
  
  const Acts::Transform3& localToGlobalTransform(const Acts::GeometryContext& tgContext) const;

  const Acts::Transform3& localToGlobalTransform(const ActsTrk::GeometryContext& gctx) const override;

  virtual const Acts::Transform3 &
  localToGlobalTransform(const ActsTrk::DetectorAlignStore* store) const final override;
  /// Return surface associated with this identifier, which should come from the
  const Acts::Surface &surface() const;
  
  /// Mutable surface to this detector element
  Acts::Surface &surface();

  /// Return a shared pointer on the ATLAS surface associated with this
  /// identifier,
  const Trk::Surface &atlasSurface() const;

  /// Returns the thickness of the module
  double thickness() const;

  IdentityHelper identityHelper() const;

  /// Returns default transform. For TRT this is static and set in constructor.
  /// For silicon detectors it is calulated from GM, and stored. Thus the method
  /// is not const. The store is mutexed.
  const Acts::Transform3 &getDefaultTransform() const;

  /// Returns the underllying GeoModel detectorelement that this one
  /// is based on.
  const GeoVDetectorElement *upstreamDetectorElement() const;

  Amg::Transform3D localToGlobal(const ActsTrk::DetectorAlignStore* store) const;
private:
  IdentifierHash m_idHash {};
  DetectorType m_type{DetectorType::UnDefined};
  ActsTrk::ReadoutSurfacePositioning<ActsDetectorElement> m_trfCache{0, this};

  std::shared_ptr<ActsTrk::SurfacePlacement> m_surfHolder{};
  /// Detector element as variant
  const GeoVDetectorElement *m_detElement{nullptr};
  /// Boundaries of the detector element
  std::shared_ptr<const Acts::SurfaceBounds> m_bounds{};
  ///  Thickness of this detector element
  double m_thickness{0.};

  std::unique_ptr<const Amg::Transform3D> m_trtTrf{};

  Identifier m_explicitIdentifier{};
};

namespace ActsTrk{
    template <> inline Amg::Transform3D 
        ReadoutSurfacePositioning<ActsDetectorElement>::fetchTransform(const DetectorAlignStore* store) const{
        return m_parent->localToGlobal(store);
   }
}

#endif
