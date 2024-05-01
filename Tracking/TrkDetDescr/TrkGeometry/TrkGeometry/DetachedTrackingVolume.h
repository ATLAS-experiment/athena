/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
//CxxUtils
#include "CxxUtils/checker_macros.h"
// Amg
#include "GeoPrimitives/GeoPrimitives.h"


#include <span>
namespace Trk {
class TrackingVolume;
class Surface;
class MaterialProperties;
class MagneticFieldProperties;

// For local spans (typedef to make it easier for C++20 std:: one)
template<class T>
using ArraySpan = std::span<T>;


/**
 @class DetachedTrackingVolume

 Base Class for a navigation object (active/passive) in the Tracking realm.

 @author Sarka.Todorova@cern.ch

 */

class DetachedTrackingVolume {
  /**Declare the IDetachedTrackingVolumeBuilder as a friend, to be able to
   * change the volumelink */
  friend class TrackingVolume;
  friend class DetachedTrackingVolumeBuilder;
  friend class IDetachedTrackingVolumeBuilder;

 public:
  /**Default Constructor*/
  DetachedTrackingVolume();

  /**Constructor with name */
  DetachedTrackingVolume(std::string name, TrackingVolume* vol);

  /**Constructor with name & layer representation*/
  DetachedTrackingVolume(std::string name,
                         TrackingVolume* vol,
                         Layer* layer,
                         const std::vector<Layer*>* multilayer = nullptr);

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
  ArraySpan<Layer const * const>  multilayerRepresentation() const;
  ArraySpan<Layer * const>  multilayerRepresentation();

  /** sign the volume - the geometry builder has to do that */
  void sign(GeometrySignature signat, GeometryType geotype);

  /** return the Signature */
  GeometrySignature geometrySignature() const;

  /** return the Type */
  GeometryType geometryType() const;

  /** set the simplified calculable components */
  void saveConstituents(
      const std::vector<std::pair<std::unique_ptr<const Trk::Volume>, float>>*);
  /** get the simplified calculable components */
  const std::vector<std::pair<std::unique_ptr<const Trk::Volume>, float>>*
  constituents() const;

  /** alignment methods: set base transform / default argument to current
   * transform */

  void setBaseTransform(Amg::Transform3D* transf = nullptr);

 private:
  /** Compactify -- set TG as owner to surfaces */
   void compactify(size_t& cSurfaces, size_t& tSurfaces);

   TrackingVolume* m_trkVolume;
   const std::string m_name;
   Layer* m_layerRepresentation;
   const std::vector<Layer*>* m_multilayerRepresentation;
   Amg::Transform3D* m_baseTransform; // optional use (for alignment purpose)
   const std::vector<std::pair<std::unique_ptr<const Trk::Volume>, float>>*
     m_constituents;
};

inline const TrackingVolume* DetachedTrackingVolume::trackingVolume() const {
  return (m_trkVolume);
}

inline TrackingVolume* DetachedTrackingVolume::trackingVolume(){
  return (m_trkVolume);
}


inline const std::string& DetachedTrackingVolume::name() const { return (m_name); }

inline const Layer* DetachedTrackingVolume::layerRepresentation() const {
  return (m_layerRepresentation);
}

inline Layer* DetachedTrackingVolume::layerRepresentation() {
  return (m_layerRepresentation);
}

inline ArraySpan<Layer const* const>
DetachedTrackingVolume::multilayerRepresentation() const
{
  if (m_multilayerRepresentation) {
    return ArraySpan<Layer const* const>(&*m_multilayerRepresentation->begin(),
                                         &*m_multilayerRepresentation->end());
  }
  return {};
}

inline ArraySpan<Layer* const>
DetachedTrackingVolume::multilayerRepresentation()
{
  if (m_multilayerRepresentation) {
    return ArraySpan<Layer* const>(&*m_multilayerRepresentation->begin(),
                                   &*m_multilayerRepresentation->end());
  }
  return {};
}

inline void DetachedTrackingVolume::saveConstituents(
    const std::vector<std::pair<std::unique_ptr<const Trk::Volume>, float>>*
        constituents) {
  m_constituents = constituents;
}

inline const std::vector<std::pair<std::unique_ptr<const Trk::Volume>, float>>*
DetachedTrackingVolume::constituents() const {
  return m_constituents;
}


}  // namespace Trk

#endif  // TRKGEOMETRY_DETACHEDTRACKINGVOLUME_H

