/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file FTagEigenVariations/SystematicNames.h
 * @brief Systematic names of eigenvariations and sources, as the BTaggingEfficiencyTool gives them.
 */

#ifndef FTAGEIGENVARIATIONS_SYSTEMATICNAMES_H
#define FTAGEIGENVARIATIONS_SYSTEMATICNAMES_H

#include <Eigen/Dense>

#include <string>
#include <string_view>

namespace ftag::ev {

  /** "FT_EFF_Eigen_<label>_<k>" for eigenvariation k of a flavour label. */
  std::string eigenName(std::string_view label, Eigen::Index k);

  /** Source name with spaces replaced by "_", prefixed with "FT_EFF_" unless it already is. */
  std::string sourceName(std::string_view source);

}

#endif
