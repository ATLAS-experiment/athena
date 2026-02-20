/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackSurfaceIntersection.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkExUtils/TrackSurfaceIntersection.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
// STD
#include <iomanip>
#include <iostream>
#include <sstream>

// constructor
Trk::TrackSurfaceIntersection::TrackSurfaceIntersection(
  const Amg::Vector3D& pos,
  const Amg::Vector3D& dir,
  double path)
  : m_position(pos)
  , m_direction(dir)
  , m_pathlength(path)
{
}

Trk::TrackSurfaceIntersection::TrackSurfaceIntersection(
  const TrackSurfaceIntersection& other)
  : m_position(other.m_position)
  , m_direction(other.m_direction)
  , m_pathlength(other.m_pathlength)
  , m_cache(other.m_cache ? other.m_cache->clone() : nullptr)
{
}

Trk::TrackSurfaceIntersection::TrackSurfaceIntersection(
  const TrackSurfaceIntersection& other,
  std::unique_ptr<IIntersectionCache> cache)
  : m_position(other.m_position)
  , m_direction(other.m_direction)
  , m_pathlength(other.m_pathlength)
  , m_cache(std::move(cache))
{
}

Trk::TrackSurfaceIntersection&
Trk::TrackSurfaceIntersection::operator=(const TrackSurfaceIntersection& other)
{
  if (this != &other) {
    m_position = other.m_position;
    m_direction = other.m_direction;
    m_pathlength = other.m_pathlength;
    m_cache = other.m_cache ? other.m_cache->clone() : nullptr;
  }
  return *this;
}

// Overload of << operator for both, MsgStream and std::ostream for debug output
MsgStream&
Trk::operator<<(MsgStream& sl, const Trk::TrackSurfaceIntersection& tsfi)
{
  std::ostringstream os;
  os<<tsfi;
  sl<<os.str();
  return sl;
}

std::ostream&
Trk::operator<<(std::ostream& sl, const Trk::TrackSurfaceIntersection& tsfi)
{
  const auto old_flags = sl.flags();
  const auto old_prec  = sl.precision();

  sl.setf(std::ios::fixed, std::ios::floatfield);
  sl.precision(7);

  sl << "Trk::TrackSurfaceIntersection\n"
     << "    position  [mm] = (" << tsfi.position().x() << ", "
     << tsfi.position().y() << ", " << tsfi.position().z() << ")\n"
     << "    direction      = (" << tsfi.direction().x() << ", "
     << tsfi.direction().y() << ", " << tsfi.direction().z() << ")\n"
     << "    pathlength [mm] = " << tsfi.pathlength() << '\n';

  sl.flags(old_flags);
  sl.precision(old_prec);
  return sl;
}
