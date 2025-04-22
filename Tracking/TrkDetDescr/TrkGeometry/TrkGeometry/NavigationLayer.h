/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// NavigationLayer.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKGEOMETRY_NAVIGATIONLAYER_H
#define TRKGEOMETRY_NAVIGATIONLAYER_H

class MsgStream;

#include "GeoPrimitives/GeoPrimitives.h"
#include "TrkDetDescrUtils/BinnedArray.h"
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkGeometry/Layer.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/Surface.h"

#include <memory>

namespace Trk {
class BinUtility;

/**
 @class NavigationLayer

 Class to be used for gaps in Volumes as a navigational link.
 Navigation Layers have a surface representation, but should usually never be
 propagated to.

 The fullfill a faster acess to the nextLayer in the navigation process inside a
 volume.

 @author Andreas.Salzburger@cern.ch
 @author Christos Anastopoulos (Athena MT modification)

 */

class NavigationLayer final : public Layer {
 public:
  /**Constructor*/
  NavigationLayer(std::unique_ptr<Surface> surfaceRepresentation,
                  Layer* previous = nullptr,
                  Layer* next = nullptr,
                  BinUtility* binUtil = nullptr);

  /**Constructor */
  NavigationLayer(std::unique_ptr<Surface> surfaceRepresentation, double thickness);

  /**Copy Constructor - */
  NavigationLayer(const NavigationLayer& lay);

  /**Destructor*/
  virtual ~NavigationLayer() override = default;

  /** Assignment operator */
  NavigationLayer& operator=(const NavigationLayer& lay);

  /** Transforms the layer into a Surface representation for extrapolation
   */
  virtual const Surface& surfaceRepresentation() const override final;
  virtual Surface& surfaceRepresentation() override final;

  /** isOnLayer() method, using isOnSurface() with Layer specific
   * tolerance */
  virtual bool isOnLayer(
      const Amg::Vector3D& gp,
      const BoundaryCheck& bcheck = BoundaryCheck(true)) const override final;

  /** getting the MaterialProperties back - for full update*/
  const MaterialProperties* fullUpdateMaterialProperties() const;

  /** getting the MaterialProperties back - for pre-update*/
  virtual double preUpdateMaterialFactor(
      const Trk::TrackParameters& par,
      Trk::PropDirection dir) const override final;

  /** getting the MaterialProperties back - for post-update*/
  virtual double postUpdateMaterialFactor(
      const Trk::TrackParameters& par,
      Trk::PropDirection dir) const override final;

  /** getting the next/overlapping Surface */
  const Surface* overlapSurface(const TrackParameters& tp,
                                const Surface* sf = nullptr) const;

  /** move the Layer */
  virtual void moveLayer(Amg::Transform3D&) override final;

  /** Resize the layer to the tracking volume - not implemented */
  virtual void resizeLayer(const VolumeBounds&, double) override final {}

  /** Resize the layer to the tracking volume - not implemented */
  virtual void resizeAndRepositionLayer(const VolumeBounds&,
                                        const Amg::Vector3D&,
                                        double) override final {}

 protected:
  std::unique_ptr<Surface> m_surfaceRepresentation;

};

inline const Surface&
NavigationLayer::surfaceRepresentation() const
{
  return (*m_surfaceRepresentation);
}

inline Surface&
NavigationLayer::surfaceRepresentation()
{
  return (*m_surfaceRepresentation);
}

inline const MaterialProperties* NavigationLayer::fullUpdateMaterialProperties() const {
  return nullptr;
}

inline double NavigationLayer::preUpdateMaterialFactor(const TrackParameters&,
                                                       PropDirection) const {
  return 0;
}

inline double NavigationLayer::postUpdateMaterialFactor(const TrackParameters&,
                                                        PropDirection) const {
  return 0;
}

inline const Surface* NavigationLayer::overlapSurface(const TrackParameters&,
                                                      const Surface*) const {
  return nullptr;
}

inline void NavigationLayer::moveLayer(Amg::Transform3D&) {}

}  // namespace Trk

#endif  // TRKGEOMETRY_NAVIGATIONLAYER_H

