/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef CP_SCALEFACTORTOOL_H
#define CP_SCALEFACTORTOOL_H

#include "FTagAnalysisInterfaces/IScaleFactorTool.h"
#include "AsgTools/AsgTool.h"
#include <AsgTools/PropertyWrapper.h>
#include "xAODBase/IParticle.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class ScaleFactorTool: public asg::AsgTool,
                       public virtual IScaleFactorTool {
  ASG_TOOL_CLASS( ScaleFactorTool, IScaleFactorTool)

  public:
  ScaleFactorTool(const std::string& name);
  StatusCode initialize() override;

  std::map<CP::SystematicSet, float> getSF(const xAOD::IParticle* p) const override;
  std::unordered_map<std::string, int> inferWPs(const xAOD::IParticle* p) const override;

  // systeamtic stuff
  virtual CP::SystematicSet affectingSystematics() const override;
  virtual CP::SystematicSet recommendedSystematics() const override;

  private:
  bool m_initialised = false;
  Gaudi::Property<std::string> m_taggerName {this, "TaggerName", "", "Name of the tagger"};
  Gaudi::Property<std::string> m_json_config_path {this, "JsonConfigFile", "", "Path to JSON config file"};
  Gaudi::Property<std::string> m_obj_container {this, "ObjContainer", "", "object container"};
  Gaudi::Property<std::string> m_pct_Name {this, "PCTName", "", "pseudo-continuous tagger name"};

  size_t m_n_sf_bins;
  size_t m_n_pct_bins;
  std::function<int(const SG::AuxElement&)> m_sf_func;
  std::function<int(const SG::AuxElement&)> m_pct_func;

  json m_json_config;

  std::unordered_map<std::string, std::unordered_set<int>> m_wp_bins;
  std::map<CP::SystematicSet, std::vector<float>> m_sf_systematics;

  std::vector<float> m_sf_values;
};
#endif // CP_SCALEFACTORTOOL_H
