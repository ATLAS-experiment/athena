/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PathResolver/PathResolver.h"
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

  std::string pathToJsonConfigFile = PathResolverFindCalibFile(m_json_config_path);
  std::ifstream jsonFile(pathToJsonConfigFile);

  if (!jsonFile.is_open()) {
    ATH_MSG_ERROR( "JSON file " + m_json_config_path + " does not exist. Please put the correct path of the file." );
    return StatusCode::FAILURE;
  }
  m_json_config = json::parse(jsonFile);
  jsonFile.close();

  if (m_outputName.empty() || !m_json_config.contains(m_outputName)){
    ATH_MSG_ERROR( "The output name " + m_outputName + " not found in JSON file: " + m_json_config_path );
    return StatusCode::FAILURE;
  }

  if (m_jetAuthor.empty() || !m_json_config[m_outputName].contains(m_jetAuthor)){
    ATH_MSG_ERROR( "Tagger: " +m_outputName+ " and Jet Collection: " +m_jetAuthor+ " not found in JSON file: " +m_json_config_path );
    return StatusCode::FAILURE;
  }

  if (m_OP.empty() || !m_json_config[m_outputName][m_jetAuthor].contains(m_OP)){
    ATH_MSG_ERROR( "OP " +m_OP+ " not available for " +m_outputName+ " tagger.");
    return StatusCode::FAILURE;
  }

  const auto& meta = m_json_config[m_outputName][m_jetAuthor]["meta"];
  const std::string truthLabel = meta["TruthLabel"];
  m_truthLabelAcc = std::make_unique<SG::AuxElement::ConstAccessor<int>>(truthLabel);

  // map truth labels to categories
  for (auto& el : meta["labelMapping"].items()) {
    std::string label = el.key();
    for (const auto& num : el.value()) {
      int key = num; 
      m_labelMap[key] = label;
    }
  }

  // map truth labels to categories
  for (auto& el : meta["labelMappingMCMC"].items()) {
    std::string label = el.key();
    for (const auto& num : el.value()) {
      int key = num; 
      m_labelMapMCMC[key] = label;
    }
  }

  // Get mass decorator if specified
  if (meta.contains("Mass")) {
    std::string massDecoratorName = meta["Mass"].get<std::string>();
    if (massDecoratorName != "default") {
      m_massAcc = std::make_unique<SG::AuxElement::ConstAccessor<float>>(massDecoratorName);
      ATH_MSG_INFO("Using decorated mass '" << massDecoratorName << "' for Efficiency SF.");
    }
  }

  if (meta.contains("PT")) {
    std::string ptDecoratorName = meta["PT"].get<std::string>();
    if (ptDecoratorName != "default") {
      m_ptAcc = std::make_unique<SG::AuxElement::ConstAccessor<float>>(ptDecoratorName);
      ATH_MSG_INFO("Using decorated pT '" << ptDecoratorName << "' for Efficiency SF.");
    }
  }
  // preload pt bins, systematics and SFs for each category
  auto& json_config_OP = m_json_config[m_outputName][m_jetAuthor][m_OP];
  if (json_config_OP.contains("data_mc")) {
    for (const auto& label : meta["labelMapping"].items()) {
      std::string labelString = label.key();
      if (json_config_OP["data_mc"].contains(labelString)) {
        for (const auto& pt : json_config_OP["data_mc"][labelString]["pt"]) {
          m_sfPtMap[labelString].push_back(BTaggingToolUtil::getExtendedFloat(pt));
        }

        m_sfMap[labelString] = json_config_OP["data_mc"][labelString]["nominal"].get<std::vector<float>>();
        for (auto& [systematicName, values] : json_config_OP["data_mc"][labelString]["systematics"].items()){
          m_sfSysMap[labelString][systematicName] = values.get<std::vector<float>>();
        }
      }
    }
  } else {
    ATH_MSG_INFO("No calibration data-mc scale factors present in json.");
  }

  if (!m_mcGenerator.value().empty()) {
    if (json_config_OP.contains("mc_mc")) {
      for (const auto& label : meta["labelMappingMCMC"].items()) {
        std::string labelStringMCMC = label.key();
        if (!json_config_OP["mc_mc"].contains(labelStringMCMC)) continue;
        auto& mcmcAtLabel = json_config_OP["mc_mc"][labelStringMCMC];
        if (mcmcAtLabel.contains("reference") && mcmcAtLabel.contains("corrections") && mcmcAtLabel.contains("binsVariables")) {
          m_mcReference[labelStringMCMC] = mcmcAtLabel["reference"];
          if (m_mcGenerator.value() == m_mcReference[labelStringMCMC]) continue;
          const auto varNames = mcmcAtLabel["binsVariables"].get<std::vector<std::string>>();
          
          if (!mcmcAtLabel["corrections"].contains(m_mcGenerator.value())) {
            ATH_MSG_ERROR("No mc-to-mc corrections for generator '" << m_mcGenerator.value() << "' and label " << labelStringMCMC);
            return StatusCode::FAILURE;
          }

          auto& handlers = m_mcmcHandlers[labelStringMCMC];
          for (const auto& entry : mcmcAtLabel["corrections"][m_mcGenerator.value()]) {
            std::map<std::string, MCMCHandler::varBounds> bounds;
            for (size_t v = 0; v < varNames.size(); ++v) {
              const auto& range = entry[v];
              if (range.size() != 2) {
                ATH_MSG_ERROR("Bin range incorrect for mc-to-mc corrections");
                return StatusCode::FAILURE;
              }
              bounds[varNames[v]] = {BTaggingToolUtil::getExtendedFloat(range[0]),
                                      BTaggingToolUtil::getExtendedFloat(range[1])};
            }
            handlers.emplace_back(std::move(bounds), entry.back().get<float>());
          } 
        } else {
          ATH_MSG_ERROR("mc-to-mc corrections incomplete for jet with truthLabel: " << labelStringMCMC << ".");
          return StatusCode::FAILURE;
        }
      }
    } else {
        ATH_MSG_INFO("No mc-to-mc corrections present in json.");
    }
  } else {
    ATH_MSG_INFO("No mc-generator added by user. No mc-to-mc corrections will be applied.");
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

  int truthLabel = (*m_truthLabelAcc)(jet);
  std::string labelString;
  auto it = m_labelMap.find(truthLabel);
  if (it != m_labelMap.end()) {
    labelString = it->second;
  } else {
    ATH_MSG_WARNING("No calibration on jet with truthLabel: " << truthLabel << ". Returning scale factor of 0.");
    return CP::CorrectionCode::OutOfValidityRange;
  }

  if (!m_sfPtMap.contains(labelString)) {
    ATH_MSG_WARNING("No calibration on jet with truthLabel: " << truthLabel << ". Returning scale factor of 0.");
    return CP::CorrectionCode::OutOfValidityRange;
  }
  const auto& pts = m_sfPtMap.at(labelString);
  size_t bin_index = pts.size();
  if (getJetPt(jet)/1000. < pts[0]) {
    ATH_MSG_WARNING("No calibration for jet with pt: " << getJetPt(jet)/1000. << ". Returning scale factor of 0.");
    return CP::CorrectionCode::OutOfValidityRange;  
  }
  for (size_t i = 1; i < pts.size(); i++) {
    if (getJetPt(jet)/1000. < pts[i]) {
      bin_index = i-1;
      break;
    }
  }

  const auto& SFs = m_sfMap.at(labelString);
  if (bin_index >= SFs.size()) {
    ATH_MSG_WARNING("No calibration for jet with pt: " << getJetPt(jet)/1000. << ". Returning scale factor of 0.");
    return CP::CorrectionCode::OutOfValidityRange;
  }

  sf = SFs[bin_index];
  
  float corr = 1.f;
  CP::CorrectionCode cc = getMCToMCCorr(jet, corr);
  if (cc != CP::CorrectionCode::Ok) return cc;
  sf *= corr;
  
  sysData tempSys;
  if (calcSystematicVariation(sys, tempSys) == StatusCode::SUCCESS && tempSys.xbb_syst != 0) {
    sf += tempSys.xbb_syst * getSFSys(labelString, bin_index);
  }

  return CP::CorrectionCode::Ok;
}

CP::CorrectionCode BTaggingEfficiencyJsonTool::getMCToMCCorr( const xAOD::Jet& jet, float& corr ) const 
{
  if (! m_initialised) {
    throw std::runtime_error("BTaggingEfficiencyJsonTool has not been initialised.");
  }

  corr = 1.0;

  if (m_mcmcHandlers.empty()) { 
   return CP::CorrectionCode::Ok;
  }

  int truthLabel = (*m_truthLabelAcc)(jet);
  auto labelIt = m_labelMapMCMC.find(truthLabel);
  if (labelIt == m_labelMapMCMC.end()) {
    ATH_MSG_WARNING("No mc-to-mc correction on jet with truthLabel: " << truthLabel << ". Returning scale factor of 1.");
    return CP::CorrectionCode::Ok;  // this process has no mc-to-mc correction
  }

  const std::string& label = labelIt->second;
  auto refIt = m_mcReference.find(label);
  if (refIt != m_mcReference.end() && refIt->second == m_mcGenerator.value()) {
    return CP::CorrectionCode::Ok;
  }

  auto handlerIt = m_mcmcHandlers.find(labelIt->second);
  if (handlerIt == m_mcmcHandlers.end()) {
    return CP::CorrectionCode::Ok;  // no mc-to-mc corrections defined for this label
  }

  for (const auto& bin : handlerIt->second) {
    if (bin.isJetWithinBounds(jet, *this)) {
      corr = bin.getFactor();
      return CP::CorrectionCode::Ok;
    }
  }

  ATH_MSG_WARNING("Jet outside mc-to-mc binning for label " << label << ". Returning correction of 1.");
  return CP::CorrectionCode::OutOfValidityRange;
}

float BTaggingEfficiencyJsonTool::getSFSys( const std::string& labelString, size_t bin_index ) const
{
  float result = 0.0;
  const auto& systematics = m_sfSysMap.at(labelString);
  for (auto& [systematicName, values] : systematics){
    float sys_value = values.at(bin_index);
    result += sys_value*sys_value;
  }
  return std::sqrt(result);
}

float BTaggingEfficiencyJsonTool::getJetMass(const xAOD::Jet& jet) const
{
    if (!m_massAcc) return jet.m();
    if (!m_massAcc->isAvailable(jet)) {
        ATH_MSG_ERROR("Decorated mass '" << SG::AuxTypeRegistry::instance().getName( m_massAcc->auxid() ) << "' not available on jet. Cannot proceed.");
        throw std::runtime_error("Decorated mass not available on jet.");
    }
    return (*m_massAcc)(jet);
}

float BTaggingEfficiencyJsonTool::getJetPt( const xAOD::Jet& jet ) const
{
  if (!m_ptAcc) {
    return jet.pt();
  }

  if (!m_ptAcc->isAvailable(jet)) {
    ATH_MSG_ERROR("Decorated pT '" << SG::AuxTypeRegistry::instance().getName( m_ptAcc->auxid() ) << "' not available on jet. Cannot proceed.");
    throw std::runtime_error("Decorated pT not available on jet.");
  }

  return (*m_ptAcc)(jet);
}

float BTaggingEfficiencyJsonTool::getJetQuantity(const xAOD::Jet& jet, const std::string &varName) const {
  if (varName == "pT") { 
    return getJetPt(jet)/1000.; 
  } else if (varName == "mass") {
    return getJetMass(jet)/1000;
  } else { 
    ATH_MSG_ERROR("No variable '" << varName << "' defined for mc-to-mc corrections.");
    throw std::runtime_error("Unknown mc-to-mc variable '" + varName + "'");
  } 
}

bool BTaggingEfficiencyJsonTool::MCMCHandler::isJetWithinBounds(
    const xAOD::Jet& jet, const BTaggingEfficiencyJsonTool& tool) const
{
  for (const auto& [varName, bounds] : m_varBinBounds) {
    const float value = tool.getJetQuantity(jet, varName);
    if (value < bounds.lowerBound || value >= bounds.upperBound) {
      return false;
    }
  }
  return true;
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

