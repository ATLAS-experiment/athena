/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// EventPrimitivesHelpers.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef EVENTPRIMITIVES_EVENTPRIMITIVESCOVARIANCEHELPERS_H
#define EVENTPRIMITIVES_EVENTPRIMITIVESCOVARIANCEHELPERS_H

#include "EventPrimitives/EventPrimitives.h"
//
#include <Eigen/Cholesky>
//
#include <limits>
/** Event Primitives Covariance Helper Functions
 @author  Christos Anastopoulos
 @author  Johannes Junggeburth
 */

namespace Amg {
/// A covariance matrix formally needs to be positive *semi* definite.
/// Not positive definite just positive *semi* definite.
///
/// A symmetric matrix M  with real entries is positive-definite
/// if the real number x^T M x is positive for every nonzero
/// real column vector x.
/// Positive-semidefinite means x^T M x non zero
/// for every nonzero real column vector x
///
/// A symmetric matrix is positive semidefinite
/// if all its eigenvalues are non negative.
///
/// A symmetric matrix is positive definite
/// if all its eigenvalues are positive
///
/// Positive Definite and Positive Semi Definit matrices
/// Have a Cholesky decomposition. The Positive (semi)
/// definiteness is a necessary and sufficient condition
///
/// A positive (semi)-definite matrix can not have
/// non-positive (negative) diagonal elements.
/// If A_ii < 0 we could choose an x vector with
/// all entries 0 bar x_i and the x^T M x would be
/// negative.
///
/// Having positive (positive or 0) diagonal elements
/// is *necessary* but not *sufficient* condition.
/// As we could have positive diagonal elements and have
/// a vector x that still could result in  x^T M x being negative.
/// Matrix A in the test shows this issue
///
/// What follows are methods to check for these
///
/// We can
/// - Just check for the *necessary* condition (relatively fast)
/// - Check using Cholosky decomposition  (still not too slow)
/// - Solve for all eigenvalues and check none
/// is zero (slow)

/// Avoid nan, inf, and elements above float max
inline bool saneCovarianceElement(double ele) {
  constexpr double upper_covariance_cutoff = std::numeric_limits<float>::max();
  return !(std::isnan(ele) || std::isinf(ele) ||
           std::abs(ele) > upper_covariance_cutoff);
}

/// Returns true if all diagonal elements of the covariance matrix
/// are finite aka sane in the above definition.
/// And equal or greater than 0.
template <int N>
inline bool hasPositiveOrZeroDiagElems(const AmgSymMatrix(N) & mat) {
  constexpr int dim = N;
  for (int i = 0; i < dim; ++i) {
    if (mat(i, i) < 0.0 || !saneCovarianceElement(mat(i, i)))
      return false;
  }
  return true;
}

inline bool hasPositiveOrZeroDiagElems(const Amg::MatrixX& mat) {
  int dim = mat.rows();
  for (int i = 0; i < dim; ++i) {
    if (mat(i, i) < 0.0 || !saneCovarianceElement(mat(i, i)))
      return false;
  }
  return true;
}

/// Returns true if all diagonal elements of the covariance matrix
/// are finite aka sane in the above definition.
/// And positive. Instead of just positive we check that we are above
/// the float epsilon
template <int N>
inline bool hasPositiveDiagElems(const AmgSymMatrix(N) & mat) {
  constexpr double MIN_COV_EPSILON = std::numeric_limits<float>::min();
  constexpr int dim = N;
  for (int i = 0; i < dim; ++i) {
    if (mat(i, i) < MIN_COV_EPSILON || !saneCovarianceElement(mat(i, i)))
      return false;
  }
  return true;
}
inline bool hasPositiveDiagElems(const Amg::MatrixX& mat) {
  constexpr double MIN_COV_EPSILON = std::numeric_limits<float>::min();
  int dim = mat.rows();
  for (int i = 0; i < dim; ++i) {
    if (mat(i, i) < MIN_COV_EPSILON || !saneCovarianceElement(mat(i, i)))
      return false;
  }
  return true;
}

/// Check if is positive semidefinit using that fact that  is needed for
/// Cholesky decomposition. We have to use LDLT from Eigen
template <int N>
inline bool isPositiveSemiDefinite(const AmgSymMatrix(N) & mat) {
  if (!hasPositiveOrZeroDiagElems(mat)) {
    // fast check for necessary condition
    return false;
  }
  Eigen::LDLT<AmgSymMatrix(N)> ldltCov(mat);
  return (ldltCov.info() == Eigen::Success && ldltCov.isPositive());
}
inline bool isPositiveSemiDefinite(const Amg::MatrixX& mat) {
  if (!hasPositiveOrZeroDiagElems(mat)) {
    // fast check for necessary condition
    return false;
  }
  Eigen::LDLT<Amg::MatrixX> ldltCov(mat);
  return (ldltCov.info() == Eigen::Success && ldltCov.isPositive());
}

/// Check if is positive semidefinit using that fact that  is needed for
/// Cholesky decomposition. We use LLT from Eigen
template <int N>
inline bool isPositiveDefinite(const AmgSymMatrix(N) & mat) {
  if (!hasPositiveDiagElems(mat)) {
    // fast check for necessary condition
    return false;
  }
  Eigen::LLT<AmgSymMatrix(N)> lltCov(mat);
  return (lltCov.info() == Eigen::Success);
}
inline bool isPositiveDefinite(const Amg::MatrixX& mat) {
  if (!hasPositiveDiagElems(mat)) {
    // fast check for necessary condition
    return false;
  }
  Eigen::LLT<Amg::MatrixX> lltCov(mat);
  return (lltCov.info() == Eigen::Success);
}

/// These are the slow test following the definition.
/// Indented mainly for testing/
template <int N>
inline bool isPositiveSemiDefiniteSlow(const AmgSymMatrix(N) & mat) {
  if (!hasPositiveOrZeroDiagElems(mat)) {
    // fast check for necessary condition
    return false;
  }
  Eigen::SelfAdjointEigenSolver<AmgSymMatrix(5)> eigensolver(mat);
  auto res = eigensolver.eigenvalues();
  for (size_t i = 0; i < N; ++i) {
    if (res[i] < 0) {
      return false;
    }
  }
  return true;
}
inline bool isPositiveSemiDefiniteSlow(const Amg::MatrixX& mat) {
  if (!hasPositiveOrZeroDiagElems(mat)) {
    // fast check for necessary condition
    return false;
  }
  Eigen::SelfAdjointEigenSolver<Amg::MatrixX> eigensolver(mat);
  auto res = eigensolver.eigenvalues();
  int dim = mat.rows();
  for (int i = 0; i < dim; ++i) {
    if (res[i] < 0) {
      return false;
    }
  }
  return true;
}
template <int N>
inline bool isPositiveDefiniteSlow(const AmgSymMatrix(N) & mat) {
  if (!hasPositiveDiagElems(mat)) {
    // fast check for necessary condition
    return false;
  }
  Eigen::SelfAdjointEigenSolver<AmgSymMatrix(5)> eigensolver(mat);
  auto res = eigensolver.eigenvalues();
  for (size_t i = 0; i < N; ++i) {
    if (res[i] <= 0) {
      return false;
    }
  }
  return true;
}
inline bool isPositiveDefiniteSlow(const Amg::MatrixX& mat) {
  if (!hasPositiveDiagElems(mat)) {
    // fast check for necessary condition
    return false;
  }
  Eigen::SelfAdjointEigenSolver<Amg::MatrixX> eigensolver(mat);
  auto res = eigensolver.eigenvalues();
  int dim = mat.rows();
  for (int i = 0; i < dim; ++i) {
    if (res[i] <= 0) {
      return false;
    }
  }
  return true;
}
// Chi squared statistic of a linear regression as defined in Frühwirth, 
// section 3.2.1, equation 3.19. Precision is defined as the inverse of 
// of the covariance.
template <typename T, typename U>
inline double chi2(const T& precision, const U& residual, const int sign = 1) {
  return sign * residual.transpose() * precision * residual;
}

}  // namespace Amg

#endif
