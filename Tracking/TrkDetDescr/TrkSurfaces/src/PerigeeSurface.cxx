/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// PerigeeSurface.cxx, (c) ATLAS Detector Software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkSurfaces/PerigeeSurface.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
// STD
#include <iomanip>
#include <iostream>
// Eigen
#include "GeoPrimitives/GeoPrimitives.h"

const Trk::NoBounds Trk::PerigeeSurface::s_perigeeBounds;

Trk::PerigeeSurface::PerigeeSurface()
  : Surface()
  , m_lineDirection{}
{}

Trk::PerigeeSurface::PerigeeSurface(const Amg::Vector3D& gp)
  : Surface()
  , m_lineDirection{}
{
  Amg::Transform3D transform (Amg::Translation3D(gp.x(), gp.y(), gp.z()));
  Surface::m_transforms = std::make_unique<Transforms>(transform, gp, s_xAxis);
}

Trk::PerigeeSurface::PerigeeSurface(const Amg::Transform3D& tTransform)
  : Surface() // default for base
  , m_lineDirection{}
{

  Surface::m_transforms =
    std::make_unique<Transforms>(tTransform, tTransform.translation(), s_xAxis);
}

#if defined(FLATTEN)
// We compile this function with optimization, even in debug builds; otherwise,
// the heavy use of Eigen makes it too slow.  However, from here we may call
// to out-of-line Eigen code that is linked from other DSOs; in that case,
// it would not be optimized.  Avoid this by forcing all Eigen code
// to be inlined here if possible.
ATH_FLATTEN
#endif
Trk::PerigeeSurface::PerigeeSurface(const PerigeeSurface& pesf)
  : Surface(pesf)
  , m_lineDirection{}
{
  if (pesf.m_transforms) {
    Surface::m_transforms = std::make_unique<Transforms>(
      pesf.m_transforms->transform, pesf.m_transforms->center, s_xAxis);
  }
}

Trk::PerigeeSurface::PerigeeSurface(const PerigeeSurface& pesf, const Amg::Transform3D& shift)
  : Surface()
  , m_lineDirection{}
{
  if (pesf.m_transforms) {
    Surface::m_transforms =
      std::make_unique<Transforms>(shift * pesf.m_transforms->transform,
                                   shift * pesf.m_transforms->center,
                                   s_xAxis);
  }
}

// Out-of-line dtor.
Trk::PerigeeSurface::~PerigeeSurface() = default;

// assignment operator
Trk::PerigeeSurface&
Trk::PerigeeSurface::operator=(const Trk::PerigeeSurface& pesf)
{
  if (this != &pesf) {
    Trk::Surface::operator=(pesf);
    m_lineDirection=pesf.m_lineDirection;
  }
  return *this;
}

bool
Trk::PerigeeSurface::operator==(const Trk::Surface& sf) const
{
  // first check the type not to compare apples with oranges
  if (sf.type()!=Trk::SurfaceType::Perigee){
      return false;
  }
  return (*this) == static_cast<const Trk::PerigeeSurface&>(sf);
}

/** Use the Surface as a ParametersBase constructor, from local parameters -
 * charged */
Trk::Surface::ChargedTrackParametersUniquePtr
Trk::PerigeeSurface::createUniqueTrackParameters(
  double l1,
  double l2,
  double phi,
  double theta,
  double qop,
  std::optional<AmgSymMatrix(5)> cov) const
{
  return std::make_unique<ParametersT<5, Charged, PerigeeSurface>>(
    l1, l2, phi, theta, qop, *this, std::move(cov));
}
/** Use the Surface as a ParametersBase constructor, from global parameters -
 * charged*/
Trk::Surface::ChargedTrackParametersUniquePtr
Trk::PerigeeSurface::createUniqueTrackParameters(
  const Amg::Vector3D& position,
  const Amg::Vector3D& momentum,
  double charge,
  std::optional<AmgSymMatrix(5)> cov) const
{
  return std::make_unique<ParametersT<5, Charged, PerigeeSurface>>(
    position, momentum, charge, *this, std::move(cov));
}

/** Use the Surface as a ParametersBase constructor, from local parameters -
 * neutral */
Trk::Surface::NeutralTrackParametersUniquePtr
Trk::PerigeeSurface::createUniqueNeutralParameters(
  double l1,
  double l2,
  double phi,
  double theta,
  double qop,
  std::optional<AmgSymMatrix(5)> cov) const
{
  return std::make_unique<ParametersT<5, Neutral, PerigeeSurface>>(
    l1, l2, phi, theta, qop, *this, std::move(cov));
}

/** Use the Surface as a ParametersBase constructor, from global parameters -
 * neutral */
Trk::Surface::NeutralTrackParametersUniquePtr
Trk::PerigeeSurface::createUniqueNeutralParameters(
  const Amg::Vector3D& position,
  const Amg::Vector3D& momentum,
  double charge,
  std::optional<AmgSymMatrix(5)> cov) const
{
  return std::make_unique<ParametersT<5, Neutral, PerigeeSurface>>(
    position, momentum, charge, *this, std::move(cov));
}

// simple local to global - from LocalParameters /
Amg::Vector3D
Trk::PerigeeSurface::localToGlobal(const Trk::LocalParameters& locpars) const
{
  if (locpars.contains(Trk::phi0)) {
    Amg::Vector3D loc3Dframe(-locpars[Trk::d0] * sin(locpars[Trk::phi0]),
                             locpars[Trk::d0] * cos(locpars[Trk::phi0]),
                             locpars[Trk::z0]);
    return Amg::Vector3D(transform() * loc3Dframe);
  }
  return {0., 0., locpars[Trk::z0] + (center().z())};
}

// We compile this function with optimization, even in debug builds; otherwise,
// the heavy use of Eigen makes it too slow.  However, from here we may call
// to out-of-line Eigen code that is linked from other DSOs; in that case,
// it would not be optimized.  Avoid this by forcing all Eigen code
// to be inlined here if possible.
ATH_FLATTEN
// true local to global method/
void
Trk::PerigeeSurface::localToGlobal(const Amg::Vector2D& locpos,
                                   const Amg::Vector3D& glomom,
                                   Amg::Vector3D& glopos) const
{
  // this is for a tilted perigee surface
  if (Surface::m_transforms) {
    // get the vector perpendicular to the momentum and the straw axis
    Amg::Vector3D radiusAxisGlobal(lineDirection().cross(glomom));
    Amg::Vector3D locZinGlobal = transform() * Amg::Vector3D(0., 0., locpos[Trk::locZ]);
    // transform zPosition into global coordinates and add locR * radiusAxis
    glopos = Amg::Vector3D(locZinGlobal + locpos[Trk::locR] * radiusAxisGlobal.normalized());
  } else {
    double phi = glomom.phi();
    glopos[Amg::x] = -locpos[Trk::d0] * sin(phi);
    glopos[Amg::y] = locpos[Trk::d0] * cos(phi);
    glopos[Amg::z] = locpos[Trk::z0];
    glopos += center();
  }
}

// true local to global method - from LocalParameters /
Amg::Vector3D
Trk::PerigeeSurface::localToGlobal(const Trk::LocalParameters& locpars,
                                   const Amg::Vector3D& glomom) const
{
  if (Surface::m_transforms) {
    Amg::Vector3D glopos = Amg::Vector3D(0., 0., 0.);
    localToGlobal(
      Amg::Vector2D(locpars[Trk::d0], locpars[Trk::z0]), glomom, glopos);
    return glopos;
  }
  double phi = glomom.phi();
  double x = -locpars[Trk::d0] * sin(phi) + center().x();
  double y = locpars[Trk::d0] * cos(phi) + center().y();
  double z = locpars[Trk::z0] + center().z();
  return {x, y, z};
}

// true global to local method
bool
Trk::PerigeeSurface::globalToLocal(const Amg::Vector3D& glopos,
                                   const Amg::Vector3D& glomom,
                                   Amg::Vector2D& locpos) const
{
  Amg::Vector3D perPos = inverseTransformMultHelper(glopos);
  double d0 = perPos.perp();
  double z0 = perPos.z();
  // decide the sign of d0
  d0 *= ((lineDirection().cross(glomom)).dot(perPos) < 0.0) ? -1.0 : 1.0;
  locpos[Trk::d0] = d0;
  locpos[Trk::z0] = z0;
  return true;
}

Trk::Intersection
Trk::PerigeeSurface::straightLineIntersection(const Amg::Vector3D& pos,
                                              const Amg::Vector3D& dir,
                                              bool forceDir,
                                              Trk::BoundaryCheck) const {
  // following nominclature found in header file and doxygen documentation
  // line one is the straight track
  const Amg::Vector3D& ma = pos;
  const Amg::Vector3D& ea = dir;
  // line two is the line surface
  const Amg::Vector3D& mb = center();
  const Amg::Vector3D& eb = lineDirection();
  // now go ahead
  Amg::Vector3D mab(mb - ma);
  double eaTeb = ea.dot(eb);
  double denom = 1 - eaTeb * eaTeb;
  if (std::abs(denom) > 10e-7) {
    double lambda0 = (mab.dot(ea) - mab.dot(eb) * eaTeb) / denom;
    // evaluate the direction, bounds are always true for Perigee
    bool isValid = forceDir ? (lambda0 > 0.) : true;
    return Trk::Intersection((ma + lambda0 * ea), lambda0, isValid);
  }
  return Trk::Intersection(pos, 0., false);
}

// return the measurement frame - this is the frame where the covariance is defined
Amg::RotationMatrix3D
Trk::PerigeeSurface::measurementFrame(const Amg::Vector3D&, const Amg::Vector3D& glomom) const
{
  Amg::RotationMatrix3D mFrame;
  // construct the measurement frame
  const Amg::Vector3D& measY = lineDirection();
  Amg::Vector3D measX(measY.cross(glomom).unit());
  Amg::Vector3D measDepth(measX.cross(measY));
  // assign the columnes
  mFrame.col(0) = measX;
  mFrame.col(1) = measY;
  mFrame.col(2) = measDepth;
  // return the rotation matrix
  return mFrame;
}

/** distance to surface */
Trk::DistanceSolution
Trk::PerigeeSurface::straightLineDistanceEstimate(const Amg::Vector3D& pos, const Amg::Vector3D& dir) const
{
  const Amg::Vector3D& C = center();
  Amg::Vector3D S(0., 0., 1.);

  double D = dir.dot(S);
  double A = (1. - D) * (1. + D);
  if (A == 0.) {
    return {1, pos.perp(), false, 0.};
  }
  double sol = (pos - C).dot(D * S - dir) / A;
  return {1, pos.perp(), false, sol};
}

Trk::DistanceSolution
Trk::PerigeeSurface::straightLineDistanceEstimate(const Amg::Vector3D& pos, const Amg::Vector3D& dir, bool) const
{
  const Amg::Transform3D& T = transform();

  double dx = pos[0] - T(3, 0);
  double dy = pos[1] - T(3, 1);
  double A = dir[0] * dir[0] + dir[1] * dir[1];

  // Step to surface
  //
  if (A > 0.) {
    return {1, 0., false, -(dir[0] * dx + dir[1] * dy) / A};
  }
  return {1, 0., false, 0.};
}

// overload of ostream operator
MsgStream&
Trk::PerigeeSurface::dump(MsgStream& sl) const
{
  sl << std::setiosflags(std::ios::fixed);
  sl << std::setprecision(7);
  sl << "Trk::PerigeeSurface:" << std::endl;
  sl << "     Center position  (x, y, z) = (" << center().x() << ", " << center().y() << ", " << center().z() << ")";
  sl << std::setprecision(-1);
  return sl;
}

// overload of ostream operator
std::ostream&
Trk::PerigeeSurface::dump(std::ostream& sl) const
{
  sl << std::setiosflags(std::ios::fixed);
  sl << std::setprecision(7);
  sl << "Trk::PerigeeSurface:" << std::endl;
  sl << "     Center position  (x, y, z) = (" << center().x() << ", " << center().y() << ", " << center().z() << ")";
  sl << std::setprecision(-1);
  return sl;
}
