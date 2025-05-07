/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MaterialLayer.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKGEOMETRY_MATERIALLAYER_H
#define TRKGEOMETRY_MATERIALLAYER_H

class MsgStream;

#include "GeoPrimitives/GeoPrimitives.h"
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkGeometry/Layer.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkGeometry/LayerMaterialProperties.h"
#include "TrkSurfaces/Surface.h"
//
#include <memory>
namespace Trk {

/**
 @class MaterialLayer

 A material layer is a simple helper class to attach material information to a
 boundary surface.
 There is a complication as they are two use cases

 1) We attach the material layer to an existing surface
 via setMaterialLayer in this case the material layer
 should NOT own the surface.
 2) We constuct a material layer with some surface
 representation. In which case it owns it

 @author Andreas.Salzburger@cern.ch
 @author Christos Anastopoulos (Athena MT modifications)

 */

class MaterialLayer final : public Layer {
 public:
  MaterialLayer() = delete;
  MaterialLayer(const MaterialLayer&) = default;
  MaterialLayer(MaterialLayer&&) = default;
  MaterialLayer& operator=(const MaterialLayer&) = default;
  MaterialLayer& operator=(MaterialLayer&&) = default;
  virtual ~MaterialLayer() = default;

  /** Constructor allowing the Material to be attached to an existing surface
   It does NOT own the representation. */
  MaterialLayer(Surface& sf, std::unique_ptr<LayerMaterialProperties> mlprop);

  /** Constructor with a surface representation. It owns that representation  */
  MaterialLayer(std::shared_ptr<Surface>&& sfso,
                std::unique_ptr<LayerMaterialProperties> mlprop);

  /** Transforms the layer into a Surface representation for extrapolation */
  virtual const Surface& surfaceRepresentation() const override final;
  virtual Surface& surfaceRepresentation() override final;

  /** isOnLayer() method, using isOnSurface() with Layer specific
   * tolerance */
  virtual bool isOnLayer(
      const Amg::Vector3D& gp,
      const BoundaryCheck& bcheck = BoundaryCheck(true)) const override final;

  /** Move the layer  - not implemented */
  virtual void moveLayer(Amg::Transform3D&) override final {};

  /** Resize the layer to the tracking volume - not implemented */
  virtual void resizeLayer(const VolumeBounds&, double) override final {}

  /** Resize the layer to the tracking volume - not implemented */
  virtual void resizeAndRepositionLayer(const VolumeBounds&,
                                        const Amg::Vector3D&,
                                        double) override final {}

 private:
  //shared_ptr as we use the custom deleter.
  //It should never be nullptr
  std::shared_ptr<Surface> m_surfaceRepresentation;
};

inline const Surface& MaterialLayer::surfaceRepresentation() const {
  return (*(m_surfaceRepresentation.get()));
}

inline Surface& MaterialLayer::surfaceRepresentation() {
  return (*(m_surfaceRepresentation.get()));
}

}  // namespace Trk

#endif  // TRKGEOMETRY_NAVIGATIONLAYER_H

