/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file FTagEigenVariations/EigenRecomposition.h
 * @brief Recomposition of eigenvariations in terms of the original uncertainty sources.
 */

#ifndef FTAGEIGENVARIATIONS_EIGENRECOMPOSITION_H
#define FTAGEIGENVARIATIONS_EIGENRECOMPOSITION_H

#include "FTagEigenVariations/EigenDecomposition.h"

#include <Eigen/Dense>

namespace ftag::ev {

  /** Each variation as a combination of the sources of a decomposition. */
  struct Recomposition {
    /// One row per entry of Decomposition::sourceNames, one column per variation.
    Eigen::MatrixXd coefficients;
    /// Per variation, the sum of its squared coefficients: the fraction of it the sources describe.
    Eigen::VectorXd describedFraction;
  };

  /**
   * @brief Least-squares coefficients c with shifts = variations * c^T, for all variations together.
   *
   * Exact when the variations are the unreduced eigenvariations and there is no stat covariance.
   * @throws std::invalid_argument if the numbers of bins differ.
   */
  Recomposition recompose(const Decomposition& decomposition, const Eigen::MatrixXd& variations);

}

#endif
