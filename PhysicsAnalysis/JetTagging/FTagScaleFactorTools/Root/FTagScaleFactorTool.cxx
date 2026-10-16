/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "FTagScaleFactorTools/FTagScaleFactorTool.h"
#include "QuantileFactory.h"
#include "PathResolver/PathResolver.h"
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;


StatusCode ftag::FTagScaleFactorTool::initialize(){
  std::string pathToJsonConfigFile = PathResolverFindCalibFile(m_json_config_path);

  std::ifstream jsonFile(pathToJsonConfigFile);
  if (!jsonFile.is_open()) {
    ATH_MSG_ERROR("JSON file " + m_json_config_path + " does not exist. Please put the correct path of the file.");
    return StatusCode::FAILURE;
  }

  json json_config = json::parse(jsonFile);
  jsonFile.close();

  if (m_taggerName.empty() || json_config.at("TaggerName").get<std::string>() != m_taggerName.value()) {
    ATH_MSG_ERROR("Tagger name " + m_taggerName + " not found in JSON file:" + m_json_config_path);
    return StatusCode::FAILURE;
  }

  if (m_obj_container.empty() || json_config.at("Container").get<std::string>() != m_obj_container.value()) {
    ATH_MSG_ERROR("Missing container config for " + m_obj_container + "in tagger" + m_taggerName);
    return StatusCode::FAILURE;
  }

  if (m_pct_Name.empty() || json_config.at("Scheme").get<std::string>() != m_pct_Name.value()) {
    ATH_MSG_ERROR(" PCT WP " + m_pct_Name + "not avaialble for " + m_taggerName);
    return StatusCode::FAILURE;
  }

  // =====================
  // pre-load json file
  // =====================
  m_sf_values = json_config.at("scale_factors").get<std::vector<float>>();
  m_invalid_sf_value = json_config.at("invalid_sf_value").get<float>();

  std::tie(m_n_sf_bins, m_sf_func) = QuantileFactory::quantileFactory(json_config.at("sf_bins"));
  std::tie(m_n_pct_bins, m_pct_func) = QuantileFactory::quantileFactory(json_config.at("pct_bins"));

  for (const auto& [name, wp_cfg] : json_config.at("working_points").items()) {
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

  for (const auto& [name, values] : json_config.at("systematics").items()) {
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

CP::CorrectionCode ftag::FTagScaleFactorTool::getScaleFactor(const xAOD::IParticle& p, float& sf, const CP::SystematicSet& sys) const
{

  if (! m_initialised) {
    ATH_MSG_ERROR( "Tool is not initialized — call initialize() first" );
    return CP::CorrectionCode::Error;
  }

  const SG::AuxElement& el = p;
  int sf_bin = m_sf_func(el);

  if (sf_bin < 0) {
    sf = m_invalid_sf_value;
    return CP::CorrectionCode::OutOfValidityRange;
  }

  int pct_bin = m_pct_func(el);
  int global = sf_bin * m_n_pct_bins + pct_bin;

  if (sys.empty()) {
    sf =  m_sf_values.at(global);
    return CP::CorrectionCode::Ok;
  }

  auto it = m_sf_systematics.find(sys);
  if (it == m_sf_systematics.end()) {
    sf = m_sf_values.at(global);
    return CP::CorrectionCode::Ok;
  }

  sf = it->second.at(global);
  return CP::CorrectionCode::Ok;

}

std::unordered_map<std::string, int> ftag::FTagScaleFactorTool::inferWPs(const xAOD::IParticle* p) const
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

CP::SystematicSet ftag::FTagScaleFactorTool::affectingSystematics() const
{
  CP::SystematicSet result;

  for (const auto& [set, _] : m_sf_systematics) {
    if (set.size() != 1) continue;
    result.insert(*set.begin());
  }

  return result;

}

CP::SystematicSet ftag::FTagScaleFactorTool::recommendedSystematics() const
{
  return affectingSystematics();
}

