/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SubtractedCylinderSurface.cxx, (c) ATLAS Detector Software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkGeometrySurfaces/SubtractedCylinderSurface.h"
// STD
#include <iomanip>
#include <iostream>

// constructor
Trk::SubtractedCylinderSurface::SubtractedCylinderSurface(const Trk::CylinderSurface& ps,
                                                          std::shared_ptr<const AreaExcluder> vol,
                                                          bool shared)
  : Trk::CylinderSurface(ps)
  , m_subtrVol(std::move(vol))
  , m_shared(shared)
{}

// copy constructor
Trk::SubtractedCylinderSurface::SubtractedCylinderSurface(const SubtractedCylinderSurface& psf) = default;

// copy constructor with shift
Trk::SubtractedCylinderSurface::SubtractedCylinderSurface(const SubtractedCylinderSurface& psf,
                                                          const Amg::Transform3D& transf)
  : Trk::CylinderSurface(psf, transf)
  , m_subtrVol{psf.m_subtrVol}
  , m_shared(psf.m_shared)
{}

//Assignement
Trk::SubtractedCylinderSurface&
Trk::SubtractedCylinderSurface::operator=(const Trk::SubtractedCylinderSurface& psf) = default;

bool
Trk::SubtractedCylinderSurface::operator==(const Trk::Surface& sf) const
{
  // first check the type not to compare apples with oranges
  const Trk::SubtractedCylinderSurface* scsf = dynamic_cast<const Trk::SubtractedCylinderSurface*>(&sf);
  if (!scsf)
    return false;
  bool surfaceEqual = Trk::CylinderSurface::operator==(sf);
  bool sharedEqual = (surfaceEqual) ? (shared() == scsf->shared()) : false;
  return sharedEqual;
}
