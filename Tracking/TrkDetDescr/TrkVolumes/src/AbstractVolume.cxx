/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// AbstractVolume.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkVolumes/AbstractVolume.h"
//
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkSurfaces/DiscSurface.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/Surface.h"
#include "TrkVolumes/BoundaryCylinderSurface.h"
#include "TrkVolumes/BoundaryDiscSurface.h"
#include "TrkVolumes/BoundaryPlaneSurface.h"
#include "TrkVolumes/BoundarySurface.h"
#include "TrkVolumes/VolumeBounds.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SystemOfUnits.h"
// STD
#include <iostream>
// Eigen accessor
#include "GeoPrimitives/GeoPrimitives.h"

// Default constructor
Trk::AbstractVolume::AbstractVolume()
  : Volume()
  , m_boundarySurfaces(nullptr)
{}

// constructor with Amg::Transform3D
Trk::AbstractVolume::AbstractVolume(
  std::unique_ptr<Amg::Transform3D> htrans,
  std::shared_ptr<Trk::VolumeBounds> volbounds)
  : Volume(std::move(htrans), std::move(volbounds))
  , m_boundarySurfaces(nullptr)
{
  createBoundarySurfaces();
}

// copy constructor - will up to now not copy the sub structure!
Trk::AbstractVolume::AbstractVolume(const Trk::AbstractVolume& vol)
  : Volume(vol)
  , m_boundarySurfaces(nullptr)
{}

// destructor
Trk::AbstractVolume::~AbstractVolume()
{
  delete m_boundarySurfaces;
}

// assignment operator
Trk::AbstractVolume&
Trk::AbstractVolume::operator=(const Trk::AbstractVolume& vol)
{
  if (this != &vol) {
    Volume::operator=(vol);
    delete m_boundarySurfaces;
    m_boundarySurfaces = new std::vector<
      std::shared_ptr<const Trk::BoundarySurface<Trk::AbstractVolume>>>(
      *vol.m_boundarySurfaces);
  }
  return *this;
}

const std::vector<
  std::shared_ptr<const Trk::BoundarySurface<Trk::AbstractVolume>>>&
Trk::AbstractVolume::boundarySurfaces() const
{
  return (*m_boundarySurfaces);
}

void Trk::AbstractVolume::createBoundarySurfaces()
{
  // prepare the BoundarySurfaces
  m_boundarySurfaces = new std::vector<
    std::shared_ptr<const Trk::BoundarySurface<Trk::AbstractVolume>>>;
  // transform Surfaces To BoundarySurfaces
  std::vector<std::unique_ptr<Trk::Surface>> surfaces =
    Trk::Volume::volumeBounds().decomposeToSurfaces(this->transform());
  auto surfIter = surfaces.begin();

  // counter to flip the inner/outer position for Cylinders
  int sfCounter = 0;
  int sfNumber = surfaces.size();

  for (; surfIter != surfaces.end(); ++surfIter) {
    sfCounter++;
    Trk::PlaneSurface* psf = dynamic_cast<Trk::PlaneSurface*>((*surfIter).get());
    if (psf) {
      m_boundarySurfaces->push_back(
          std::shared_ptr<const Trk::BoundarySurface<Trk::AbstractVolume>>(
              new Trk::BoundaryPlaneSurface<Trk::AbstractVolume>(this, nullptr,
                                                                 *psf)));
      continue;
    }
    Trk::DiscSurface* dsf = dynamic_cast<Trk::DiscSurface*>((*surfIter).get());
    if (dsf) {
      m_boundarySurfaces->push_back(
          std::shared_ptr<const Trk::BoundarySurface<Trk::AbstractVolume>>(
              new Trk::BoundaryDiscSurface<Trk::AbstractVolume>(this, nullptr,
                                                                *dsf)));
      continue;
    }
    Trk::CylinderSurface* csf = dynamic_cast<Trk::CylinderSurface*>((*surfIter).get());
    if (csf) {
      Trk::AbstractVolume* inner =
        (sfCounter == 3 && sfNumber > 3) ? nullptr : this;
      Trk::AbstractVolume* outer = (inner) ? nullptr : this;
      m_boundarySurfaces->push_back(
          std::shared_ptr<const Trk::BoundarySurface<Trk::AbstractVolume>>(
              new Trk::BoundaryCylinderSurface<Trk::AbstractVolume>(
                  inner, outer, *csf)));
      continue;
    }
  }
}

