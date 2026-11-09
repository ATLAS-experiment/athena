/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file FTagEigenVariations/EigenDecomposition.h
 * @brief Eigenvector decomposition of the flavour tagging scale factor uncertainties.
 */

#ifndef FTAGEIGENVARIATIONS_EIGENDECOMPOSITION_H
#define FTAGEIGENVARIATIONS_EIGENDECOMPOSITION_H

#include <Eigen/Dense>

#include <cstddef>
#include <set>
#include <string>
#include <vector>

namespace ftag::ev {

  /// Eigenvalues at or below this fraction of the largest one are treated as zero.
  inline constexpr double relativeEigenvalueThreshold = 1e-12;

  /** Scale factor bins of one flavour, flattened to one index. */
  struct FlavourBlock {
    /// Flavour label, e.g. "B".
    std::string label;
    /// Symmetric statistical covariance of the scale factors, one row per bin.
    Eigen::MatrixXd statCovariance;
  };

  /** One uncertainty source, given as the shift of the scale factor in each bin. */
  struct Source {
    /// Source name as stored in the calibration.
    std::string name;
    /// One entry per flavour block; an empty vector means the source does not affect that block.
    std::vector<Eigen::VectorXd> shifts;
    /// Empty for a source that is correlated across all bins. Otherwise one group id per bin and block:
    /// bins with the same id, in any block, are correlated, bins with different ids are not.
    std::vector<std::vector<std::size_t>> binGroups;
  };

  /** Everything a decomposition is computed from. */
  struct DecompositionInput {
    /// Flavour blocks, one for the per-flavour strategy and several for the global one.
    std::vector<FlavourBlock> blocks;
    /// Uncertainty sources.
    std::vector<Source> sources;
    /// Names of sources kept out of the decomposition.
    std::set<std::string> excluded;
  };

  /** Eigenvariations over the joint bins of all blocks, blocks concatenated in input order. */
  struct Decomposition {
    /// Index of the first bin of each block in the joint bins.
    std::vector<Eigen::Index> blockOffsets;
    /// Sources entering the covariance; a grouped source gives one entry per group, "<source>_bin<id>".
    std::vector<std::string> sourceNames;
    /// Shift per joint bin, one column per entry of sourceNames.
    Eigen::MatrixXd shifts;
    /// Total covariance, stat plus shifts * shifts^T.
    Eigen::MatrixXd covariance;
    /// Eigenvalues above the threshold, descending.
    Eigen::VectorXd eigenvalues;
    /// Columns sqrt(lambda_k) * u_k, ordered like eigenvalues, with the largest |entry| of u_k positive.
    Eigen::MatrixXd variations;
  };

  /**
   * @brief Decompose the total covariance of all blocks into independent eigenvariations.
   * @throws std::invalid_argument on inconsistent input.
   */
  Decomposition decompose(const DecompositionInput& input);

}

#endif
