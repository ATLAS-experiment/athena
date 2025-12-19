/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// DiscSurface.cxx, (c) ATLAS Detector Software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkSurfaces/DiscSurface.h"
#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkSurfaces/DiscBounds.h"
#include "TrkSurfaces/DiscTrapezoidalBounds.h"
#include "TrkSurfaces/AnnulusBounds.h"
#include "TrkSurfaces/AnnulusBoundsPC.h"
// CxxUtils
#include "CxxUtils/inline_hints.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
// Eigen
#include "GeoPrimitives/GeoPrimitives.h"

const Trk::NoBounds Trk::DiscSurface::s_boundless;

// default constructor
Trk::DiscSurface::DiscSurface()
  : Trk::Surface()
  , m_bounds(nullptr)
  , m_referencePoint(nullptr)
{}

// copy constructor
Trk::DiscSurface::DiscSurface(const DiscSurface& dsf)
  : Trk::Surface(dsf)
  , m_bounds(dsf.m_bounds)
  , m_referencePoint(nullptr)
{}

// copy constructor with shift
Trk::DiscSurface::DiscSurface(const DiscSurface& dsf,
                              const Amg::Transform3D& transf)
  : Trk::Surface(dsf, transf)
  , m_bounds(dsf.m_bounds)
  , m_referencePoint(nullptr)
{}

// construct a disc with full phi coverage
Trk::DiscSurface::DiscSurface(const Amg::Transform3D& htrans,
                              double rmin,
                              double rmax)
  : Trk::Surface(htrans)
  , m_bounds(std::make_shared<const Trk::DiscBounds>(rmin, rmax))
  , m_referencePoint(nullptr)
{}

// construct a disc with given phi coverage
Trk::DiscSurface::DiscSurface(const Amg::Transform3D& htrans,
                              double rmin,
                              double rmax,
                              double hphisec)
  : Trk::Surface(htrans)
  , m_bounds(std::make_shared<Trk::DiscBounds>(rmin, rmax, hphisec))
  , m_referencePoint(nullptr)
{}

Trk::DiscSurface::DiscSurface(const Amg::Transform3D& htrans,
                              double minhalfx,
                              double maxhalfx,
                              double maxR,
                              double minR,
                              double avephi,
                              double stereo)
  : Trk::Surface(htrans)
  , m_bounds(std::make_shared<Trk::DiscTrapezoidalBounds>(minhalfx,
                                                          maxhalfx,
                                                          maxR,
                                                          minR,
                                                          avephi,
                                                          stereo))
  , m_referencePoint(nullptr)
{}

// construct a disc with given bounds
Trk::DiscSurface::DiscSurface(const Amg::Transform3D& htrans,
                              std::shared_ptr<const Trk::DiscBounds> dbounds)
  : Trk::Surface(htrans)
  , m_bounds(std::move(dbounds))
  , m_referencePoint(nullptr)
{}

// construct a disc with given bounds
Trk::DiscSurface::DiscSurface(const Amg::Transform3D& htrans,
                              std::shared_ptr<const Trk::DiscTrapezoidalBounds> dbounds)
  : Trk::Surface(htrans)
  , m_bounds(std::move(dbounds))
  , m_referencePoint(nullptr)
{}

Trk::DiscSurface::DiscSurface(const Amg::Transform3D& htrans,
                              std::shared_ptr<const Trk::AnnulusBoundsPC> annpcbounds)
  : Trk::Surface(htrans),
  m_bounds(std::move(annpcbounds)),
  m_referencePoint(nullptr)
{}

Trk::DiscSurface::DiscSurface(const Amg::Transform3D& htrans,
                              const Trk::AnnulusBounds& annbounds,
                              const TrkDetElementBase* detElem)
  : Trk::Surface(htrans),
    m_referencePoint(nullptr)
{

  if(detElem != nullptr) {
    m_associatedDetElement = detElem;
    m_associatedDetElementId = m_associatedDetElement->identify();
  }

  // build AnnulusBoundsPC from XY AnnulusBounds
  std::pair<AnnulusBoundsPC, double> res = AnnulusBoundsPC::fromCartesian(annbounds);
  m_bounds = std::make_shared<AnnulusBoundsPC>(res.first);
  double phiShift = res.second;
  // construct shifted transform
  // we get the necessary rotation from ::fromCartesian(), and we need to make
  // the local coordinate system to be rotated correctly here
  Amg::Vector2D origin2D = res.first.moduleOrigin();
  Amg::Translation3D transl(Amg::Vector3D(origin2D.x(), origin2D.y(), 0));
  Amg::Rotation3D rot(Amg::AngleAxis3D(-phiShift, Amg::Vector3D::UnitZ()));
  Amg::Transform3D originTrf;
  originTrf = transl * rot;

  m_transforms->transform = m_transforms->transform * originTrf.inverse();
}

// construct a disc from a transform, bounds is not set.
Trk::DiscSurface::DiscSurface(const Amg::Transform3D& htrans)
  : Trk::Surface(htrans)
  , m_bounds(nullptr)
  , m_referencePoint(nullptr)
{}

// construct form TrkDetElementBase
Trk::DiscSurface::DiscSurface(const Trk::TrkDetElementBase& detelement)
  : Trk::Surface(detelement)
  , m_bounds(nullptr)
  , m_referencePoint(nullptr)
{}

// Out-of-line dtor.
Trk::DiscSurface::~DiscSurface() = default;

Trk::DiscSurface&
Trk::DiscSurface::operator=(const DiscSurface& dsf)
{
  if (this != &dsf) {
    Trk::Surface::operator=(dsf);
    m_bounds = dsf.m_bounds;
    m_referencePoint.store(nullptr);
  }
  return *this;
}

bool
Trk::DiscSurface::operator==(const Trk::Surface& sf) const
{
  // first check the type not to compare apples with oranges
  if (sf.type()!=Trk::SurfaceType::Disc){
      return false;
  }
  return (*this) == static_cast<const Trk::DiscSurface&>(sf);
}

/* Use the Surface as a ParametersBase constructor, from local parameters -
 * charged */
Trk::Surface::ChargedTrackParametersUniquePtr
Trk::DiscSurface::createUniqueTrackParameters(
    double l1, double l2, double phi, double theta, double qop,
    std::optional<AmgSymMatrix(5)> cov) const {
  return std::make_unique<ParametersT<5, Charged, DiscSurface>>(
      l1, l2, phi, theta, qop, *this, std::move(cov));
}
/** Use the Surface as a ParametersBase constructor, from global parameters -
 * charged*/
Trk::Surface::ChargedTrackParametersUniquePtr
Trk::DiscSurface::createUniqueTrackParameters(
    const Amg::Vector3D& position, const Amg::Vector3D& momentum, double charge,
    std::optional<AmgSymMatrix(5)> cov) const {
  return std::make_unique<ParametersT<5, Charged, DiscSurface>>(
      position, momentum, charge, *this, std::move(cov));
}

/** Use the Surface as a ParametersBase constructor, from local parameters -
 * neutral */
Trk::Surface::NeutralTrackParametersUniquePtr
Trk::DiscSurface::createUniqueNeutralParameters(
    double l1, double l2, double phi, double theta, double qop,
    std::optional<AmgSymMatrix(5)> cov) const {
  return std::make_unique<ParametersT<5, Neutral, DiscSurface>>(
      l1, l2, phi, theta, qop, *this, std::move(cov));
}

/** Use the Surface as a ParametersBase constructor, from global parameters -
 * neutral */
Trk::Surface::NeutralTrackParametersUniquePtr
Trk::DiscSurface::createUniqueNeutralParameters(
    const Amg::Vector3D& position, const Amg::Vector3D& momentum, double charge,
    std::optional<AmgSymMatrix(5)> cov) const {
  return std::make_unique<ParametersT<5, Neutral, DiscSurface>>(
      position, momentum, charge, *this, std::move(cov));
}

const Amg::Vector3D&
Trk::DiscSurface::globalReferencePoint() const
{
  if (!m_referencePoint) {
    const Trk::DiscBounds* dbo =
      dynamic_cast<const Trk::DiscBounds*>(&(bounds()));
    if (dbo) {
      double rMedium = bounds().r();
      double phi = dbo->averagePhi();
      Amg::Vector3D gp(rMedium * cos(phi), rMedium * sin(phi), 0.);
      m_referencePoint.set(std::make_unique<Amg::Vector3D>(transform() * gp));
    } else {
      const Trk::DiscTrapezoidalBounds* dtbo =
        dynamic_cast<const Trk::DiscTrapezoidalBounds*>(&(bounds()));
      // double rMedium = dtbo ? bounds().r() : dtbo->rCenter() ; //nonsense, or
      // logic inverted?
      double rMedium = bounds().r();
      double phi = dtbo ? dtbo->averagePhi() : 0.;
      Amg::Vector3D gp(rMedium * cos(phi), rMedium * sin(phi), 0.);
      m_referencePoint.set(std::make_unique<Amg::Vector3D>(transform() * gp));
    }
  }
  return (*m_referencePoint);
}

// Avoid out-of-line Eigen calls
ATH_FLATTEN
void
Trk::DiscSurface::localToGlobal(const Amg::Vector2D& locpos,
                                const Amg::Vector3D&,
                                Amg::Vector3D& glopos) const
{
  // create the position in the local 3d frame
  Amg::Vector3D loc3Dframe(locpos[Trk::locR] * cos(locpos[Trk::locPhi]),
                           locpos[Trk::locR] * sin(locpos[Trk::locPhi]),
                           0.);
  // transport it to the globalframe
  glopos = transform() * loc3Dframe;
}

/** local<->global transformation in case of polar local coordinates */
bool
Trk::DiscSurface::globalToLocal(const Amg::Vector3D& glopos,
                                const Amg::Vector3D&,
                                Amg::Vector2D& locpos) const
{
  Amg::Vector3D loc3Dframe = inverseTransformMultHelper(glopos);
  locpos = Amg::Vector2D(loc3Dframe.perp(), loc3Dframe.phi());
  return (std::fabs(loc3Dframe.z()) <= s_onSurfaceTolerance);
}

Trk::Intersection
Trk::DiscSurface::straightLineIntersection(const Amg::Vector3D& pos,
                                           const Amg::Vector3D& dir,
                                           bool forceDir,
                                           Trk::BoundaryCheck bchk) const {
  double denom = dir.dot(normal());
  if (denom) {
    double u = (normal().dot((center() - pos))) / (denom);
    Amg::Vector3D intersectPoint(pos + u * dir);
    // evaluate the intersection in terms of direction
    bool isValid = forceDir ? (u > 0.) : true;
    // evaluate (if necessary in terms of boundaries)
    isValid = bchk ? (isValid && isOnSurface(intersectPoint)) : isValid;
    // return the result
    return Trk::Intersection(intersectPoint, u, isValid);
  }
  return Trk::Intersection(pos, 0., false);
}

#if defined(FLATTEN)
// We compile this function with optimization, even in debug builds; otherwise,
// the heavy use of Eigen makes it too slow.  However, from here we may call
// to out-of-line Eigen code that is linked from other DSOs; in that case,
// it would not be optimized.  Avoid this by forcing all Eigen code
// to be inlined here if possible.
ATH_FLATTEN
#endif
bool
Trk::DiscSurface::isOnSurface(const Amg::Vector3D& glopo,
                              const Trk::BoundaryCheck& bchk,
                              double tol1,
                              double tol2) const
{
  Amg::Vector3D loc3Dframe = inverseTransformMultHelper(glopo);
  if (std::abs(loc3Dframe.z()) > (s_onSurfaceTolerance + tol1)) {
    return false;
  }
  return (bchk
            ? bounds().inside(
                Amg::Vector2D(loc3Dframe.perp(), loc3Dframe.phi()), tol1, tol2)
            : true);
}

/** distance to surface */
Trk::DistanceSolution
Trk::DiscSurface::straightLineDistanceEstimate(const Amg::Vector3D& pos,
                                               const Amg::Vector3D& dir) const
{
  double tol = 0.001;

  const Amg::Vector3D& C = center();
  const Amg::Vector3D& N = normal();

  double S = C.dot(N);
  double b = S < 0. ? -1 : 1;
  double d = (pos - C).dot(N); // distance to surface

  double A = b * dir.dot(N);
  if (A == 0.) { // direction parallel to surface
    if (fabs(d) < tol) {
      return {1, 0., true, 0.};
    }
    return {0, d, true, 0.};
  }

  double D = b * (S - (pos.dot(N))) / A;
  return {1, d, true, D};
}

Trk::DistanceSolution
Trk::DiscSurface::straightLineDistanceEstimate(const Amg::Vector3D& pos,
                                               const Amg::Vector3D& dir,
                                               bool bound) const
{
  const Amg::Transform3D& T = transform();
  const double Az[3] = { T(0, 2), T(1, 2), T(2, 2) };

  // Transformation to cylinder system coordinates
  //
  const double dx = pos[0] - T(0, 3);
  const double dy = pos[1] - T(1, 3);
  const double dz = pos[2] - T(2, 3);
  const double z = dx * Az[0] + dy * Az[1] + dz * Az[2];
  const double az = dir[0] * Az[0] + dir[1] * Az[1] + dir[2] * Az[2];

  // Step to surface
  //
  int ns = 0;
  double s = 0.;
  if (az != 0.) {
    s = -z / az;
    ns = 1;
  }
  double dist = std::abs(z);
  if (!bound) {
    return {ns, dist, true, s};
  }

  // Min distance to surface
  //
  const double x = dx * T(0, 0) + dy * T(1, 0) + dz * T(2, 0);
  const double y = dx * T(0, 1) + dy * T(1, 1) + dz * T(2, 1);

  Amg::Vector2D lp(sqrt(x * x + y * y), atan2(y, x));

  double d = bounds().minDistance(lp);
  if (d > 0.) {
    dist = std::sqrt(dist * dist + d * d);
  }

  return {ns, dist, true, s};
}
