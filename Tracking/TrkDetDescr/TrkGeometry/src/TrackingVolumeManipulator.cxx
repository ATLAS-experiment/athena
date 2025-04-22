/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackingVolumeManipulator.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "TrkGeometry/TrackingVolumeManipulator.h"

#include <utility>

#include "TrkGeometry/TrackingVolume.h"
#include "TrkVolumes/CylinderVolumeBounds.h"

void
Trk::TrackingVolumeManipulator::glueVolumes(
  Trk::TrackingVolume& firstVol,
  Trk::BoundarySurfaceFace firstFace,
  Trk::TrackingVolume& secondVol,
  Trk::BoundarySurfaceFace secondFace) const
{
  // check if it is a cylinder volume
  const Trk::CylinderVolumeBounds* cylBounds =
    dynamic_cast<const Trk::CylinderVolumeBounds*>(&(firstVol.volumeBounds()));

  if (firstFace == Trk::tubeOuterCover && secondFace == Trk::tubeInnerCover)
    return glueVolumes(secondVol, secondFace, firstVol, firstFace);

  // the second volume gets the face of the first volume assigned
  (secondVol.m_boundarySurfaces)[secondFace] =
    (firstVol.m_boundarySurfaces)[firstFace];
  // the face of the first volume has been an inner tube
  if (cylBounds && firstFace == Trk::tubeInnerCover &&
      secondFace == Trk::tubeOuterCover) {
    (secondVol.m_boundarySurfaces)[secondFace]->setInsideVolume(&secondVol);
  } else {
    (secondVol.m_boundarySurfaces)[secondFace]->setOutsideVolume(&secondVol);
  }
}

void
Trk::TrackingVolumeManipulator::setBoundarySurface(
  Trk::TrackingVolume& tvol,
  std::shared_ptr<Trk::BoundarySurface<Trk::TrackingVolume>> bsurf,
  Trk::BoundarySurfaceFace face)
{
  (tvol.m_boundarySurfaces)[face] = std::move(bsurf);
}

void
Trk::TrackingVolumeManipulator::setInsideVolume(Trk::TrackingVolume& tvol,
                                                Trk::BoundarySurfaceFace face,
                                                Trk::TrackingVolume* insidevol)
{

  (tvol.m_boundarySurfaces)[face]->setInsideVolume(insidevol);
}

void
Trk::TrackingVolumeManipulator::setInsideVolumeArray(
  Trk::TrackingVolume& tvol,
  Trk::BoundarySurfaceFace face,
  Trk::BinnedArray<Trk::TrackingVolume>* insidevolarray)
{

  (tvol.m_boundarySurfaces)[face]->setInsideVolumeArray(
    std::shared_ptr<Trk::BinnedArray<Trk::TrackingVolume>>(insidevolarray));
}

void
Trk::TrackingVolumeManipulator::setInsideVolumeArray(
  Trk::TrackingVolume& tvol,
  Trk::BoundarySurfaceFace face,
  const std::shared_ptr<Trk::BinnedArray<Trk::TrackingVolume>>&
    insidevolarray)
{
  (tvol.m_boundarySurfaces)[face]->setInsideVolumeArray(insidevolarray);
}

void
Trk::TrackingVolumeManipulator::setOutsideVolume(
  Trk::TrackingVolume& tvol,
  Trk::BoundarySurfaceFace face,
  Trk::TrackingVolume* outsidevol)
{
  (tvol.m_boundarySurfaces)[face]->setOutsideVolume(outsidevol);
}

void
Trk::TrackingVolumeManipulator::setOutsideVolumeArray(
  Trk::TrackingVolume& tvol,
  Trk::BoundarySurfaceFace face,
  Trk::BinnedArray<Trk::TrackingVolume>* outsidevolarray)
{

  (tvol.m_boundarySurfaces)[face]->setOutsideVolumeArray(
    std::shared_ptr<Trk::BinnedArray<Trk::TrackingVolume>>(outsidevolarray));
}

void
Trk::TrackingVolumeManipulator::setOutsideVolumeArray(
  Trk::TrackingVolume& tvol,
  Trk::BoundarySurfaceFace face,
  const std::shared_ptr<Trk::BinnedArray<Trk::TrackingVolume>>&
    outsidevolarray)
{
  (tvol.m_boundarySurfaces)[face]->setOutsideVolumeArray(outsidevolarray);
}

void
Trk::TrackingVolumeManipulator::confineVolume(TrackingVolume& tvol,
                                              TrackingVolume* outsideVol)
{

  const auto& bounds = tvol.boundarySurfaces();
  for (const auto & bound : bounds) {
    if (bound->outsideVolume() == nullptr) {
      bound->setOutsideVolume(outsideVol);
    }
    if (bound->insideVolume() == nullptr) {
      bound->setInsideVolume(outsideVol);
    }
  }
}
