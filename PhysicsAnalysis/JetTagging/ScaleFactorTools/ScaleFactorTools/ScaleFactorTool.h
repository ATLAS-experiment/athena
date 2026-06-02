/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef CP_SCALEFACTORTOOL_H
#define CP_SCALEFACTORTOOL_H

#include "FTagAnalysisInterfaces/IScaleFactorTool.h"
#include "AsgTools/AsgTool.h"
#include <AsgTools/PropertyWrapper.h>
#include <nlohmann/json.hpp>
#include "ScaleFactorTools/ToolUtils.h"
using json = nlohmann::json;

class ScaleFactorTool: public asg::AsgTool,
                       public virtual IScaleFactorTool {
  ASG_TOOL_CLASS( ScaleFactorTool, IScaleFactorTool)

  public:
  ScaleFactorTool(const json& cfg);
  float getSF(const SG::AuxElement& el) const;

  private:
  bool m_initialised = false;
  Gaudi::Property<std::string> m_taggerName {this, "TaggerName", "", "Name of the tagger"};
  Gaudi::Property<std::string> m_json_config_path {this, "JsonConfigFile", "", "Path to JSON config file"};
  Gaudi::Property<std::string> m_obj_container {this, "ObjContainer", "", "object container"};

  ToolUtils::QuantileFunc m_sf_func;
  ToolUtils::QuantileFunc m_pcbt_func;

  json m_json_config;

  int m_n_pcbt_bins;
  int inferPCBTBins(const json& cfg);

  std::vector<float> m_sf_values;
};
