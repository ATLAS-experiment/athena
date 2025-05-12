/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackingVolumeManipulator.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKGEOMETRY_TRACKINGVOLUMEMANIPULATOR_H
#define TRKGEOMETRY_TRACKINGVOLUMEMANIPULATOR_H

// Trk
#include "TrkDetDescrUtils/BinnedArray.h"
#include <memory>
#include "TrkVolumes/BoundarySurface.h"
#include "TrkVolumes/BoundarySurfaceFace.h"

namespace Trk {

class TrackingVolume;

/** @class TrackingVolumeManipulator

   The only way to change BoundarySurfaces etc from Tracking Volumes.
   Tools have to inherit from this one to have friend rights to the
   TrackingVolume;

   @author Andreas.Salzburger@cern.ch
  */

class TrackingVolumeManipulator
{
public:
  /** constructor */
  TrackingVolumeManipulator() {}
  /** Destructor */
  ~TrackingVolumeManipulator() {}

protected:
  /** protected method to glue two Volumes together
      input:
       - first TrackingVolume that keeps boundary surface
       - face of the BoundarySurface to be kept
       - second volume that gets glued to the first volume
       - face of the BoundarySurface to be shared
      */
  void glueVolumes(TrackingVolume& firstVol,
                   BoundarySurfaceFace firstFace,
                   TrackingVolume& secondVol,
                   BoundarySurfaceFace secondFace) const;

  /** protected method to set the boundary surface of a tracking volume */
  static void setBoundarySurface(
    TrackingVolume& tvol,
    std::shared_ptr<BoundarySurface<TrackingVolume>> bsurf,
    BoundarySurfaceFace face);

  /** protected method to set inside Volume of a BoundarySurface:
      input:
       - the volume that holdes the BoundarySurface
       - the face type of the boundary to be set
       - the volume to be set as inside volume */
  static void setInsideVolume(TrackingVolume& tvol,
                              BoundarySurfaceFace face,
                              TrackingVolume* insidevol);

  /** protected method to set inside VolumeArray of a BoundarySurface:
      input:
       - ithe volume that holdes the BoundarySurface
       - the face type of the boundary to be set
       - the volume array to be set as inside volume array */
  static void setInsideVolumeArray(TrackingVolume& tvol,
                                   BoundarySurfaceFace face,
                                   BinnedArray<TrackingVolume>* insidevolarray);

  /** protected method to set inside VolumeArray of a BoundarySurface:
      input:
       - ithe volume that holdes the BoundarySurface
       - the face type of the boundary to be set
       - the volume array to be set as inside volume array */
  static void setInsideVolumeArray(
    TrackingVolume& tvol,
    BoundarySurfaceFace face,
    const std::shared_ptr<BinnedArray<TrackingVolume>>& insidevolarray);

  /** protected method to set outside Volume of a BoundarySurface:
      input:
       - the volume that holdes the BoundarySurface
       - the face type of the boundary to be set
       - the volume to be set as outside volume */
  static void setOutsideVolume(TrackingVolume& tvol,
                               BoundarySurfaceFace face,
                               TrackingVolume* outsidevol);

  /** protected method to set outside VolumeArray of a BoundarySurface:
      input:
       - the volume that holdes the BoundarySurface
       - the face type of the boundary to be set
       - the volume array to be set as outside volume array */
  static void setOutsideVolumeArray(
    TrackingVolume& tvol,
    BoundarySurfaceFace face,
    BinnedArray<TrackingVolume>* outsidevolarray);

  /** protected method to set outside VolumeArray of a BoundarySurface:
      input:
       - the volume that holdes the BoundarySurface
       - the face type of the boundary to be set
       - the volume array to be set as outside volume array */
  static void setOutsideVolumeArray(
    TrackingVolume& tvol,
    BoundarySurfaceFace face,
    const std::shared_ptr<BinnedArray<TrackingVolume>>& outsidevolarray);

  /** protected method to confine (dense) volumes:
      input:
       - the volume that holdes the BoundarySurfaces (embedded)
       - the volume to be set as outside volume  */
  static void confineVolume(TrackingVolume& tvol, TrackingVolume* outsidevol);
};
} // namespace Trk

#endif
