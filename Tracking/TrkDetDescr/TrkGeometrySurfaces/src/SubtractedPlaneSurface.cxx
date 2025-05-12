/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SubtractedPlaneSurface.cxx, (c) ATLAS Detector Software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkGeometrySurfaces/SubtractedPlaneSurface.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
// STD
#include <iomanip>
#include <iostream>


// copy constructor
Trk::SubtractedPlaneSurface::SubtractedPlaneSurface(const SubtractedPlaneSurface& psf) = default;

// copy constructor with shift
Trk::SubtractedPlaneSurface::SubtractedPlaneSurface(const SubtractedPlaneSurface& psf, const Amg::Transform3D& transf)
  : Trk::PlaneSurface(psf, transf)
  , m_subtrVol(psf.m_subtrVol)
  , m_shared(psf.m_shared)
{}

// constructor
Trk::SubtractedPlaneSurface::SubtractedPlaneSurface(const Trk::PlaneSurface& ps,
                                                    std::shared_ptr<const AreaExcluder> vol,
                                                    bool shared)
  : Trk::PlaneSurface(ps)
  , m_subtrVol(std::move(vol))
  , m_shared(shared)
{}


Trk::SubtractedPlaneSurface&
Trk::SubtractedPlaneSurface::operator=(const Trk::SubtractedPlaneSurface& psf) = default;

bool
Trk::SubtractedPlaneSurface::operator==(const Trk::Surface& sf) const
{
  // first check the type not to compare apples with oranges
  const Trk::SubtractedPlaneSurface* spsf = dynamic_cast<const Trk::SubtractedPlaneSurface*>(&sf);
  if (!spsf)
    return false;
  bool surfaceEqual = Trk::PlaneSurface::operator==(sf);
  bool sharedEqual = (surfaceEqual) ? (shared() == spsf->shared()) : false;
  return sharedEqual;
}
