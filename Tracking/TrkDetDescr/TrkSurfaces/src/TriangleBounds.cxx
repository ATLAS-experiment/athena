/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TriangleBounds.cxx, (c) ATLAS Detector Software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkSurfaces/TriangleBounds.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
// STD
#include <iomanip>
#include <iostream>
#include "CxxUtils/inline_hints.h"

// default constructor
Trk::TriangleBounds::TriangleBounds()
  : m_boundValues(TriangleBounds::bv_length, 0.)
{}

// rectangle constructor - float constructor
Trk::TriangleBounds::TriangleBounds(const std::vector<std::pair<float, float>>& vertices)
  : m_boundValues(TriangleBounds::bv_length, 0.)
{
  size_t ib = 0;
  for (const std::pair<float, float>& p : vertices) {
    m_boundValues[2 * ib] = p.first;
    m_boundValues[2 * ib + 1] = p.second;
    if (ib == 2)
      break;
    ++ib;
  }
}

    // Inline Eigen in dbg builds.
    ATH_FLATTEN
// rectangle constructor - double constructor
Trk::TriangleBounds::TriangleBounds(const std::vector<std::pair<double, double>>& vertices)
  : m_boundValues(TriangleBounds::bv_length, 0.)
{
  size_t ib = 0;
  for (const std::pair<double, double>& p : vertices) {
    m_boundValues[2 * ib] = p.first;
    m_boundValues[2 * ib + 1] = p.second;
    if (ib == 2)
      break;
    ++ib;
  }
}

// constructor from three points
Trk::TriangleBounds::TriangleBounds(const Amg::Vector2D& p1, const Amg::Vector2D& p2, const Amg::Vector2D& p3)
  : m_boundValues(TriangleBounds::bv_length, 0.)
{
  m_boundValues[TriangleBounds::bv_x1] = p1.x();
  m_boundValues[TriangleBounds::bv_y1] = p1.y();
  m_boundValues[TriangleBounds::bv_x2] = p2.x();
  m_boundValues[TriangleBounds::bv_y2] = p2.y();
  m_boundValues[TriangleBounds::bv_x3] = p3.x();
  m_boundValues[TriangleBounds::bv_y3] = p3.y();
}

bool
Trk::TriangleBounds::operator==(const Trk::SurfaceBounds& sbo) const
{
  // check the type first not to compare apples with oranges
  const Trk::TriangleBounds* tribo = dynamic_cast<const Trk::TriangleBounds*>(&sbo);
  if (!tribo)
    return false;
  return (m_boundValues == tribo->m_boundValues);
}

bool
Trk::TriangleBounds::inside(const Amg::Vector2D& locpo,
                            double tol1,
                            double tol2) const
{
  std::pair<double, double> locB(m_boundValues[TriangleBounds::bv_x2] -
                                   m_boundValues[TriangleBounds::bv_x1],
                                 m_boundValues[TriangleBounds::bv_y2] -
                                   m_boundValues[TriangleBounds::bv_y1]);
  std::pair<double, double> locT(
    m_boundValues[TriangleBounds::bv_x3] - locpo[0],
    m_boundValues[TriangleBounds::bv_y3] - locpo[1]);
  std::pair<double, double> locV(
    m_boundValues[TriangleBounds::bv_x1] - locpo[0],
    m_boundValues[TriangleBounds::bv_y1] - locpo[1]);

  // special case :: third vertex ?
  if (locT.first * locT.first + locT.second * locT.second < tol1 * tol1)
    return true;

  // special case : lies on base ?
  double db = locB.first * locV.second - locB.second * locV.first;
  if (std::abs(db) < tol1) {
    double a =
      (locB.first != 0) ? -locV.first / locB.first : -locV.second / locB.second;
    return a > -tol2 && a - 1. < tol2;
  }

  double dn = locB.first * locT.second - locB.second * locT.first;

  if (std::abs(dn) > std::abs(tol1)) {
    double t = (locB.first * locV.second - locB.second * locV.first) / dn;
    if (t > 0.)
      return false;

    double a = (locB.first != 0.)
                 ? (t * locT.first - locV.first) / locB.first
                 : (t * locT.second - locV.second) / locB.second;
    if (a < -tol2 || a - 1. > tol2)
      return false;
  } else {
    return false;
  }
  return true;
}

bool
Trk::TriangleBounds::inside(const Amg::Vector2D& locpo,
                            const BoundaryCheck& bchk) const
{
  if (bchk.bcType == 0)
    return TriangleBounds::inside(
      locpo, bchk.toleranceLoc1, bchk.toleranceLoc2);

  // a fast FALSE
  double fabsR = std::sqrt(locpo[Trk::locX] * locpo[Trk::locX] +
                           locpo[Trk::locY] * locpo[Trk::locY]);
  double max_ell = bchk.lCovariance(0, 0) > bchk.lCovariance(1, 1)
                     ? bchk.lCovariance(0, 0)
                     : bchk.lCovariance(1, 1);
  double limit = bchk.nSigmas * std::sqrt(max_ell);
  double r_max = TriangleBounds::r();
  if (fabsR > (r_max + limit))
    return false;

  // compute KDOP and axes for surface polygon
  std::vector<KDOP> elementKDOP(3);
  std::vector<Amg::Vector2D> elementP(3);
  float theta =
    (bchk.lCovariance(1, 0) != 0 &&
     (bchk.lCovariance(1, 1) - bchk.lCovariance(0, 0)) != 0)
      ? .5 * bchk.FastArcTan(2 * bchk.lCovariance(1, 0) /
                             (bchk.lCovariance(1, 1) - bchk.lCovariance(0, 0)))
      : 0.;
  sincosCache scResult = bchk.FastSinCos(theta);
  AmgMatrix(2, 2) rotMatrix;
  rotMatrix << scResult.cosC, scResult.sinC, -scResult.sinC, scResult.cosC;
  AmgMatrix(2, 2) normal;
  // cppcheck-suppress constStatement
  normal << 0, -1, 1, 0;
  // ellipse is always at (0,0), surface is moved to ellipse position and then
  // rotated
  Amg::Vector2D p;
  p << m_boundValues[TriangleBounds::bv_x1],
    m_boundValues[TriangleBounds::bv_y1];
  elementP[0] = (rotMatrix * (p - locpo));
  p << m_boundValues[TriangleBounds::bv_x2],
    m_boundValues[TriangleBounds::bv_y2];
  elementP[1] = (rotMatrix * (p - locpo));
  p << m_boundValues[TriangleBounds::bv_x3],
    m_boundValues[TriangleBounds::bv_y3];
  elementP[2] = (rotMatrix * (p - locpo));
  std::vector<Amg::Vector2D> axis = { normal * (elementP[1] - elementP[0]),
                                      normal * (elementP[2] - elementP[1]),
                                      normal * (elementP[2] - elementP[0]) };
  bchk.ComputeKDOP(elementP, axis, elementKDOP);
  // compute KDOP for error ellipse
  std::vector<KDOP> errelipseKDOP(3);
  bchk.ComputeKDOP(bchk.EllipseToPoly(3), axis, errelipseKDOP);
  // check if KDOPs overlap and return result
  return bchk.TestKDOPKDOP(elementKDOP, errelipseKDOP);
}

double
Trk::TriangleBounds::minDistance(const Amg::Vector2D& pos) const
{
  const int Np = 3;

  double X[3] = { m_boundValues[TriangleBounds::bv_x1],
                  m_boundValues[TriangleBounds::bv_x2],
                  m_boundValues[TriangleBounds::bv_x3] };
  double Y[3] = { m_boundValues[TriangleBounds::bv_y1],
                  m_boundValues[TriangleBounds::bv_y2],
                  m_boundValues[TriangleBounds::bv_y3] };

  double dm = 1.e+20;
  double Ao = 0.;
  bool in = true;

  for (int i = 0; i != Np; ++i) {

    int j = (i == Np-1 ? 0 : i+1);

    double x = X[i] - pos[0];
    double y = Y[i] - pos[1];
    double dx = X[j] - X[i];
    double dy = Y[j] - Y[i];
    double A = x * dy - y * dx;
    double S = -(x * dx + y * dy);

    if (S <= 0.) {
      double d = x * x + y * y;
      if (d < dm)
        dm = d;
    } else {
      double a = dx * dx + dy * dy;
      if (S <= a) {
        double d = (A * A) / a;
        if (d < dm)
          dm = d;
      }
    }
    if (i && in && Ao * A < 0.)
      in = false;
    Ao = A;
  }
  if (in){
    return -sqrt(dm);
  }
  return sqrt(dm);
}

// ostream operator overload
MsgStream&
Trk::TriangleBounds::dump(MsgStream& sl) const
{
  sl << std::setiosflags(std::ios::fixed);
  sl << std::setprecision(7);
  sl << "Trk::TriangleBounds:  generating vertices (X, Y) " << '\n';
  sl << "(" << m_boundValues[TriangleBounds::bv_x1] << " , " << m_boundValues[TriangleBounds::bv_y1] << ") " << '\n';
  sl << "(" << m_boundValues[TriangleBounds::bv_x2] << " , " << m_boundValues[TriangleBounds::bv_y2] << ") " << '\n';
  sl << "(" << m_boundValues[TriangleBounds::bv_x3] << " , " << m_boundValues[TriangleBounds::bv_y3] << ") ";
  sl << std::setprecision(-1);
  return sl;
}

std::ostream&
Trk::TriangleBounds::dump(std::ostream& sl) const
{
  sl << std::setiosflags(std::ios::fixed);
  sl << std::setprecision(7);
  sl << "Trk::TriangleBounds:  generating vertices (X, Y)";
  sl << "(" << m_boundValues[TriangleBounds::bv_x1] << " , " << m_boundValues[TriangleBounds::bv_y1] << ") " << '\n';
  sl << "(" << m_boundValues[TriangleBounds::bv_x2] << " , " << m_boundValues[TriangleBounds::bv_y2] << ") " << '\n';
  sl << "(" << m_boundValues[TriangleBounds::bv_x3] << " , " << m_boundValues[TriangleBounds::bv_y3] << ") ";
  sl << std::setprecision(-1);
  return sl;
}
