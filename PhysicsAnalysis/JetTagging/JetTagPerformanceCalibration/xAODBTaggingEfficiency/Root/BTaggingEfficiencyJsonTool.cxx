/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODBTaggingEfficiency/BTaggingToolUtil.h"
#include "xAODBTaggingEfficiency/BTaggingEfficiencyJsonTool.h"
#include <fstream>

BTaggingEfficiencyJsonTool::BTaggingEfficiencyJsonTool ( const std::string &name ) :
  asg::AsgTool ( name )
{
  m_initialised = false;
}

BTaggingEfficiencyJsonTool::~BTaggingEfficiencyJsonTool() {
}

StatusCode BTaggingEfficiencyJsonTool::initialize()
{
  ATH_MSG_INFO("Initialize BTagging Efficiency Json Tool from: " + m_json_config_path);

  std::ifstream jsonFile(m_json_config_path);
  if (!jsonFile.is_open()) {
    ATH_MSG_ERROR( "JSON file " + m_json_config_path + " do not exist. Please put the correct path of the file." );
    return StatusCode::FAILURE;
  }
  m_json_config = json::parse(jsonFile);
  jsonFile.close();

  if (m_taggerName.empty() || !m_json_config.contains(m_taggerName)){
    ATH_MSG_ERROR( " Tagger " + m_taggerName + " not found in JSON file: " + m_json_config_path );
    return StatusCode::FAILURE;
  }

  if (m_jetAuthor.empty() || !m_json_config[m_taggerName].contains(m_jetAuthor)){
    ATH_MSG_ERROR( "Tagger: " +m_taggerName+ " and Jet Collection: " +m_jetAuthor+ " not found in JSON file: " +m_json_config_path );
    return StatusCode::FAILURE;
  }

  if (m_OP.empty() || !m_json_config[m_taggerName][m_jetAuthor].contains(m_OP)){
    ATH_MSG_ERROR( "OP " +m_OP+ " not available for " +m_taggerName+ " tagger.");
    return StatusCode::FAILURE;
  }

  const auto& meta = m_json_config[m_taggerName][m_jetAuthor]["meta"];
  m_truthlabel = meta["TruthLabel"];

  // map truth labels to categories
  for (auto& el : meta["labelMapping"].items()) {
    std::string label = el.key();
    for (const auto& num : el.value()) {
      int key = num; 
      m_labelMap[key] = label;
    }
  }

  // preload pt bins, systematics and SFs for each category
  auto& json_config_OP = m_json_config[m_taggerName][m_jetAuthor][m_OP];
  for (auto& label : meta["labelMapping"].items()) {
    std::string labelString = label.key();;

    for (const auto& pt : json_config_OP[labelString]["pt"]) {
      m_ptMap[labelString].push_back(BTaggingToolUtil::getExtendedFloat(pt));
    }

    m_sfMap[labelString] = json_config_OP[labelString]["nominal"].get<std::vector<float>>();
    for (auto& [systematicName, values] : json_config_OP[labelString]["systematics"].items()){
      m_sysMap[labelString][systematicName] = values.get<std::vector<float>>();
    }
  }
  
  m_currentSys = nullptr;
  m_sysCache.initialize(affectingSystematics(),
                   [this](const CP::SystematicSet& systConfig, sysData& sys)
                   {return calcSystematicVariation(systConfig, sys);});
  if (m_sysCache.get(CP::SystematicSet(), m_currentSys) != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Failed to initialize systematic cache");
    return StatusCode::FAILURE;
  }

  m_initialised = true;
  return StatusCode::SUCCESS;
}

CP::CorrectionCode BTaggingEfficiencyJsonTool::getScaleFactor( const xAOD::Jet& jet, float& sf, const CP::SystematicSet& sys ) const 
{
  if (! m_initialised) {
    throw std::runtime_error("BTaggingEfficiencyJsonTool has not been initialised.");
  }

  sf = 0.0;

  SG::AuxElement::ConstAccessor<int> truthLabelAccessor( m_truthlabel );
  int truthLabel = truthLabelAccessor( jet );
  std::string labelString;
  auto it = m_labelMap.find(truthLabel);
  if (it != m_labelMap.end()) {
    labelString = it->second;
  } else {
    ATH_MSG_WARNING("No calibration on jet with truthLabel: " << truthLabel << ". Returning scale factor of 0.");
    return CP::CorrectionCode::OutOfValidityRange;
  }

  const auto& pts = m_ptMap.at(labelString);
  size_t bin_index = pts.size();
  for (size_t i = 1; i < pts.size(); i++) {
    if (jet.pt()/1000. < pts[i]) {
      bin_index = i-1;
      break;
    }
  }

  const auto& SFs = m_sfMap.at(labelString);
  if (bin_index >= SFs.size()) {
    ATH_MSG_WARNING("No calibration for jet with pt: " << jet.pt()/1000. << ". Returning scale factor of 0.");
    return CP::CorrectionCode::OutOfValidityRange;
  }
  
  sf = SFs[bin_index];
  
  sysData tempSys;
  if (calcSystematicVariation(sys, tempSys) == StatusCode::SUCCESS && tempSys.xbb_syst != 0) {
    sf += tempSys.xbb_syst * getSFSys(labelString, bin_index);
  }

  return CP::CorrectionCode::Ok;
}

float BTaggingEfficiencyJsonTool::getSFSys( const std::string& labelString, size_t bin_index ) const
{
  float result = 0.0;
  const auto& systematics = m_sysMap.at(labelString);
  for (auto& [systematicName, values] : systematics){
    float sys_value = values.at(bin_index);
    result += sys_value*sys_value;
  }
  return std::sqrt(result);
}

StatusCode BTaggingEfficiencyJsonTool::calcSystematicVariation(const CP::SystematicSet& systConfig, sysData& sys) const
{
  CP::SystematicVariation syst = systConfig.getSystematicByBaseName("BTagging_Xbb_SYST");
  sys.xbb_syst = syst.parameter();

  return StatusCode::SUCCESS;
}

CP::SystematicSet BTaggingEfficiencyJsonTool::affectingSystematics() const
{
  CP::SystematicSet affectingSystematics;
  affectingSystematics.insert(CP::SystematicVariation("BTagging_Xbb_SYST", 1));
  affectingSystematics.insert(CP::SystematicVariation("BTagging_Xbb_SYST", -1));
  
  return affectingSystematics;
}

CP::SystematicSet BTaggingEfficiencyJsonTool::recommendedSystematics() const
{
    return affectingSystematics();
}

