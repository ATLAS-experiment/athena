/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrapezoidVolumeBounds.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkVolumes/TrapezoidVolumeBounds.h"

#include "TrkDetDescrUtils/GeometryStatics.h"

// TrkSurfaces
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/RectangleBounds.h"
#include "TrkSurfaces/TrapezoidBounds.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SystemOfUnits.h"
// STD
#include <cmath>
#include <iomanip>
#include <iostream>

Trk::TrapezoidVolumeBounds::TrapezoidVolumeBounds()
  : VolumeBounds()
  , m_minHalfX(0.)
  , m_maxHalfX(0.)
  , m_halfY(0.)
  , m_halfZ(0.)
  , m_alpha(0.)
  , m_beta(0.)
  , m_objectAccessor()
{}

Trk::TrapezoidVolumeBounds::TrapezoidVolumeBounds(
  double minhalex,
  double maxhalex,
  double haley,
  double halez)
  : VolumeBounds()
  , m_minHalfX(minhalex)
  , m_maxHalfX(maxhalex)
  , m_halfY(haley)
  , m_halfZ(halez)
  , m_alpha(0.)
  , m_beta(0.)
  , m_objectAccessor()
{
  m_alpha = atan((m_maxHalfX - m_minHalfX) / 2 / m_halfY) + 0.5 * M_PI;
  m_beta = m_alpha;
}

Trk::TrapezoidVolumeBounds::TrapezoidVolumeBounds(
  double minhalex,
  double haley,
  double halez,
  double alpha,
  double beta)
  : VolumeBounds()
  , m_minHalfX(minhalex)
  , m_maxHalfX(0.)
  , m_halfY(haley)
  , m_halfZ(halez)
  , m_alpha(alpha)
  , m_beta(beta)
  , m_objectAccessor()
{
  double gamma = (alpha > beta) ? (alpha - 0.5 * M_PI) : (beta - 0.5 * M_PI);
  m_maxHalfX = m_minHalfX + (2. * m_halfY) * tan(gamma);
}

Trk::TrapezoidVolumeBounds::TrapezoidVolumeBounds(
  const Trk::TrapezoidVolumeBounds& trabo)
  : VolumeBounds()
  , m_minHalfX(trabo.m_minHalfX)
  , m_maxHalfX(trabo.m_maxHalfX)
  , m_halfY(trabo.m_halfY)
  , m_halfZ(trabo.m_halfZ)
  , m_alpha(trabo.m_alpha)
  , m_beta(trabo.m_beta)
  , m_objectAccessor(trabo.m_objectAccessor)
{}

Trk::TrapezoidVolumeBounds::~TrapezoidVolumeBounds() = default;

Trk::TrapezoidVolumeBounds&
Trk::TrapezoidVolumeBounds::operator=(const Trk::TrapezoidVolumeBounds& trabo)
{
  if (this != &trabo) {
    m_minHalfX = trabo.m_minHalfX;
    m_maxHalfX = trabo.m_maxHalfX;
    m_halfY = trabo.m_halfY;
    m_halfZ = trabo.m_halfZ;
    m_alpha = trabo.m_alpha;
    m_beta = trabo.m_beta;
    m_objectAccessor = trabo.m_objectAccessor;
  }
  return *this;
}

std::vector<std::unique_ptr<Trk::Surface>>
  Trk::TrapezoidVolumeBounds::decomposeToSurfaces
  (const Amg::Transform3D& transform)
{
  auto retsf = std::vector<std::unique_ptr<Trk::Surface>>();

  // face surfaces xy
  Amg::RotationMatrix3D trapezoidRotation(transform.rotation());
  Amg::Vector3D trapezoidX(trapezoidRotation.col(0));
  Amg::Vector3D trapezoidY(trapezoidRotation.col(1));
  Amg::Vector3D trapezoidZ(trapezoidRotation.col(2));
  Amg::Vector3D trapezoidCenter(transform.translation());

  //   (1) - at negative local z
  retsf.push_back(std::make_unique<Trk::PlaneSurface>(
    Amg::Transform3D(
      transform *
      Amg::AngleAxis3D(180 * Gaudi::Units::deg, Amg::Vector3D(0., 1., 0.)) *
      Amg::Translation3D(Amg::Vector3D(0., 0., this->halflengthZ()))),
    this->faceXYTrapezoidBounds()));
  //   (2) - at positive local z
  retsf.push_back(std::make_unique<Trk::PlaneSurface>(
    Amg::Transform3D(
      transform *
      Amg::Translation3D(Amg::Vector3D(0., 0., this->halflengthZ()))),
    this->faceXYTrapezoidBounds()));
  // face surfaces yz
  // transmute cyclical
  //   (3) - at point A, attached to alpha opening angle
  //  the yz bound are created in such a way that the surface y-direction has to
  //  become the z-direction. The resulting surface normal has to point in
  //  y-direction. This is achieved by rotating the plane surface by -90
  //  degrees around the x-axis.
  //  Then, the plane has to be rotated around the z-axis by alpha.
  //
  //  double c=cos(M_PI/2);
  //  double s=sin(M_PI/2);
  //  Amg::RotationMatrix3D rotate_to_xz;
  //  rotate_to_xz << 1.f, 0.f, 0.f, // 1 0  0
  //                  0.f, c,  -s,   // 0 0 -1
  //                  0.f, s,   c;   // 0 1  0
  //
  //  s=sin(-alpha());
  //  c=cos(-alpha());
  //  Amg::RotationMatrix3D rotate_left;
  //  rotate_left <<   c,   s,   0.f,
  //                  -s,   c,   0.f,
  //                 0.f, 0.f,   1.f;
  //
  //  Amg::RotationMatrix3D rotateToFaceAlpha( rotate_left * rotate_to_xz);
  Amg::RotationMatrix3D rotateToFaceAlpha;
  {
  double s=sin(-alpha());
  double c=cos(-alpha());
  rotateToFaceAlpha <<   c,  0.f,   s,
                        -s,  0.f,   c,
                       0.f, -1.f, 0.f;
  }

  std::shared_ptr<RectangleBounds> faceAlphaBounds = this->faceAlphaRectangleBounds();
  Amg::Vector3D faceAlphaPosition0(
    -0.5 * (this->minHalflengthX() + this->maxHalflengthX()), 0., 0.);
  Amg::Vector3D faceAlphaPosition = transform * faceAlphaPosition0;
  retsf.push_back(std::make_unique<Trk::PlaneSurface>( Amg::Translation3D(faceAlphaPosition)
                                                       * Amg::Transform3D(trapezoidRotation * rotateToFaceAlpha),
                                                       faceAlphaBounds));
  //   (4) - at point B, attached to beta opening angle
  //  same as above but rotate to the right by beta
  Amg::RotationMatrix3D rotateToFaceBeta;
  {
  double s=sin(beta());
  double c=cos(beta());
  rotateToFaceBeta  <<   c,  0.f,   s,
                        -s,  0.f,   c,
                       0.f, -1.f, 0.f;
  }

  std::shared_ptr<RectangleBounds> faceBetaBounds = this->faceBetaRectangleBounds();
  // Amg::Vector3D
  // faceBetaPosition(B+faceBetaRotation.colX()*faceBetaBounds->halflengthX());
  Amg::Vector3D faceBetaPosition0(
    0.5 * (this->minHalflengthX() + this->maxHalflengthX()), 0., 0.);
  Amg::Vector3D faceBetaPosition = transform * faceBetaPosition0;
  retsf.push_back(std::make_unique<Trk::PlaneSurface>( Amg::Translation3D(faceBetaPosition)
                                                       * Amg::Transform3D(trapezoidRotation * rotateToFaceBeta),
                                                       faceBetaBounds));
  retsf.push_back(std::make_unique<Trk::PlaneSurface>(
    Amg::Transform3D(
      transform *
      Amg::AngleAxis3D(180. * Gaudi::Units::deg, Amg::Vector3D(1., 0., 0.)) *
      Amg::Translation3D(Amg::Vector3D(0., this->halflengthY(), 0.)) *
      Amg::AngleAxis3D(-90 * Gaudi::Units::deg, Amg::Vector3D(0., 1., 0.)) *
      Amg::AngleAxis3D(-90. * Gaudi::Units::deg, Amg::Vector3D(1., 0., 0.))),
    this->faceZXRectangleBoundsBottom()));
  //   (6) - at positive local x
  retsf.push_back(std::make_unique<Trk::PlaneSurface>(
    Amg::Transform3D(
      transform *
      Amg::Translation3D(Amg::Vector3D(0., this->halflengthY(), 0.)) *
      Amg::AngleAxis3D(-90 * Gaudi::Units::deg, Amg::Vector3D(0., 1., 0.)) *
      Amg::AngleAxis3D(-90. * Gaudi::Units::deg, Amg::Vector3D(1., 0., 0.))),
    this->faceZXRectangleBoundsTop()));

  return retsf;
}

// faces in xy
std::shared_ptr<Trk::TrapezoidBounds>
Trk::TrapezoidVolumeBounds::faceXYTrapezoidBounds() const
{
  return std::make_shared<Trk::TrapezoidBounds>(m_minHalfX, m_maxHalfX, m_halfY);
}

std::shared_ptr<Trk::RectangleBounds>
Trk::TrapezoidVolumeBounds::faceAlphaRectangleBounds() const
{
  return std::make_shared<Trk::RectangleBounds>(m_halfY / cos(m_alpha - 0.5 * M_PI), m_halfZ);
}

std::shared_ptr<Trk::RectangleBounds>
Trk::TrapezoidVolumeBounds::faceBetaRectangleBounds() const
{
  return std::make_shared<Trk::RectangleBounds>(m_halfY / cos(m_beta - 0.5 * M_PI), m_halfZ);
}

std::shared_ptr<Trk::RectangleBounds>
Trk::TrapezoidVolumeBounds::faceZXRectangleBoundsBottom() const
{
  return std::make_shared<Trk::RectangleBounds>(m_halfZ, m_minHalfX);
}

std::shared_ptr<Trk::RectangleBounds>
Trk::TrapezoidVolumeBounds::faceZXRectangleBoundsTop() const
{
  // double delta = (m_alpha < m_beta) ? m_alpha - M_PI/2. : m_beta - M_PI/2.;
  // return new Trk::RectangleBounds(m_halfZ,
  // 0.5*(m_minHalfX+m_minHalfX+2.*m_halfY/cos(delta)));
  return std::make_shared<Trk::RectangleBounds>(m_halfZ, m_maxHalfX);
}

bool
Trk::TrapezoidVolumeBounds::inside(const Amg::Vector3D& pos, double tol) const
{
  if (std::abs(pos.z()) > m_halfZ + tol)
    return false;
  if (std::abs(pos.y()) > m_halfY + tol)
    return false;
  std::shared_ptr<Trk::TrapezoidBounds> faceXYBounds = this->faceXYTrapezoidBounds();
  Amg::Vector2D locp(pos.x(), pos.y());
  bool inside(faceXYBounds->inside(locp, tol, tol));
  return inside;
}

// ostream operator overload
MsgStream&
Trk::TrapezoidVolumeBounds::dump(MsgStream& sl) const
{
  std::stringstream temp_sl;
  temp_sl << std::setiosflags(std::ios::fixed);
  temp_sl << std::setprecision(7);
  temp_sl
    << "Trk::TrapezoidVolumeBounds: (minhalfX, halfY, halfZ, alpha, beta) = ";
  temp_sl << "(" << m_minHalfX << ", " << m_halfY << ", " << m_halfZ;
  temp_sl << ", " << m_alpha << ", " << m_beta << ")";
  sl << temp_sl.str();
  return sl;
}

std::ostream&
Trk::TrapezoidVolumeBounds::dump(std::ostream& sl) const
{
  std::stringstream temp_sl;
  temp_sl << std::setiosflags(std::ios::fixed);
  temp_sl << std::setprecision(7);
  temp_sl
    << "Trk::TrapezoidVolumeBounds: (minhalfX, halfY, halfZ, alpha, beta) = ";
  temp_sl << "(" << m_minHalfX << ", " << m_halfY << ", " << m_halfZ;
  temp_sl << ", " << m_alpha << ", " << m_beta << ")";
  sl << temp_sl.str();
  return sl;
}

