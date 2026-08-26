/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "ScaleFactorTools/ScaleFactorTool.h"
#include "QuantileFactory.h"
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

  if (m_taggerName.empty() || m_json_config.at("TaggerName").get<std::string>() != m_taggerName.value()) {
    ATH_MSG_ERROR("Tagger name " + m_taggerName + " not found in JSON file:" + m_json_config_path);
    return StatusCode::FAILURE;
  }

  if (m_obj_container.empty() || m_json_config.at("Container").get<std::string>() != m_obj_container.value()) {
    ATH_MSG_ERROR("Missing container config for " + m_obj_container + "in tagger" + m_taggerName);
    return StatusCode::FAILURE;
  }

  if (m_pct_Name.empty() || m_json_config.at("Scheme").get<std::string>() != m_pct_Name.value()) {
    ATH_MSG_ERROR(" PCT WP " + m_pct_Name + "not avaialble for " + m_taggerName);
    return StatusCode::FAILURE;
  }

  // =====================
  // pre-load json file
  // =====================
  m_sf_values = m_json_config.at("scale_factors").get<std::vector<float>>();
  m_invalid_sf_value = m_json_config.at("invalid_sf_value").get<int>();

  std::tie(m_n_sf_bins, m_sf_func) = QuantileFactory::quantileFactory(m_json_config.at("sf_bins"));
  std::tie(m_n_pct_bins, m_pct_func) = QuantileFactory::quantileFactory(m_json_config.at("pct_bins"));

  for (const auto& [name, wp_cfg] : m_json_config.at("working_points").items()) {
    std::unordered_set<int> bins;
    for (int b : wp_cfg.at("bins")) {
      bins.insert(b);
    }
    m_wp_bins[name] = std::move(bins);
  }

  if (m_sf_values.size() != m_n_sf_bins * m_n_pct_bins) {
    ATH_MSG_ERROR( "Nominal SF size mismatch: expected " << m_n_sf_bins * m_n_pct_bins << " bins but found " << m_sf_values.size());
    return StatusCode::FAILURE;
  }

  for (const auto& [name, values] : m_json_config.at("systematics").items()) {
    CP::SystematicSet set;
    set.insert(CP::SystematicVariation(name));

    std::vector<float> vec = values.get<std::vector<float>>();
    if (vec.size() != m_sf_values.size()) {
      ATH_MSG_ERROR("Systematic size mismatch with nominal SFs");
      return StatusCode::FAILURE;
    }
    m_sf_systematics[set] = std::move(vec);
  }

  for (const auto& [set, _] : m_sf_systematics) {
    if (set.size() != 1) {
      ATH_MSG_ERROR("Failed to initialize systematic cache");
      return StatusCode::FAILURE;
    }
  }

  m_initialised = true;

  return StatusCode::SUCCESS;
}

std::map<CP::SystematicSet, float> ScaleFactorTool::getSF(const xAOD::IParticle* p) const
{
  std::map<CP::SystematicSet, float> result;

  const SG::AuxElement& el = *p;
  int sf_bin = m_sf_func(el);

  if (sf_bin < 0) {
    result[CP::SystematicSet()] = m_invalid_sf_value;
    for (const auto& [set, _] : m_sf_systematics) {
      result[set] = m_invalid_sf_value;
    }
    return result;
  }

  int pct_bin = m_pct_func(el);
  int global = sf_bin * m_n_pct_bins + pct_bin;

  result[CP::SystematicSet()] = m_sf_values.at(global);

  for (const auto& [set, vec] : m_sf_systematics) {
    result[set] = vec.at(global);
  }
  return result;
}

std::unordered_map<std::string, int> ScaleFactorTool::inferWPs(const xAOD::IParticle* p) const
{
  const SG::AuxElement& el = *p;
  int pct_bin = m_pct_func(el);

  std::unordered_map<std::string, int> result;

  for (const auto& [name, bins] : m_wp_bins) {
    if (bins.find(pct_bin) != bins.end()) {
      result[name] = 1;
    } else {
      result[name] = 0;
    }
  }

  result[m_pct_Name] = pct_bin;

  return result;

}


CP::SystematicSet ScaleFactorTool::affectingSystematics() const
{
  CP::SystematicSet result;

  for (const auto& [set, _] : m_sf_systematics) {
    if (set.size() != 1) continue;
    result.insert(*set.begin());
  }

  return result;

}

CP::SystematicSet ScaleFactorTool::recommendedSystematics() const
{
  return affectingSystematics();
}

