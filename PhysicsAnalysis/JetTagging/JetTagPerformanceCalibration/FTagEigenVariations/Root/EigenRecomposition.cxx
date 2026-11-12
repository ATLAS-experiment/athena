/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FTagEigenVariations/EigenRecomposition.h"

#include <stdexcept>

ftag::ev::Recomposition ftag::ev::recompose(const Decomposition& decomposition, const Eigen::MatrixXd& variations) {
  if (decomposition.shifts.rows() != variations.rows()) {
    throw std::invalid_argument("ftag::ev::recompose: shifts and variations have different numbers of bins");
  }
  const Eigen::MatrixXd gram = variations.transpose() * variations;
  const Eigen::LDLT<Eigen::MatrixXd> solver(gram);
  if (solver.info() != Eigen::Success) throw std::runtime_error("ftag::ev::recompose: solve failed");

  Recomposition result;
  result.coefficients = solver.solve(variations.transpose() * decomposition.shifts).transpose();
  result.describedFraction = result.coefficients.colwise().squaredNorm().transpose();
  return result;
}
