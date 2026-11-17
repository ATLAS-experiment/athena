/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file FTagEigenVariations/IFTagEigenRecompositionTool.h
 * @brief Interface of the tool that expresses eigenvector nuisance parameters in the original uncertainties.
 */

#ifndef FTAGEIGENVARIATIONS_IFTAGEIGENRECOMPOSITIONTOOL_H
#define FTAGEIGENVARIATIONS_IFTAGEIGENRECOMPOSITIONTOOL_H

#include "AsgTools/IAsgTool.h"

#include <string>
#include <vector>

namespace ftag::ev {

  /** Recomposition of the eigenvector nuisance parameters of one decomposition. */
  struct RecompositionTable {
    /// Eigenvector nuisance parameters, named like the BTaggingEfficiencyTool systematics
    std::vector<std::string> nuisanceParameters;
    /// Original uncertainties, named like the BTaggingEfficiencyTool named systematics
    std::vector<std::string> sources;
    /// coefficients[k][s]: coefficient of source s in nuisance parameter k
    std::vector<std::vector<double>> coefficients;
    /// Part of each nuisance parameter the sources describe, the sum of its squared coefficients
    std::vector<double> describedFraction;
  };

  class IFTagEigenRecompositionTool : virtual public asg::IAsgTool {
    ASG_TOOL_INTERFACE( ftag::ev::IFTagEigenRecompositionTool )

  public:
    /**
     * @brief Recomposition for a flavour label (B, C, T, Light), named like the systematics of that flavour.
     *
     * With SFGlobalEigen, every label gives the same table of the global eigenvariations, FT_EFF_Eigen_Global_<k>.
     * @throws std::invalid_argument for any other label.
     */
    virtual const RecompositionTable& recomposition(const std::string& label) const = 0;
  };

}

#endif
