/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SubtractedDiscSurface.cxx, (c) ATLAS Detector Software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkGeometrySurfaces/SubtractedDiscSurface.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
// STD
#include <iomanip>
#include <iostream>


// constructor
Trk::SubtractedDiscSurface::SubtractedDiscSurface(
    const Trk::DiscSurface& ps,
    std::shared_ptr<const AreaExcluder> vol,
    bool shared)
    : Trk::DiscSurface(ps), m_subtrVol(std::move(vol)), m_shared(shared) {}

// copy constructor
Trk::SubtractedDiscSurface::SubtractedDiscSurface(const SubtractedDiscSurface& psf) = default;

// copy constructor with shift
Trk::SubtractedDiscSurface::SubtractedDiscSurface(const SubtractedDiscSurface& psf, const Amg::Transform3D& shift)
  : Trk::DiscSurface(psf, shift)
  , m_subtrVol{psf.m_subtrVol}
  , m_shared(psf.m_shared)
{}

//Assignment
Trk::SubtractedDiscSurface&
Trk::SubtractedDiscSurface::operator=(const Trk::SubtractedDiscSurface& psf) = default;

bool
Trk::SubtractedDiscSurface::operator==(const Trk::Surface& sf) const
{
  // first check the type not to compare apples with oranges
  const Trk::SubtractedDiscSurface* sdsf = dynamic_cast<const Trk::SubtractedDiscSurface*>(&sf);
  if (!sdsf)
    return false;
  bool surfaceEqual = Trk::DiscSurface::operator==(sf);
  bool sharedEqual = (surfaceEqual) ? (shared() == sdsf->shared()) : false;
  return sharedEqual;
}
