/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file FTagEigenVariations/FTagEigenRecompositionTool.h
 * @brief Recomposition of the eigenvector nuisance parameters of a CDI file into the original uncertainties.
 */

#ifndef FTAGEIGENVARIATIONS_FTAGEIGENRECOMPOSITIONTOOL_H
#define FTAGEIGENVARIATIONS_FTAGEIGENRECOMPOSITIONTOOL_H

#include "FTagEigenVariations/IFTagEigenRecompositionTool.h"

#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"
#include "FTagAnalysisInterfaces/IBTaggingEigenVectorRecompositionTool.h"

#include <map>
#include <string>
#include <vector>

namespace ftag::ev {

  /**
   * @brief Reads a CDI file and computes the recomposition of its eigenvector decompositions in initialize().
   *
   * The properties carry the names of the BTaggingEfficiencyTool ones, and the tool also provides the interface of
   * the BTaggingEigenVectorRecompositionTool. Not recommended to analyses until the BTaggingEfficiencyTool uses the
   * same decomposition.
   */
  class FTagEigenRecompositionTool : public asg::AsgTool,
                                     virtual public IFTagEigenRecompositionTool,
                                     virtual public IBTaggingEigenVectorRecompositionTool {
    ASG_TOOL_CLASS3( FTagEigenRecompositionTool, ftag::ev::IFTagEigenRecompositionTool,
                     IBTaggingEigenVectorRecompositionTool, CP::ISystematicsTool )

  public:
    using asg::AsgTool::AsgTool;

    StatusCode initialize() override;

    const RecompositionTable& recomposition(const std::string& label) const override;

    // IBTaggingEigenVectorRecompositionTool: keys "Eigen_<label>_<k>" ("Eigen_Global_<k>" with SFGlobalEigen),
    // source names as in the CDI file, and std::invalid_argument for an unknown label or eigenvector index
    std::vector<std::string> getListOfOriginalNuisanceParameters(const std::string& label) const override;
    std::map<std::string, std::map<std::string, float>>
    getCoefficientMap(const std::string& label, const std::vector<unsigned int>& eigenIdxList) const override;
    std::vector<float> getCoefficients(const std::string& label, const unsigned int evIdx) const override;
    int getNumEigenVectors(const std::string& label) const override;

    // The tool applies no systematic variations
    CP::SystematicSet affectingSystematics() const override { return {}; }
    CP::SystematicSet recommendedSystematics() const override { return {}; }
    bool isAffectedBySystematic(const CP::SystematicVariation&) const override { return false; }
    StatusCode applySystematicVariation(const CP::SystematicSet&) override { return StatusCode::SUCCESS; }

  private:
    Gaudi::Property<std::string> m_cdiFile{this, "ScaleFactorFileName", "", "CDI file (uses PathResolver)"};
    Gaudi::Property<std::string> m_tagger{this, "TaggerName", "", "tagger name as in the CDI file"};
    Gaudi::Property<std::string> m_operatingPoint{this, "OperatingPoint", "", "operating point as in the CDI file"};
    Gaudi::Property<std::string> m_jetAuthor{this, "JetAuthor", "", "jet collection as in the CDI file"};
    Gaudi::Property<std::string> m_strategy{this, "SystematicsStrategy", "SFEigen", "'SFEigen' or 'SFGlobalEigen'"};

    Gaudi::Property<std::string> m_calibrationB{this, "ScaleFactorBCalibration", "default", "b-jet calibration name"};
    Gaudi::Property<std::string> m_calibrationC{this, "ScaleFactorCCalibration", "default", "c-jet calibration name"};
    Gaudi::Property<std::string> m_calibrationT{this, "ScaleFactorTCalibration", "default", "tau-jet calibration name"};
    Gaudi::Property<std::string> m_calibrationLight{this, "ScaleFactorLightCalibration", "default",
                                                    "light-jet calibration name"};

    Gaudi::Property<std::string> m_reductionB{this, "EigenvectorReductionB", "Loose", "'Loose', 'Medium' or 'Tight'"};
    Gaudi::Property<std::string> m_reductionC{this, "EigenvectorReductionC", "Loose", "'Loose', 'Medium' or 'Tight'"};
    Gaudi::Property<std::string> m_reductionLight{this, "EigenvectorReductionLight", "Loose",
                                                  "'Loose', 'Medium' or 'Tight'"};

    Gaudi::Property<std::string> m_exclude{this, "ExcludeFromEigenVectorTreatment", "",
                                           "(semicolon-separated) uncertainties excluded for all flavours"};
    Gaudi::Property<std::string> m_excludeB{this, "ExcludeFromEigenVectorBTreatment", "",
                                            "(semicolon-separated) uncertainties excluded for b-jets"};
    Gaudi::Property<std::string> m_excludeC{this, "ExcludeFromEigenVectorCTreatment", "",
                                            "(semicolon-separated) uncertainties excluded for c- and tau-jets"};
    Gaudi::Property<std::string> m_excludeLight{this, "ExcludeFromEigenVectorLightTreatment", "",
                                                "(semicolon-separated) uncertainties excluded for light-jets"};
    Gaudi::Property<bool> m_excludeRecommended{this, "ExcludeRecommendedFromEigenVectorTreatment", false,
                                               "also exclude the uncertainties the CDI file recommends to exclude"};

    Gaudi::Property<std::string> m_suffixB{this, "UncertaintyBSuffix", "", "suffix of the b-jet eigenvector names"};
    Gaudi::Property<std::string> m_suffixC{this, "UncertaintyCSuffix", "", "suffix of the c-jet eigenvector names"};
    Gaudi::Property<std::string> m_suffixT{this, "UncertaintyTSuffix", "", "suffix of the tau-jet eigenvector names"};
    Gaudi::Property<std::string> m_suffixLight{this, "UncertaintyLightSuffix", "",
                                               "suffix of the light-jet eigenvector names"};
    Gaudi::Property<bool> m_splitTauCharm{this, "splitTauCharmEff", false,
                                          "decompose tau-jets separately instead of sharing the c-jet decomposition"};

    /// Recomposition of one label, with the source names as in the CDI file
    struct Entry {
      RecompositionTable table;
      std::vector<std::string> cdiSources;
    };

    const Entry& entry(const std::string& label) const;

    /// One entry per label, or one "Global" entry with SFGlobalEigen; tau jets map to the c-jet entry unless split
    std::map<std::string, Entry> m_entries;
  };

}

#endif
