/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// CuboidVolumeBounds.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkVolumes/CuboidVolumeBounds.h"
// TrkSurfaces
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/RectangleBounds.h"
#include "TrkSurfaces/Surface.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SystemOfUnits.h"
// STD
#include <cmath>
#include <iostream>

Trk::CuboidVolumeBounds::CuboidVolumeBounds()
  : VolumeBounds()
  , m_halfX(0.)
  , m_halfY(0.)
  , m_halfZ(0.)
  , m_objectAccessor()
{}

Trk::CuboidVolumeBounds::CuboidVolumeBounds(
  double halex,
  double haley,
  double halez)
  : VolumeBounds()
  , m_halfX(halex)
  , m_halfY(haley)
  , m_halfZ(halez)
  , m_objectAccessor()
{}

Trk::CuboidVolumeBounds::CuboidVolumeBounds(const Trk::CuboidVolumeBounds& bobo)
  : VolumeBounds()
  , m_halfX(bobo.m_halfX)
  , m_halfY(bobo.m_halfY)
  , m_halfZ(bobo.m_halfZ)
  , m_objectAccessor(bobo.m_objectAccessor)
{}

Trk::CuboidVolumeBounds::~CuboidVolumeBounds() = default;

Trk::CuboidVolumeBounds&
Trk::CuboidVolumeBounds::operator=(const Trk::CuboidVolumeBounds& bobo)
{
  if (this != &bobo) {
    m_halfX = bobo.m_halfX;
    m_halfY = bobo.m_halfY;
    m_halfZ = bobo.m_halfZ;
    m_objectAccessor = bobo.m_objectAccessor;
  }
  return *this;
}

std::vector<std::unique_ptr<Trk::Surface>>
  Trk::CuboidVolumeBounds::decomposeToSurfaces
  (const Amg::Transform3D& transform)
{

  auto retsf = std::vector<std::unique_ptr<Trk::Surface>>() ;
  // memory optimisation
  retsf.reserve(6);
  // face surfaces xy
  //   (1) - at negative local z
  retsf.push_back(std::make_unique<Trk::PlaneSurface>(
    Amg::Transform3D(
      transform *
      Amg::AngleAxis3D(180. * Gaudi::Units::deg, Amg::Vector3D(0., 1., 0.)) *
      Amg::Translation3D(Amg::Vector3D(0., 0., this->halflengthZ()))),
    this->faceXYRectangleBounds()));
  //   (2) - at positive local z
  retsf.push_back(std::make_unique<Trk::PlaneSurface>(
    Amg::Transform3D(
      transform *
      Amg::Translation3D(Amg::Vector3D(0., 0., this->halflengthZ()))),
    this->faceXYRectangleBounds()));
  // face surfaces yz
  // transmute cyclical
  //   (3) - at negative local x
  retsf.push_back(std::make_unique<Trk::PlaneSurface>(
    Amg::Transform3D(
      transform *
      Amg::AngleAxis3D(180. * Gaudi::Units::deg, Amg::Vector3D(0., 0., 1.)) *
      Amg::Translation3D(Amg::Vector3D(this->halflengthX(), 0., 0)) *
      Amg::AngleAxis3D(90. * Gaudi::Units::deg, Amg::Vector3D(0., 1., 0)) *
      Amg::AngleAxis3D(90. * Gaudi::Units::deg, Amg::Vector3D(0., 0., 1.))),
    this->faceYZRectangleBounds()));
  //   (4) - at positive local x
  retsf.push_back(std::make_unique<Trk::PlaneSurface>(
    Amg::Transform3D(
      transform *
      Amg::Translation3D(Amg::Vector3D(this->halflengthX(), 0., 0.)) *
      Amg::AngleAxis3D(90. * Gaudi::Units::deg, Amg::Vector3D(0., 1., 0.)) *
      Amg::AngleAxis3D(90. * Gaudi::Units::deg, Amg::Vector3D(0., 0., 1.))),
    this->faceYZRectangleBounds()));
  // face surfaces zx
  //   (5) - at negative local y
  retsf.push_back(std::make_unique<Trk::PlaneSurface>(
    Amg::Transform3D(
      transform *
      Amg::AngleAxis3D(180. * Gaudi::Units::deg, Amg::Vector3D(1., 0., 0.)) *
      Amg::Translation3D(Amg::Vector3D(0., this->halflengthY(), 0.)) *
      Amg::AngleAxis3D(-90 * Gaudi::Units::deg, Amg::Vector3D(0., 1., 0.)) *
      Amg::AngleAxis3D(-90. * Gaudi::Units::deg, Amg::Vector3D(1., 0., 0.))),
    this->faceZXRectangleBounds()));
  //   (6) - at positive local y
  retsf.push_back(std::make_unique<Trk::PlaneSurface>(
    Amg::Transform3D(
      transform *
      Amg::Translation3D(Amg::Vector3D(0., this->halflengthY(), 0.)) *
      Amg::AngleAxis3D(-90 * Gaudi::Units::deg, Amg::Vector3D(0., 1., 0.)) *
      Amg::AngleAxis3D(-90. * Gaudi::Units::deg, Amg::Vector3D(1., 0., 0.))),
    this->faceZXRectangleBounds()));

  return retsf;
}

std::shared_ptr<Trk::RectangleBounds>
Trk::CuboidVolumeBounds::faceXYRectangleBounds() const
{
  return std::make_shared<Trk::RectangleBounds>(m_halfX, m_halfY);
}

std::shared_ptr<Trk::RectangleBounds>
Trk::CuboidVolumeBounds::faceYZRectangleBounds() const
{
  return std::make_shared<Trk::RectangleBounds>(m_halfY, m_halfZ);
}

std::shared_ptr<Trk::RectangleBounds>
Trk::CuboidVolumeBounds::faceZXRectangleBounds() const
{
  return std::make_shared<Trk::RectangleBounds>(m_halfZ, m_halfX);
}

// ostream operator overload

MsgStream&
Trk::CuboidVolumeBounds::dump(MsgStream& sl) const
{

  std::stringstream temp_sl;
  temp_sl << std::setiosflags(std::ios::fixed);
  temp_sl << std::setprecision(7);
  temp_sl << "Trk::CuboidVolumeBounds: (halfX, halfY, halfZ) = ";
  temp_sl << "(" << m_halfX << ", " << m_halfY << ", " << m_halfZ << ")";
  sl << temp_sl.str();
  return sl;
}

std::ostream&
Trk::CuboidVolumeBounds::dump(std::ostream& sl) const
{
  std::stringstream temp_sl;
  temp_sl << std::setiosflags(std::ios::fixed);
  temp_sl << std::setprecision(7);
  temp_sl << "Trk::CuboidVolumeBounds: (halfX, halfY, halfZ) = ";
  temp_sl << "(" << m_halfX << ", " << m_halfY << ", " << m_halfZ << ")";
  sl << temp_sl.str();
  return sl;
}

