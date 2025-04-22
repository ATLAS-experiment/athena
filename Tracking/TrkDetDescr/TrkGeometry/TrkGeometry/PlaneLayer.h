/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PlaneLayer.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKGEOMETRY_PLANELAYER_H
#define TRKGEOMETRY_PLANELAYER_H

class MsgStream;

#include "TrkEventPrimitives/PropDirection.h"
#include "TrkGeometry/Layer.h"
#include "TrkSurfaces/PlaneSurface.h"
// STL sorting
#include <algorithm>

namespace Trk {

class RectangleBounds;
class TrapezoidBounds;
class DiamondBounds;
class EllipseBounds;
class LayerMaterialProperties;

/**
 @class PlaneLayer

 Class to describe a planar detector layer for tracking,
 it inhertis from both, Layer base class and PlaneSurface class

 @author Andreas.Salzburger@cern.ch
 */

class PlaneLayer final
  : public PlaneSurface
  , public Layer
{
public:
  /**Default Constructor*/
  PlaneLayer() {}

  /**Constructor with PlaneSurface
     components and MaterialProperties
     - rectangle bounds */
  PlaneLayer(const Amg::Transform3D & transform, const SurfaceBounds* rbounds,
             const LayerMaterialProperties& laymatprop, double thickness = 0.,
             std::unique_ptr<OverlapDescriptor> od = nullptr, int laytyp = int(Trk::active));

  PlaneLayer(Trk::PlaneSurface* plane,
             const LayerMaterialProperties& laymatprop, double thickness = 0.,
             std::unique_ptr<OverlapDescriptor> od = nullptr, int laytyp = int(Trk::active));

  /**Constructor with PlaneSurface
     components and MaterialProperties
     - shared bounds */
  PlaneLayer(const Amg::Transform3D & transform,
             const std::shared_ptr<const Trk::SurfaceBounds>& tbounds,
             const Trk::LayerMaterialProperties& laymatprop,
             double thickness = 0., std::unique_ptr<Trk::OverlapDescriptor> olap = nullptr,
             int laytyp = int(Trk::active));

  /**Constructor with PlaneSurface
  components and pointer to SurfaceArray (passing ownership),
  - rectangle bounds */
  PlaneLayer(const Amg::Transform3D & transform, const Trk::SurfaceBounds* tbounds,
             std::unique_ptr<SurfaceArray> surfaceArray, double thickness = 0.,
             std::unique_ptr<OverlapDescriptor> od = nullptr, int laytyp = int(Trk::active));

  /**Copy constructor of PlaneLayer*/
  PlaneLayer(const PlaneLayer& pla);

  /**Copy constructor with shift*/
  PlaneLayer(const PlaneLayer& pla, const Amg::Transform3D& tr);

  /**Assignment operator for PlaneLayers */
  PlaneLayer& operator=(const PlaneLayer&);

  /**Destructor*/
  virtual ~PlaneLayer() override {}

  /** Transforms the layer into a Surface representation for extrapolation */
  virtual const PlaneSurface& surfaceRepresentation() const override final;
  virtual PlaneSurface& surfaceRepresentation() override final;


  /** getting the MaterialProperties back - for pre-update*/

  virtual double preUpdateMaterialFactor(
      const Trk::TrackParameters& par,
      Trk::PropDirection dir) const override final;

  /** getting the MaterialProperties back - for post-update*/

  virtual double postUpdateMaterialFactor(
      const Trk::TrackParameters& par,
      Trk::PropDirection dir) const override final;

  /** move the Layer */
  virtual void moveLayer(Amg::Transform3D& shift) override final;

  /** Resize the layer to the tracking volume - not implemented */
  virtual void resizeLayer(const VolumeBounds&, double) override final {}

  /** Resize the layer to the tracking volume - not implemented */
  virtual void resizeAndRepositionLayer(const VolumeBounds&,
                                        const Amg::Vector3D&,
                                        double) override final {}
};

/** @class PlaneLayerSorterX
   Functor for PlaneLayer X-Sorting */

class PlaneLayerSorterX {
 public:
  /** Default Constructor */
  PlaneLayerSorterX() {}

  bool operator()(const PlaneLayer* one, const PlaneLayer* two) const {
    return (one->center().x() < two->center().x());
  }
};

/** @class PlaneLayerSorterY
   Functor for PlaneLayer Y-Sorting */

class PlaneLayerSorterY {
 public:
  /** Default Constructor */
  PlaneLayerSorterY() {}

  bool operator()(const PlaneLayer* one, const PlaneLayer* two) const {
    return (one->center().y() < two->center().y());
  }
};

/** @class PlaneLayerSorterZ
   Functor for PlaneLayer Z-Sorting */

class PlaneLayerSorterZ {
 public:
  /** Default Constructor */
  PlaneLayerSorterZ() {}

  bool operator()(const PlaneLayer* one, const PlaneLayer* two) const {
    return (one->center().z() < two->center().z());
  }
};

}  // namespace Trk

#endif  // TRKGEOMETY_PLANELAYER_H

