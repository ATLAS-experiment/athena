/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSDETECTORELEMENT_H
#define ACTSGEOMETRY_ACTSDETECTORELEMENT_H

// Amg Eigen plugin includes
#include "GeoPrimitives/GeoPrimitives.h"

#include "GeoModelKernel/GeoVDetectorElement.h"
#include "ActsGeometryInterfaces/IDetectorElement.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsGeoUtils/TransformCache.h"
// ATHENA INCLUDES
#include "HGTD_ReadoutGeometry/HGTD_DetectorElement.h"
#include "HGTD_Identifier/HGTD_ID.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "TRT_ReadoutGeometry/TRT_BaseElement.h"
#include "TrkSurfaces/AnnulusBounds.h"
#include "CxxUtils/CachedValue.h"

// ACTS
#include "Acts/Geometry/DetectorElementBase.hpp"
#include "Acts/Geometry/GeometryContext.hpp"

// STL
#include <iostream>

namespace Acts {
class SurfaceBounds;
}

class ActsTrackingGeometrySvc;

class IdentityHelper;

/// @class ActsDetectorElement
///
class ActsDetectorElement : public ActsTrk::IDetectorElement, public GeoVDetectorElement {
public:
  using DetectorType = ActsTrk::DetectorType;
  using AlignmentStore = ActsGeometryContext::AlignmentStore;


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


  virtual unsigned int storeAlignedTransforms(const ActsTrk::DetectorAlignStore& alignStore) const override;
  
  virtual const Acts::Transform3 &
  transform(const Acts::GeometryContext &gctx) const final override;

  /// Return surface associated with this identifier, which should come from the
  virtual const Acts::Surface &surface() const final override;

  /// Mutable surface to this detector element
  virtual Acts::Surface &surface() final override;

  /// Return a shared pointer on the ATLAS surface associated with this
  /// identifier,
  const Trk::Surface &atlasSurface() const;

  /// Returns the thickness of the module
  virtual double thickness() const final override;

  IdentityHelper identityHelper() const;

  /// Returns default transform. For TRT this is static and set in constructor.
  /// For silicon detectors it is calulated from GM, and stored. Thus the method
  /// is not const. The store is mutexed.
  const Acts::Transform3 &getDefaultTransform() const;

  /// Returns the underllying GeoModel detectorelement that this one
  /// is based on.
  const GeoVDetectorElement *upstreamDetectorElement() const;

private:
  DetectorType m_type{DetectorType::UnDefined};

  ActsTrk::TransformCache m_trfCache;
  /// Detector element as variant
  const GeoVDetectorElement *m_detElement{nullptr};
  /// Boundaries of the detector element
  std::shared_ptr<const Acts::SurfaceBounds> m_bounds{};
  ///  Thickness of this detector element
  double m_thickness{0.};
  /// Corresponding Surface
  std::shared_ptr<Acts::Surface> m_surface{};
  std::vector<std::shared_ptr<const Acts::Surface>> m_surfaces{};



  Identifier m_explicitIdentifier{};
};

#endif
