/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file FTagEigenVariations/CDIInput.h
 * @brief Inputs of the eigenvector decomposition from CDI scale factor containers.
 */

#ifndef FTAGEIGENVARIATIONS_CDIINPUT_H
#define FTAGEIGENVARIATIONS_CDIINPUT_H

#include "FTagEigenVariations/EigenDecomposition.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace Analysis {
  class CalibrationDataHistogramContainer;
}

namespace ftag::ev::cdi {

  /** Eigenvector reduction scheme; the values are the indices into the "ReducedSets" of a container. */
  enum class Reduction { Tight = 0, Medium = 1, Loose = 2 };

  /** "Loose", "Medium" or "Tight". @throws std::invalid_argument otherwise. */
  Reduction parseReduction(const std::string& scheme);

  /** One CDI scale factor container, flattened over its bins without under- and overflow. */
  struct Container {
    /// Block with the stat covariance; the flavour label is the one passed to readContainer.
    FlavourBlock block;
    /// Uncertainty sources, each with its shifts for this single block.
    std::vector<Source> sources;
    /// Sources the calibration recommends to exclude ("excluded_set").
    std::vector<std::string> recommendedExclusions;
    /// Number of eigenvariations to keep, indexed by Reduction, as far as stored ("ReducedSets").
    std::vector<int> reducedSets;
    /// Histogram bin number of each bin of the block.
    std::vector<int> histogramBins;
  };

  /**
   * @brief Read a scale factor container the way the current eigenvector implementation does.
   *
   * The container is not const since its tag weight axis is only computed on first access.
   * @throws std::invalid_argument for a container without result histogram or with inconsistent histograms.
   */
  Container readContainer(Analysis::CalibrationDataHistogramContainer& container, const std::string& label);

  /** Exclusion and reduction settings, per flavour label as passed to readContainer. */
  struct Settings {
    /// Sources to exclude, per flavour label. A trailing "*" matches by prefix.
    std::map<std::string, std::vector<std::string>> exclusions;
    /// Also exclude the sources the calibration recommends to exclude.
    bool excludeRecommended = false;
    /// Reduction per flavour label; Loose if not given.
    std::map<std::string, Reduction> reductions;
  };

  /**
   * @brief Input of the per-flavour decomposition of one container.
   *
   * Exclusions that match no source of the container are ignored.
   */
  DecompositionInput perFlavourInput(const Container& container, const Settings& settings);

  /**
   * @brief Input of the global decomposition of the containers of all flavours.
   *
   * Sources with the same name in several containers become one source; names that differ only by a leading
   * "FT_EFF_" count as the same, and the source takes the prefixed name. A source that is uncorrelated
   * between bins in any container is also uncorrelated between flavours. Exclusions of all flavours apply.
   */
  DecompositionInput globalInput(const std::vector<Container>& containers, const Settings& settings);

  /**
   * @brief Number of eigenvariations to keep for the per-flavour decomposition, or none to keep all.
   *
   * Loose keeps all unless the container stores a number for it.
   * @throws std::invalid_argument if the container stores no number for the requested reduction.
   */
  std::optional<Eigen::Index> eigenvariationsToKeep(const Container& container, const Settings& settings);

}

#endif
