/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "ScaleFactorTools/ScaleFactorTool.h"
#include "PathResolver/PathResolver.h"
#include <fstream>

ScaleFactorTool::ScaleFactorTool(const std::string& name)
  : asg::AsgTool(name)
{
  m_initialised = false;
}

StatusCode ScaleFactorTool::initialize(){
  std::string pathToJsonConfigFile = PathResolverFindCalibFile(m_json_config_path);

  std::ifstream jsonFile(pathToJsonConfigFile);
  if (!jsonFile.is_open()) {
    ATH_MSG_ERROR("JSON file " + m_json_config_path + " does not exist. Please put the correct path of the file.");
    return StatusCode::FAILURE;
  }

  m_json_config = json::parse(jsonFile);
  jsonFile.close();

  if (m_taggerName.empty()) {
    ATH_MSG_ERROR("Tagger name not set");
    return StatusCode::FAILURE;
  }

  if (m_obj_container.empty()) {
    ATH_MSG_ERROR("Missing container config for " + m_obj_container + "in tagger" + m_taggerName);
    return StatusCode::FAILURE;
  }

  // =====================
  // pre-load json file
  // =====================
  m_sf_values = m_json_config.at("scale_factors").get<std::vector<float>>();

  m_sf_func   = ToolUtils::quantileFactory(m_json_config.at("sf_bins"));
  m_pcbt_func = ToolUtils::quantileFactory(m_json_config.at("pcbt_bins"));
  m_n_pcbt_bins = inferPCBTBins(m_json_config.at("pcbt_bins"));

  m_initialised = true;

  return StatusCode::SUCCESS;
}

float ScaleFactorTool::getSF(const xAOD::IParticle* p) const
{
  const SG::AuxElement& el = *p;
  int sf_bin = m_sf_func(el);
  std::cout << "sf_bin = " << sf_bin << std::endl;
  int pcbt_bin = m_pcbt_func(el);
  std::cout << " pcbt_bins total = " << m_n_pcbt_bins << std::endl;
  std::cout << " pcbt_bin = " << pcbt_bin << std::endl;
  int global = sf_bin * m_n_pcbt_bins + pcbt_bin;
  std::cout << " global = " << global << std::endl;
  return m_sf_values.at(global);
}

// ==========================
// Infer PCT bins
// ==========================
int ScaleFactorTool::inferPCBTBins(const json& cfg)
{
  if (cfg.at("type") == "enumerate"){
    return cfg.at("edges").size() - 1;
  }
  if (cfg.at("type") == "nodes"){
    int total = 0;
    for (const auto& node : cfg.at("nodes")) {
      total += node.at("edges").size() + 1;
    }
    return total;
  }
  throw std::runtime_error("Cannot infer pcbt bins");
}
