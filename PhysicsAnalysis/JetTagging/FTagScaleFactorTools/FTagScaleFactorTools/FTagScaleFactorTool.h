/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef FTAGSCALEFACTORTOOLS_FTAGSCALEFACTORTOOL_H
#define FTAGSCALEFACTORTOOLS_FTAGSCALEFACTORTOOL_H

#include "FTagAnalysisInterfaces/IFTagScaleFactorTool.h"
#include "AsgTools/AsgTool.h"
#include <AsgTools/PropertyWrapper.h>
#include "xAODBase/IParticle.h"
#include "PATInterfaces/CorrectionCode.h"
#include <vector>

namespace ftag{
class FTagScaleFactorTool: public asg::AsgTool,
                       public virtual ftag::IFTagScaleFactorTool {
  ASG_TOOL_CLASS( FTagScaleFactorTool, ftag::IFTagScaleFactorTool)

  public:
  using asg::AsgTool::AsgTool;
  StatusCode initialize() override;

  CP::CorrectionCode getScaleFactor(const xAOD::IParticle& p, float& sf, const CP::SystematicSet& sys) const override;
  std::vector<int> inferWPs(const xAOD::IParticle* p) const override;

  // systeamtic stuff
  virtual CP::SystematicSet affectingSystematics() const override;
  virtual CP::SystematicSet recommendedSystematics() const override;

  private:
  bool m_initialised = false;
  Gaudi::Property<std::string> m_taggerName {this, "TaggerName", "", "Name of the tagger"};
  Gaudi::Property<std::string> m_json_config_path {this, "JsonConfigFile", "", "Path to JSON config file"};
  Gaudi::Property<std::string> m_obj_container {this, "ObjContainer", "", "object container"};
  Gaudi::Property<std::string> m_pct_Name {this, "PCTName", "", "pseudo-continuous tagger name"};
  Gaudi::Property<float>       m_invalid_sf_value {this, "InvalidSFValue", -1.f, "Value returned for invalid SF"};
  Gaudi::Property<std::vector<std::string>> m_selected_wps {this, "WorkingPoints", {}, "List of working-point names to evaluate. Optional- leave empty if inferWPs() is not called."};

  size_t m_n_sf_bins = 0;
  size_t m_n_pct_bins = 0;

  std::function<int(const SG::AuxElement&)> m_sf_func{};
  std::function<int(const SG::AuxElement&)> m_pct_func{};

  std::vector<std::unordered_set<int>> m_ordered_wp_bins{};
  std::unordered_map<CP::SystematicSet, std::vector<float>> m_sf_systematics{};

  std::vector<float> m_sf_values{};
};
}
#endif // FTAGSCALEFACTORTOOLS_SCALEFACTORTOOL_H
