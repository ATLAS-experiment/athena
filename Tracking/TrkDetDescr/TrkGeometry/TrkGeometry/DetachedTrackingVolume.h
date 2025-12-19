/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// DetachedTrackingVolume.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKGEOMETRY_DETACHEDTRACKINGVOLUME_H
#define TRKGEOMETRY_DETACHEDTRACKINGVOLUME_H

class MsgStream;

#include "TrkDetDescrUtils/GeometrySignature.h"
#include "TrkGeometry/Layer.h"
#include "TrkGeometry/OverlapDescriptor.h"
#include "TrkGeometry/PlaneLayer.h"
#include "TrkSurfaces/Surface.h"
// Amg
#include "GeoPrimitives/GeoPrimitives.h"
#include <span>
namespace Trk {
class TrackingVolume;
class Surface;
class MaterialProperties;
class MagneticFieldProperties;

/**
 @class DetachedTrackingVolume

 Base Class for a navigation object (active/passive) in the Tracking realm.

 @author Sarka.Todorova@cern.ch

 */

class DetachedTrackingVolume final{
  /**Declare the IDetachedTrackingVolumeBuilder as a friend, to be able to
   * change the volumelink */
  friend class TrackingVolume;
  friend class DetachedTrackingVolumeBuilder;
  friend class IDetachedTrackingVolumeBuilder;

 public:
  /**Default Constructor*/
  DetachedTrackingVolume();

  /**Constructor with name */
  DetachedTrackingVolume(std::string name, std::unique_ptr<TrackingVolume> vol);

  /**Constructor with name & layer representation*/
  DetachedTrackingVolume(std::string name,
                         std::unique_ptr<TrackingVolume> vol,
                         std::unique_ptr<Layer> layer,
                         std::unique_ptr<const std::vector<Layer*>> multilayer = nullptr);

  /**Destructor*/
  ~DetachedTrackingVolume();

  /** returns the TrackingVolume */
  const TrackingVolume* trackingVolume() const;
  TrackingVolume* trackingVolume();

  /** returns the Name */
  const std::string& name() const;

  /** moving object around */
  void move (Amg::Transform3D& shift);

  /** clone with transform*/
  DetachedTrackingVolume* clone(const std::string& name,
                                      Amg::Transform3D& shift) const;

  /** returns layer representation */
  const Layer* layerRepresentation() const;
  Layer* layerRepresentation();

  /** returns (multi)layer representation */
  std::span<Layer const * const>  multilayerRepresentation() const;
  std::span<Layer * const>  multilayerRepresentation();

  /** sign the volume - the geometry builder has to do that */
  void sign(GeometrySignature signat, GeometryType geotype);

  /** return the Signature */
  GeometrySignature geometrySignature() const;

  /** return the Type */
  GeometryType geometryType() const;

  /** alignment methods: set base transform / default argument to current
   * transform */
  void setBaseTransform(std::unique_ptr<Amg::Transform3D> transf = nullptr);

 private:
  /** Compactify -- set TG as owner to surfaces */
   void compactify(size_t& cSurfaces, size_t& tSurfaces);
   std::unique_ptr<TrackingVolume> m_trkVolume;
   std::unique_ptr<Layer> m_layerRepresentation;
   //We own also the elements in the vector
   std::unique_ptr<const std::vector<Layer*>> m_multilayerRepresentation = nullptr;
   const std::string m_name{"undefined"};
   // optional use (for alignment purpose)
   std::unique_ptr<Amg::Transform3D> m_baseTransform = nullptr;
};

inline const TrackingVolume* DetachedTrackingVolume::trackingVolume() const {
  return m_trkVolume.get();
}

inline TrackingVolume* DetachedTrackingVolume::trackingVolume(){
  return m_trkVolume.get();
}

inline const std::string& DetachedTrackingVolume::name() const { return (m_name); }

inline const Layer* DetachedTrackingVolume::layerRepresentation() const {
  return m_layerRepresentation.get();
}

inline Layer* DetachedTrackingVolume::layerRepresentation() {
  return m_layerRepresentation.get();
}


inline std::span<Layer const* const>
DetachedTrackingVolume::multilayerRepresentation() const
{
  if (m_multilayerRepresentation) {
    return std::span<Layer const* const>(m_multilayerRepresentation->begin(),
                                         m_multilayerRepresentation->end());
  }
  return {};
}

inline std::span<Layer* const>
DetachedTrackingVolume::multilayerRepresentation()
{
  if (m_multilayerRepresentation) {
    return std::span<Layer* const>(m_multilayerRepresentation->begin(),
                                   m_multilayerRepresentation->end());
  }
  return {};
}

}  // namespace Trk

#endif  // TRKGEOMETRY_DETACHEDTRACKINGVOLUME_H

