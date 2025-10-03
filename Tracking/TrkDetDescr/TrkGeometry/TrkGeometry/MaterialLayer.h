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
#include "TrkGeometry/LayerMaterialProperties.h"
#include "TrkParameters/TrackParameters.h"
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

class MaterialLayer : public Layer {
 public:
  MaterialLayer() = default;
  virtual ~MaterialLayer() = default;

  /** Transforms the layer into a Surface representation for extrapolation */
  virtual const Surface& surfaceRepresentation() const override = 0;
  virtual Surface& surfaceRepresentation() override = 0;
  /** isOnLayer() method, using isOnSurface() with Layer specific
   * tolerance */
  virtual bool isOnLayer(
      const Amg::Vector3D& gp,
      const BoundaryCheck& bcheck = BoundaryCheck(true)) const override = 0;
  /** Move the layer  - not implemented */
  virtual void moveLayer(Amg::Transform3D&) override final {};
  /** Resize the layer to the tracking volume - not implemented */
  virtual void resizeLayer(const VolumeBounds&, double) override final {}
  /** Resize the layer to the tracking volume - not implemented */
  virtual void resizeAndRepositionLayer(const VolumeBounds&,
                                        const Amg::Vector3D&,
                                        double) override final {}
};

class MaterialLayerOwnSurf final : public MaterialLayer {
 public:
  virtual ~MaterialLayerOwnSurf() = default;
  /** Constructor with a surface representation. It owns that representation  */
  MaterialLayerOwnSurf(std::unique_ptr<Surface> surfaceRepresentation,
                       std::unique_ptr<LayerMaterialProperties> mlprop)
      : Trk::MaterialLayer(),
        m_surfaceRepresentation(std::move(surfaceRepresentation)) {
    m_layerMaterialProperties = std::move(mlprop);
    m_layerThickness = 1.;
  }

  virtual const Surface& surfaceRepresentation() const override final {
    return *(m_surfaceRepresentation.get());
  }
  virtual Surface& surfaceRepresentation() override final {
    return *(m_surfaceRepresentation.get());
  }
  virtual bool isOnLayer(
      const Amg::Vector3D& gp,
      const BoundaryCheck& bcheck = BoundaryCheck(true)) const override final {
    return m_surfaceRepresentation.get()->isOnSurface(gp, bcheck);
  }

 private:
  std::unique_ptr<Surface> m_surfaceRepresentation{};
};

class MaterialLayerNoOwnSurf : public MaterialLayer {
 public:
  virtual ~MaterialLayerNoOwnSurf() = default;

  /** Constructor allowing the Material to be attached to an existing surface
   It does NOT own the representation. */
  MaterialLayerNoOwnSurf(Surface* surfaceRepresentation,
                         std::unique_ptr<LayerMaterialProperties> mlprop)
      : Trk::MaterialLayer(), m_surfaceRepresentation(surfaceRepresentation) {
    m_layerMaterialProperties = std::move(mlprop);
    m_layerThickness = 1.;
  }

  virtual const Surface& surfaceRepresentation() const override final {
    return *m_surfaceRepresentation;
  }
  virtual Surface& surfaceRepresentation() override final {
    return *m_surfaceRepresentation;
  }
  virtual bool isOnLayer(
      const Amg::Vector3D& gp,
      const BoundaryCheck& bcheck = BoundaryCheck(true)) const override final {
    return m_surfaceRepresentation->isOnSurface(gp, bcheck);
  }

 private:
  Surface* m_surfaceRepresentation{};
};

}  // namespace Trk

#endif  // TRKGEOMETRY_MATERIALLAYER_H

