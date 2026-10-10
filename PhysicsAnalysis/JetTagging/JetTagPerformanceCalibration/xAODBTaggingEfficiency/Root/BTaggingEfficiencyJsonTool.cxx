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

  if (m_minPt > 0.){ 
    // Convert MeV cut to GeV as in the JSON files numbers are in GeV 
    m_minPt = m_minPt * BTaggingToolUtil::MeVToGeV; 
  }

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
  m_truthLabelAcc = std::make_unique<SG::ConstAccessor<int>>(truthLabel);

  // map truth labels to categories for scale factors
  // NB: MC-MC scale factors have a dedicated truth label to categories mapping
  if (meta.contains("labelMapping")) {
    for (auto& el : meta["labelMapping"].items()) {
      std::string label = el.key();
      for (const auto& num : el.value()) {
        int key = num; 
        m_labelMap[key] = label;
      }
    }
  } else {
    ATH_MSG_WARNING("No 'labelMapping' defined in JSON.");
  }

  // map truth labels to categories for MC-MC scale factors
  if (meta.contains("labelMappingMCMC")) {
    for (auto& el : meta["labelMappingMCMC"].items()) {
      std::string label = el.key();
      for (const auto& num : el.value()) {
        int key = num; 
        m_labelMapMCMC[key] = label;
      }
    }
  } else {
    ATH_MSG_WARNING("No 'labelMappingMCMC' defined in JSON.");
  }

  // Get mass decorator if specified
  if (meta.contains("Mass")) {
    std::string massDecoratorName = meta["Mass"].get<std::string>();
    if (massDecoratorName != "default") {
      m_massAcc = std::make_unique<SG::ConstAccessor<float>>(massDecoratorName);
      ATH_MSG_INFO("Using decorated mass '" << massDecoratorName << "' for Efficiency SF.");
    }
  }

  if (meta.contains("PT")) {
    std::string ptDecoratorName = meta["PT"].get<std::string>();
    if (ptDecoratorName != "default") {
      m_ptAcc = std::make_unique<SG::ConstAccessor<float>>(ptDecoratorName);
      ATH_MSG_INFO("Using decorated pT '" << ptDecoratorName << "' for Efficiency SF.");
    }
  }
  // preload pt bins, systematics and SFs for each category
  const auto& json_config_OP = m_json_config[m_outputName][m_jetAuthor][m_OP];
  if (json_config_OP.contains("data_mc")) {
    if (!meta.contains("labelMapping")) {
      ATH_MSG_ERROR("JSON contains data-mc scale factors but no 'labelMapping'.");
      return StatusCode::FAILURE;
    }
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
    ATH_MSG_WARNING("No calibration data-mc scale factors present in json.");
  }

  if (meta.contains("labelMappingMCMC") && !json_config_OP.contains("mc_mc")) {
    ATH_MSG_ERROR("JSON contains labelMappingMCMC but no mc-mc entry found for the WP used.");
    return StatusCode::FAILURE;
  }

  if (json_config_OP.contains("mc_mc")) {
    // Get the MC-MC json configuration related config
    const auto &json_config_OPMCMC = json_config_OP["mc_mc"];

    if (!meta.contains("labelMappingMCMC")) {
      ATH_MSG_ERROR("JSON contains mc-to-mc corrections but no 'labelMappingMCMC'.");
      return StatusCode::FAILURE;
    }
    if (!json_config_OPMCMC.contains("binsVariables")) {
      ATH_MSG_ERROR("No binning variables defined for mc-to-mc corrections in JSON.");
      return StatusCode::FAILURE;
    }
    if (!json_config_OPMCMC.contains("corrections")){
      ATH_MSG_ERROR("No mc-to-mc corrections in JSON.");
      return StatusCode::FAILURE;
    }

    const auto &varNames = json_config_OPMCMC["binsVariables"].get<std::vector<std::string>>();
    std::vector<varType> vTypes;
    vTypes.reserve(varNames.size());
    for (const auto& name : varNames) {
      const varType vType = getVariableType(name);
      vTypes.push_back(vType);
    }
    if (!m_mcGenerator.value().empty()) {
      // JSON file contains mc-to-mc corrections and user provided the mc generator
      // Hence retrieving relevant MC-MC information from the JSON file
      for (const auto& label : meta["labelMappingMCMC"].items()) {
        std::string labelStringMCMC = label.key();

        if (!json_config_OPMCMC["corrections"].contains(labelStringMCMC)) {
          ATH_MSG_ERROR("JSON file contains labelStringMCMC=" << labelStringMCMC <<  " but no corresponding MC-MC SF found in the file");
          return StatusCode::FAILURE;
        }
        // Retrieve the corrections associted to the label string 
        const auto& mcmcAtLabel = json_config_OPMCMC["corrections"][labelStringMCMC];
        if (mcmcAtLabel.contains("reference")) {
          // Retrieve the reference generator used for the calibration 
          m_mcReference[labelStringMCMC] = mcmcAtLabel["reference"];

          // If the MC generator of the sample being processed is the same as the reference MC generator
          // Then there is no MC-MC scale factor to be applied as processing the same MC generator as used for the calibration
          // Hence skipping the rest 
          if (m_mcGenerator.value() == m_mcReference.at(labelStringMCMC)) continue;

          // Make sure the generator associated to the sample being processed is provided 
          if (!mcmcAtLabel.contains(m_mcGenerator.value())) {
            ATH_MSG_ERROR("No mc-to-mc corrections for generator '" << m_mcGenerator.value() << "' and label " << labelStringMCMC);
            return StatusCode::FAILURE;
          }
          
          for (const auto& entry : mcmcAtLabel[m_mcGenerator.value()]) {
            try {
              m_mcmcHandlers[labelStringMCMC].push_back(MCMCHandler(entry, vTypes));
              // // Useful for debugging
              // // Get the latest handler that was just added 
              // const MCMCHandler &handler = m_mcmcHandlers[labelStringMCMC][ m_mcmcHandlers[labelStringMCMC].size() -1 ];
              // // Then print its content 
              // std::cout << "labelStringMCMC=" << labelStringMCMC << ": " << handler.to_string() << std::endl;
            } catch (const std::exception& e) {
              ATH_MSG_ERROR("Malformed mc-to-mc correction for label " << labelStringMCMC
                            << " and generator '" << m_mcGenerator.value() << "': " << e.what());
              return StatusCode::FAILURE;
            }
          }
        } else {
          ATH_MSG_ERROR("mc-to-mc corrections incomplete for jet with truthLabel: " << labelStringMCMC << ".");
          return StatusCode::FAILURE;
        }
      }
    } else {
        // The JSON file contains MC-MC scale factors but the user did not provide the MC Generator of the sample processed
        ATH_MSG_ERROR("No mc generator passed to the tool. Please provide the mc generator for the mc-to-mc corrections.");
        return StatusCode::FAILURE;
    }
  } else {
    // No mc-to-mc corrections present in the given json file.
    ATH_MSG_WARNING("No mc-to-mc corrections present in json.");
  }

  // Now check there is no overlapping bins for MCMChandlers  
  for (const auto& [labelStringMCMC, handlers] : m_mcmcHandlers) {
    for (size_t i = 0; i < handlers.size(); ++i) {
      for (size_t j = i + 1; j < handlers.size(); ++j) {
        if (handlers[i].overlaps(handlers[j])) {
          ATH_MSG_ERROR("Overlapping mc-to-mc bins for label " << labelStringMCMC
                        << " and generator '" << m_mcGenerator.value() << "': "
                        << handlers[i] << " and " << handlers[j] );
          return StatusCode::FAILURE;
        }
      }
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

  sf = 1.0;

  int truthLabel = (*m_truthLabelAcc)(jet);
  std::string labelString;
  auto it = m_labelMap.find(truthLabel);
  if (it != m_labelMap.end()) {
    labelString = it->second;
  } else {
    ATH_MSG_WARNING("No calibration on jet with truthLabel: " << truthLabel << ". Returning scale factor of 1.");
    return CP::CorrectionCode::OutOfValidityRange;
  }

  if (!m_sfPtMap.contains(labelString)) {
    ATH_MSG_WARNING("No calibration on jet with truthLabel: " << truthLabel << ". Returning scale factor of 1.");
    return CP::CorrectionCode::OutOfValidityRange;
  }
  const auto& pts = m_sfPtMap.at(labelString);
  size_t bin_index = pts.size();
  if (getJetPtInGeV(jet) < pts[0]) {
    ATH_MSG_WARNING("No calibration for jet with pt: " << getJetPtInGeV(jet) << ". Returning scale factor of 1.");
    return CP::CorrectionCode::OutOfValidityRange;  
  }
  for (size_t i = 1; i < pts.size(); i++) {
    if (getJetPtInGeV(jet) < pts[i]) {
      bin_index = i-1;
      break;
    }
  }

  const auto& SFs = m_sfMap.at(labelString);
  if (bin_index >= SFs.size()) {
    ATH_MSG_WARNING("No calibration for jet with pt: " << getJetPtInGeV(jet) << ". Returning scale factor of 1.");
    return CP::CorrectionCode::OutOfValidityRange;
  }

  sf = SFs[bin_index];
  
  float corr = 1.;
  CP::CorrectionCode worstCorrectionCode = CP::CorrectionCode::Ok;
  CP::CorrectionCode ccMCMC = getMCToMCCorr(jet, corr);
  worstCorrectionCode = std::min(worstCorrectionCode, ccMCMC);
  sf *= corr;
  
  sysData tempSys;
  if (calcSystematicVariation(sys, tempSys) == StatusCode::SUCCESS && tempSys.xbb_syst != 0) {
    sf += tempSys.xbb_syst * getSFSys(labelString, bin_index);
  }

  return worstCorrectionCode;
}

CP::CorrectionCode BTaggingEfficiencyJsonTool::getMCToMCCorr( const xAOD::Jet& jet, float& corr ) const 
{
  if (! m_initialised) {
    throw std::runtime_error("BTaggingEfficiencyJsonTool has not been initialised.");
  }

  corr = 1.0;

  // If MC-MC handlers map is empty it means the JSON file does not contain MC-MC scale factors
  // Hence exiting directly from the function
  if (m_mcmcHandlers.empty()) { 
   return CP::CorrectionCode::Ok;
  }

  // Retrieve jet truth labelling
  // and check if a string label is associated to it
  // e.g. truthLabel integer is associated to fully-contained top->qqb and the corresponding string label could be "Top" for instance
  int truthLabel = (*m_truthLabelAcc)(jet);
  auto labelIt = m_labelMapMCMC.find(truthLabel);
  if (labelIt == m_labelMapMCMC.end()) {
    ATH_MSG_WARNING("No mc-to-mc correction on jet with truthLabel: " << truthLabel << ". Returning scale factor of 1.");
    return CP::CorrectionCode::OutOfValidityRange;  // this process has no mc-to-mc correction
  }

  const std::string& label = labelIt->second;
  auto handlerIt = m_mcmcHandlers.find(label);
  if (handlerIt == m_mcmcHandlers.end()) {
    // If the generator of the sample processed is the same as the generator of the reference sample for the given label
    // then the map of MC-MC handler is not filled for that label (check happening in the initialize function) 
    // Thus exiting here as the generator of the sample being processed is the same generator as the one used for the calibration
    return CP::CorrectionCode::Ok; 
  }

  // Loop over all MC-MC handlers and find the one where the jet if falling into
  for (const auto& handler : handlerIt->second) {
    if (handler.isJetWithinBounds(jet, *this)) {
      // // Useful for debugging
      // // Print jet truth label pT mass and eta and the corresponding handler
      // std::cout << "jet(label=" << label 
      //   << ", pT=" << getJetQuantity(jet, "pT") 
      //   << ", mass=" << getJetQuantity(jet, "mass") 
      //   << ", abseta=" << getJetQuantity(jet, "abseta") << "): "
      //   << handler << std::endl;
      
      corr = handler.getScaleFactor();
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

float BTaggingEfficiencyJsonTool::getJetMassInGeV(const xAOD::Jet& jet) const
{   
    // Convert mass from MeV to GeV 
    if (!m_massAcc) {
      return jet.m() * BTaggingToolUtil::MeVToGeV;
    }
    if (!m_massAcc->isAvailable(jet)) {
        ATH_MSG_ERROR("Decorated mass '" << SG::AuxTypeRegistry::instance().getName( m_massAcc->auxid() ) << "' not available on jet. Cannot proceed.");
        throw std::runtime_error("Decorated mass not available on jet.");
    }
    return (*m_massAcc)(jet) * BTaggingToolUtil::MeVToGeV;
}

float BTaggingEfficiencyJsonTool::getJetPtInGeV( const xAOD::Jet& jet ) const
{ 
  // Convert pT from MeV to GeV 
  if (!m_ptAcc) {
    return jet.pt() * BTaggingToolUtil::MeVToGeV;
  }

  if (!m_ptAcc->isAvailable(jet)) {
    ATH_MSG_ERROR("Decorated pT '" << SG::AuxTypeRegistry::instance().getName( m_ptAcc->auxid() ) << "' not available on jet. Cannot proceed.");
    throw std::runtime_error("Decorated pT not available on jet.");
  }

  return (*m_ptAcc)(jet) * BTaggingToolUtil::MeVToGeV;
}

float BTaggingEfficiencyJsonTool::getJetQuantity(const xAOD::Jet& jet, varType vType) const {
  if (vType == varType::pT) { 
    return getJetPtInGeV(jet); 
  } else if (vType == varType::mass) {
    return getJetMassInGeV(jet);
  }  else if (vType == varType::eta) {
    return jet.eta();
  } else if (vType == varType::abseta) {
    return std::abs(jet.eta());
  } else { 
    ATH_MSG_ERROR("Unsupported jet variable quantity requested: '" << getVariableName(vType) << "'");
    throw std::runtime_error("BTaggingEfficiencyJsonTool::getJetQuantity unsupported jet variable: '" + getVariableName(vType) + "'");
  }
}

// Two half-open intervals [low, high) overlap if each one starts before the other ends.
// Touching edges (upper = other lower) are not considered overlapping.
bool BTaggingEfficiencyJsonTool::BoundsHandler::varBounds::overlaps(const varBounds& o) const {
  const bool endsBeforeOther = upperBound < o.lowerBound || approxEqual(upperBound, o.lowerBound, varEps);
  const bool otherEndsBefore = o.upperBound < lowerBound || approxEqual(o.upperBound, lowerBound, varEps);
  return !endsBeforeOther && !otherEndsBefore;
}
bool BTaggingEfficiencyJsonTool::BoundsHandler::varBounds::operator<(const varBounds& o) const {
  if (!approxEqual(lowerBound, o.lowerBound, varEps)) return lowerBound < o.lowerBound;
  if (!approxEqual(upperBound, o.upperBound, varEps)) return upperBound < o.upperBound;
  // Both upper and lower bounds are equal within tolerance
  return false;
}
// Define an ordering of varBounds compare first lowerBound then upperBound
// within tolerance
bool BTaggingEfficiencyJsonTool::BoundsHandler::varBounds::operator==(const varBounds& o) const {
  return approxEqual(lowerBound, o.lowerBound, varEps) && approxEqual(upperBound, o.upperBound, varEps);
}

BTaggingEfficiencyJsonTool::BoundsHandler::BoundsHandler(
    const json& jsonConfig, const std::vector<varType>& vTypes, const size_t nExtraValues)
{
  // Make sure the vector of variable provided is not empty
  if (vTypes.empty()){
    throw std::runtime_error(
      "empty list of bin variables");
  }

  // The entry should be a list starting with one [low, high] range per variable in varNames
  // followed by nExtraValues values handled by the child class
  // NB: it's possible to pass all additional information with only one extra value 
  // that is itself a JSON list and can hold several fields e.g. the SF and its uncertainty.
  // varNames={"pT", "mass"}, nExtraValues=1 and entry = [[250, 500], [50, 100], {"SF": 0.9, "SF_uncert_1": 0.1, "SF_uncert_2": 0.03}]
  if (!jsonConfig.is_array() || jsonConfig.size() != vTypes.size() + nExtraValues) {
    throw std::runtime_error(
      "Bin config must contain " + std::to_string(vTypes.size()) +
      " [low, high] ranges followed by " + std::to_string(nExtraValues) + " value(s): jsonConfig=" + jsonConfig.dump());
  }

  // Loop over variables and retrieve corresponding min and max values
  for (size_t v = 0; v < vTypes.size(); ++v) {
    const auto& range = jsonConfig[v];
    if (!range.is_array() || range.size() != 2) {
      throw std::runtime_error(
        "Bin range for variable '" + getVariableName(vTypes[v]) + "' must be [low, high]");
    }
    varBounds vBounds;
    vBounds.lowerBound = BTaggingToolUtil::getExtendedFloat(range[0]);
    vBounds.upperBound = BTaggingToolUtil::getExtendedFloat(range[1]);

    // Let's make sure that lowerBound < upperBound
    if (vBounds.lowerBound >= vBounds.upperBound){
      throw std::runtime_error(
        "Bin issue, the min value >= max value for varName='" + getVariableName(vTypes[v]) + "': min=" + std::to_string(vBounds.lowerBound)+ ", max=" + std::to_string(vBounds.upperBound) + " for jsonConfig=" + jsonConfig.dump());
    }

    // Finally add the varBounds to the map of variable bounds
    m_varBinBounds[vTypes[v]] = vBounds;
  }
}

BTaggingEfficiencyJsonTool::MCMCHandler::MCMCHandler(
    const json& jsonConfig, const std::vector<varType>& vTypes)
  // The bounds are loaded by the parent class, the json config should have one extra entry and should contain the MC-MC SF
  // e.g. jsonConfig = [[250, 500], [50, 100], {"SF": 0.9}] for varNames={"pT", "mass"}
  : BoundsHandler(jsonConfig, vTypes)
{ 
  // Get last entry 
  const auto & mcmcEntry = jsonConfig.back(); 
  // The last entry should be a JSON with the SF 
  if (!mcmcEntry.is_object()) {
    throw std::runtime_error("mc-to-mc last entry in jsonConfig is not an object for jsonConfig=" + mcmcEntry.dump());
  }

  // Check if the last entry contains the SF and that's a number 
  if (!mcmcEntry.contains("SF") || !mcmcEntry["SF"].is_number()){
     throw std::runtime_error("mc-to-mc mcmcEntry is not containing a scale factor for jsonConfig=" + jsonConfig.dump());
  }

  // Retrieve the MC-MC scale factor
  m_MCMCSF = mcmcEntry["SF"].get<float>();
}

bool BTaggingEfficiencyJsonTool::BoundsHandler::isJetWithinBounds(
    const xAOD::Jet& jet, const BTaggingEfficiencyJsonTool& tool) const
{
  // Loop over all variables e.g. pT, eta etc
  for (const auto& [vType, bounds] : m_varBinBounds) {
    // For each variable get the corresponding jet value
    // e.g. get the jet pT if the variable is pT
    const float value = tool.getJetQuantity(jet, vType);
    // Check if the jet value is within the lower and upper bound defined for the bin
    // e.g. check if jet pT is within pTmin and pTmax
    // If not within those bounds then the jet is not falling in the bin
    // thus return false
    if (value < bounds.lowerBound || value >= bounds.upperBound) {
      return false;
    }
  }
  // If reaching this point it means the jet is falling into the bin
  // Hence returning true
  return true;
}

BTaggingEfficiencyJsonTool::varType BTaggingEfficiencyJsonTool::getVariableType(const std::string& varName)
{
  if (varName == "pT") {
    return varType::pT;
  } else if (varName == "mass") {
    return varType::mass;
  } else if (varName == "eta") {
    return varType::eta;
  } else if (varName == "abseta") {
    return varType::abseta;
  } else {
    throw std::runtime_error("Variable '" + varName + "' not defined in BTaggingEfficiencyJsonTool.");
    return varType::unknown;
  }
}

std::string BTaggingEfficiencyJsonTool::getVariableName(varType vType)
{
  if (vType == varType::pT) {
    return "pT";
  } else if (vType == varType::mass) {
    return "mass";
  } else if (vType == varType::eta) {
    return "eta";
  } else if (vType == varType::abseta) {
    return "abseta";
  } else {
    throw std::runtime_error("Variable type " + std::to_string(static_cast<int>(vType)) + " not defined in BTaggingEfficiencyJsonTool.");
    return "unknown";
  }
}

std::ostream& operator<<(std::ostream& os, const BTaggingEfficiencyJsonTool::BoundsHandler& handler){
  // Add the list of variables with lower and upper bounds
  // e.g. "pT: [250, 500], mass: [50, 100]"
  // The separator ", " is only added between entries, not after the last one
  bool first = true;
  for (const auto& [vType, bounds] : handler.m_varBinBounds) {
    if (!first) os << ", ";
    first = false;
    os << BTaggingEfficiencyJsonTool::getVariableName(vType) << ": [" << bounds.lowerBound << ", " << bounds.upperBound << "]";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const BTaggingEfficiencyJsonTool::MCMCHandler& handler){
  // Return
  // MCMCHandler(var1: [minVar1, maxVar1], var2: [minVar2, maxVar2], ... , SF: sfValue)
  // e.g. MCMCHandler(pT: [250, 500], abseta[0., 2.0], SF: 0.9)
  os << "MCMCHandler(";
  // Add the list of variables with lower and upper bounds
  os << dynamic_cast<const BTaggingEfficiencyJsonTool::BoundsHandler&>(handler);
  // Finally add the MC-MC SF value
  os << ", SF: " << handler.m_MCMCSF << ")";
  return os;
}

bool BTaggingEfficiencyJsonTool::BoundsHandler::operator<(const BoundsHandler& o) const {
  // Ordering of handlers loop over variables as stored in the map
  // and let the first variable whose bounds differ decide of the ordering

  // Make sure maps have the same size
  if (m_varBinBounds.size() != o.m_varBinBounds.size()) {
    throw std::logic_error("Cannot compare handlers with a different number of variables");
  }
  // Loop over the different variables bounds and compare
  for (const auto& [vType, vBounds1] : m_varBinBounds) {
    // Make sure the other handler has the same variable
    const auto it = o.m_varBinBounds.find(vType);
    if (it == o.m_varBinBounds.end()) {
      throw std::logic_error("Cannot compare handlers: variable '" + getVariableName(vType) +
                             "' missing in the other handler");
    }
    const varBounds& vBounds2 = it->second;
    // First variable whose bounds differ decides the ordering
    if (!(vBounds1 == vBounds2)) return vBounds1 < vBounds2;
  }
  return false;  // all bounds equal
}

bool BTaggingEfficiencyJsonTool::BoundsHandler::overlaps(const BoundsHandler& o) const {
  // Two handlers that are N-dimensional bins overlap if
  // for every variable their bounds overlap
  // If for one variable there is no overlapping bounds then the bins are not overlapping

  // Make sure maps have the same size
  if (m_varBinBounds.size() != o.m_varBinBounds.size()) {
    throw std::logic_error("Cannot check overlap of handlers with a different number of variables");
  }
  // Loop over the different variables bounds and check the overlap
  for (const auto& [vType, vBounds1] : m_varBinBounds) {
    // Make sure the other handler has the same variable
    const auto it = o.m_varBinBounds.find(vType);
    if (it == o.m_varBinBounds.end()) {
      throw std::logic_error("Cannot check overlap of handlers: variable '" + getVariableName(vType) +
                             "' missing in the other handler");
    }
    // If bounds do not overlap for one variable then bins do not overlap
    if (!vBounds1.overlaps(it->second)) return false;
  }
  // Bounds overlap for all variables
  // so handlers have some overlap
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

