/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// AmgMatrixBasePlugin.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef EVENTPRIMITIVES_AMGMATRIXBASEPLUGIN_H
#define EVENTPRIMITIVES_AMGMATRIXBASEPLUGIN_H

#include <cmath>


/** This is a plugin that makes Eigen look like CLHEP
  & defines some convenience methods */

// ------- Methods for 3D vector type objects ---------------------- //

/** unit method - forward normalized() */
inline const PlainObject unit() const {
  return (*this).normalized();
}

/** mag method  */
inline Scalar mag() const {
  return (*this).norm();
}

/** mag2 method - forward to squaredNorm() */
inline Scalar mag2() const {
  return (*this).squaredNorm();
}

/** perp2 method - perpendicular length squared */
inline Scalar perp2() const {
  constexpr int size = Eigen::MatrixBase<Derived>::SizeAtCompileTime;
  constexpr int isVector = Eigen::MatrixBase<Derived>::IsVectorAtCompileTime;
  static_assert(isVector && size>=2, "Method applicable for vectors of size >=2");
  return ((*this)[0] * (*this)[0] + (*this)[1] * (*this)[1]);
}

/** perp method - perpendicular length */
inline Scalar perp() const {
  constexpr int size = Eigen::MatrixBase<Derived>::SizeAtCompileTime;
  constexpr int isVector = Eigen::MatrixBase<Derived>::IsVectorAtCompileTime;
  static_assert(isVector && size>=2, "Method applicable for vectors of size >=2");
  return std::sqrt(this->perp2());
}

/** perp2 method wrt another vector */
inline Scalar perp2(const MatrixBase<Derived>& vec) {
  Scalar tot = vec.mag2();
  if (tot > 0) {
    Scalar s = this->dot(vec);
    return this->mag2() - s * s / tot;
  }
  return this->mag2();
}

/** perp2 method wrt another vector */
inline Scalar perp(const MatrixBase<Derived>& vec) {
  return std::sqrt(this->perp2(vec));
}

/** phi method */
inline Scalar phi() const {
  constexpr int size = Eigen::MatrixBase<Derived>::SizeAtCompileTime;
  constexpr int isVector = Eigen::MatrixBase<Derived>::IsVectorAtCompileTime;
  static_assert(isVector && size>=2, "Method applicable for vectors of size >=2");
  return std::atan2((*this)[1], (*this)[0]);
}

/** theta method */
inline Scalar theta() const {
  constexpr int size = Eigen::MatrixBase<Derived>::SizeAtCompileTime;
  constexpr int isVector = Eigen::MatrixBase<Derived>::IsVectorAtCompileTime;
  static_assert(isVector && size>=3, "Method applicable for vectors of size >=3");
  return std::atan2(this->perp(), (*this)[2]);
}

/** pseudorapidity  method */
inline Scalar eta() const {
  constexpr int size = Eigen::MatrixBase<Derived>::SizeAtCompileTime;
  constexpr int isVector = Eigen::MatrixBase<Derived>::IsVectorAtCompileTime;
  static_assert(isVector && size>=3, "Method applicable for vectors of size >=3");
  const Scalar rho2 = (*this).x() * (*this).x() + (*this).y() * (*this).y();
  const Scalar z = (*this).z();
  const Scalar z2 = z * z;
  constexpr Scalar epsilon = 2. * std::numeric_limits<Scalar>::epsilon();
  // avoid magnitude being parallel to z
  if (rho2 >  z2 * epsilon) {
    const double m = std::sqrt(rho2 + z2);
    return 0.5 * std::log((m + z) / (m - z));
  }
  if (z == 0) {
    return 0.0;
  }
  // Following math/genvector/inc/Math/GenVector/etaMax.h in ROOT 6.26
  constexpr Scalar s_etaMax = static_cast<Scalar>(22756.0);
  // Following math/genvector/inc/Math/GenVector/eta.h in ROOT 6.26
  return (z > 0) ? z + s_etaMax : z - s_etaMax;
}

inline Scalar deltaR(const MatrixBase<Derived>& vec) const {
  //we assert in eta/phi methods
  double a = this->eta() - vec.eta();
  double b = this->deltaPhi(vec);
  return std::sqrt(a * a + b * b);
}

inline Scalar deltaPhi(const MatrixBase<Derived>& vec) const {
  //we assert in eta/phi methods
  double dphi = vec.phi() - this->phi();
  if (dphi > M_PI) {
    dphi -= M_PI * 2;
  } else if (dphi <= -M_PI) {
    dphi += M_PI * 2;
  }
  return dphi;
}

/** method to fill elements for a symmetric matrix*/
inline void fillSymmetric(size_t i, size_t j, Scalar value) {
  (*this)(i, j) = value;
  (*this)(j, i) = value;
}

/** similarity method : yields ms = m*s*m^T */
template <typename OtherDerived>
inline Matrix<Scalar, OtherDerived::RowsAtCompileTime,
              OtherDerived::RowsAtCompileTime>
similarity(const MatrixBase<OtherDerived>& m) const {
  return m * (this->derived() * m.transpose());
}

/** similarityT method : yields ms = m^T*s*m */
template <typename OtherDerived>
inline Matrix<Scalar, OtherDerived::RowsAtCompileTime,
              OtherDerived::RowsAtCompileTime>
similarityT(const MatrixBase<OtherDerived>& m) const {
  return m.transpose() * (this->derived() * m);
}

#endif
