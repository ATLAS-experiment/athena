/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FTagEigenVariations/EigenReduction.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

Eigen::MatrixXd ftag::ev::reduce(const Decomposition& decomposition, Eigen::Index nKeep) {
  if (nKeep < 1) throw std::invalid_argument("ftag::ev::reduce: at least one variation must be kept");
  const Eigen::MatrixXd& all = decomposition.variations;
  if (nKeep >= all.cols()) return all;

  const Eigen::Index nKept = nKeep - 1;
  Eigen::MatrixXd reduced(all.rows(), nKeep);
  reduced.leftCols(nKept) = all.leftCols(nKept);
  const auto tail = all.rightCols(all.cols() - nKept);
  for (Eigen::Index u = 0; u < all.rows(); ++u) {
    const double sigma = tail.row(u).norm();
    reduced(u, nKept) = tail(u, 0) < 0.0 ? -sigma : sigma;
  }
  return reduced;
}

ftag::ev::ReductionQuality ftag::ev::reductionQuality(const Eigen::MatrixXd& covariance,
                                                      const Eigen::MatrixXd& variations) {
  if (covariance.rows() != variations.rows()) {
    throw std::invalid_argument("ftag::ev::reductionQuality: covariance and variations have different numbers of bins");
  }
  const Eigen::MatrixXd reduced = variations * variations.transpose();
  const Eigen::VectorXd full = covariance.diagonal();
  const Eigen::VectorXd kept = reduced.diagonal();

  ReductionQuality quality;
  quality.sigmaRatio = Eigen::VectorXd::Ones(full.size());
  for (Eigen::Index u = 0; u < full.size(); ++u) {
    if (full(u) > 0.0) quality.sigmaRatio(u) = std::sqrt(kept(u) / full(u));
  }
  for (Eigen::Index u = 0; u < full.size(); ++u) {
    for (Eigen::Index v = 0; v < u; ++v) {
      if (full(u) <= 0.0 || full(v) <= 0.0 || kept(u) <= 0.0 || kept(v) <= 0.0) continue;
      const double change = reduced(u, v) / std::sqrt(kept(u) * kept(v))
                          - covariance(u, v) / std::sqrt(full(u) * full(v));
      quality.maxCorrelationChange = std::max(quality.maxCorrelationChange, std::abs(change));
    }
  }
  return quality;
}
