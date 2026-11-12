/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file FTagEigenVariations/EigenReduction.h
 * @brief Reduction of the eigenvariations to a smaller number, and its quality.
 */

#ifndef FTAGEIGENVARIATIONS_EIGENREDUCTION_H
#define FTAGEIGENVARIATIONS_EIGENREDUCTION_H

#include "FTagEigenVariations/EigenDecomposition.h"

#include <Eigen/Dense>

namespace ftag::ev {

  /** How well a set of variations reproduces the full covariance. */
  struct ReductionQuality {
    /// Per joint bin, uncertainty from the variations over the full one; 1 where both are zero.
    Eigen::VectorXd sigmaRatio;
    /// Largest absolute change of a correlation coefficient, over bins with uncertainty in both.
    double maxCorrelationChange = 0.0;
  };

  /**
   * @brief Keep the leading nKeep - 1 eigenvariations and merge the others into one.
   *
   * The merged variation has the full remaining variance in every bin, with the sign of the leading
   * merged eigenvariation. All variations are kept unchanged if nKeep is at least their number.
   * @throws std::invalid_argument if nKeep is below 1.
   */
  Eigen::MatrixXd reduce(const Decomposition& decomposition, Eigen::Index nKeep);

  /**
   * @brief Compare the covariance of a set of variations, one per column, with the full one.
   * @throws std::invalid_argument if the numbers of bins differ.
   */
  ReductionQuality reductionQuality(const Eigen::MatrixXd& covariance, const Eigen::MatrixXd& variations);

}

#endif
