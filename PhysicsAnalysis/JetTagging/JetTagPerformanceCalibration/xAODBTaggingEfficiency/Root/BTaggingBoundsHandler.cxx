/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODBTaggingEfficiency/BTaggingBoundsHandler.h"
#include <nlohmann/json.hpp>

// Two half-open intervals [low, high) overlap if each one starts before the other ends.
// Touching edges (upper = other lower) are not considered overlapping.
bool BTaggingBoundsHandler::varBounds::overlaps(const varBounds& oBounds) const {
  const bool endsBeforeOther = m_upperBound < oBounds.m_lowerBound || BTaggingToolUtil::approxEqual(m_upperBound, oBounds.m_lowerBound, m_varEps);
  const bool otherEndsBefore = oBounds.m_upperBound < m_lowerBound || BTaggingToolUtil::approxEqual(oBounds.m_upperBound, m_lowerBound, m_varEps);
  return !endsBeforeOther && !otherEndsBefore;
}
bool BTaggingBoundsHandler::varBounds::operator<(const varBounds& oBounds) const {
  if (!BTaggingToolUtil::approxEqual(m_lowerBound, oBounds.m_lowerBound, m_varEps)) return m_lowerBound < oBounds.m_lowerBound;
  if (!BTaggingToolUtil::approxEqual(m_upperBound, oBounds.m_upperBound, m_varEps)) return m_upperBound < oBounds.m_upperBound;
  // Both upper and lower bounds are equal within tolerance
  return false;
}
// Define an ordering of varBounds compare first lowerBound then upperBound
// within tolerance
bool BTaggingBoundsHandler::varBounds::operator==(const varBounds& oBounds) const {
  return BTaggingToolUtil::approxEqual(m_lowerBound, oBounds.m_lowerBound, m_varEps) && BTaggingToolUtil::approxEqual(m_upperBound, oBounds.m_upperBound, m_varEps);
}

BTaggingBoundsHandler::BTaggingBoundsHandler(
    const json& jsonConfig, JetQuantityGetter *funcGetJetQuantity){
  
  m_funcGetJetQuantity = funcGetJetQuantity;
  // Make sure that the pointer to the function provided is valud 
  if (!m_funcGetJetQuantity) {
    throw std::runtime_error("Jet quantity getter function is a null pointer for jsonConfig=" + jsonConfig.dump());
  }

  // Make sure the function that is held by the pointer has been defined 
  if ( !(*m_funcGetJetQuantity) ){ 
    throw std::runtime_error("Jet quantity getter function points to a std::function that has not been initialized for jsonConfig=" + jsonConfig.dump());
  }

  // The BTaggingBoundsHandler expects a json configuration that contains an entry called "binVariables". 
  // The content that is not read by the BTaggingBoundsHandler can be stored in another entry e.g. called "content" 
  // For example: 
  // jsonConfig = { "binVariables": {"pT": [250, 300 ]  , "mass": [50 , 75]}, "content":  {"SF": 0.9, uncerts:{ "SF_uncert_1": 0.1, "SF_uncert_2": 0.03}} }
  // The binVariables can contain several variables with [low, high) values specified 

  if (!jsonConfig.contains("binVariables")) {
    throw std::runtime_error(
      "jsonConfig does not contain binVariables entry jsonConfig=" + jsonConfig.dump());
  }

  // Retrieve binVariables 
  const auto &binVariables = jsonConfig["binVariables"];

  // Make sure that binVariables it is not empty and that it is an array 
  if (binVariables.empty()){
    throw std::runtime_error(
      "binVariables entry is empty for jsonConfig=" + jsonConfig.dump());
  }
  if (!binVariables.is_object()){
    throw std::runtime_error(
      "binVariables entry is not an object for jsonConfig=" + jsonConfig.dump());
  }
  
  // Loop over entries from binVariables
  // and retrieve corresponding min and max values for each variable
  for (const auto& [varName, varBinRange] : binVariables.items()){
    // Make sure that varBinRange is a list of [low, high] value 
    if (!varBinRange.is_array() || varBinRange.size() != 2) {
      throw std::runtime_error(
        "Bin range for varName='" + varName + "' must be [low, high] while varBinRange="+ varBinRange.dump() + " for jsonConfig=" + jsonConfig.dump());
    }

    // Retrieve variable type 
    BTaggingToolUtil::varType vType;
    try {
      vType = BTaggingToolUtil::getVariableType( varName );
    }
    catch (const std::exception& e) {
      // throw error 
      throw std::runtime_error("Could not retrieve variable type for varName='" + varName + "' for jsonConfig=" + jsonConfig.dump() + " --> Error=" + e.what());
    }

    try {
      // Retrieve the corresponding bounds 
      varBounds vBounds(
        BTaggingToolUtil::getExtendedFloat(varBinRange[0]),
        BTaggingToolUtil::getExtendedFloat(varBinRange[1])
      );

      // Finally add the varBounds to the map of variable bounds
      m_varBinBounds[vType] = vBounds;
    }
    catch (const std::exception& e) {
      throw std::runtime_error("Bin issue varName='" + varName + "' and varBinRange="+ varBinRange.dump() + " for jsonConfig=" + jsonConfig.dump() + " --> Error=" + e.what());
    }
  }
}

// Returns the list of variables of the handler 
std::vector<BTaggingToolUtil::varType> BTaggingBoundsHandler::getListVariables() const {
  std::vector<BTaggingToolUtil::varType> varTypes;
  for (const auto& itr : m_varBinBounds){
    varTypes.push_back(itr.first);
  }
  return varTypes;
}

bool BTaggingBoundsHandler::isJetWithinBounds(const xAOD::Jet& jet) const {
  // Loop over all variables e.g. pT, eta etc
  for (const auto& [vType, vBounds] : m_varBinBounds) {
    // For each variable get the corresponding jet value
    // e.g. get the jet pT if the variable is pT
    const float value = (*m_funcGetJetQuantity)(jet, vType);
    // Check if the jet value is within the lower and upper bound defined for the bin
    // e.g. check if jet pT is within pTmin and pTmax
    // If not within those bounds then the jet is not falling in the bin
    // thus return false
    if ( !(vBounds.isWithinBounds(value)) ) {
      return false;
    }
  }
  // If reaching this point it means the jet is falling into the bin
  // Hence returning true
  return true;
}

std::string BTaggingBoundsHandler::onlyBounds_to_string() const {
  std::ostringstream os;
  // Add the list of variables with lower and upper bounds
  // e.g. "pT: [250, 500], mass: [50, 100]"
  // The separator ", " is only added between entries, not after the last one
  bool first = true;
  for (const auto& [vType, vBounds] : m_varBinBounds) {
    if (!first) os << ", ";
    first = false;
    os << BTaggingToolUtil::getVariableName(vType) << ": [" << vBounds.getLowerBound() << ", " << vBounds.getUpperBound() << "]";
  }
  return os.str();
}

std::string BTaggingBoundsHandler::to_string() const {
  std::ostringstream os;
  os << "BTaggingBoundsHandler(" << onlyBounds_to_string() << ")";
  return os.str();
}

bool BTaggingBoundsHandler::operator<(const BTaggingBoundsHandler& oBounds) const {
  // Ordering of handlers loop over variables as stored in the map
  // and let the first variable whose bounds differ decide of the ordering

  // Make sure maps have the same size
  if (m_varBinBounds.size() != oBounds.m_varBinBounds.size()) {
    throw std::logic_error("Cannot compare handlers with a different number of variables: " + this->to_string() + " versus " + oBounds.to_string() );
  }

  // Make sure that the function used to retrieve the jet quantities is also the same 
  // We do not want to have two handlers with different functions 
  // as they may then return different jet quantities e.g. imagine one handler 
  // is returning the regular jet pT while the other one returns the regressed jet pT
  // that would imply migration and so we cannot ensure orthogonality in that case 
  if (m_funcGetJetQuantity != oBounds.m_funcGetJetQuantity){
    throw std::logic_error("Cannot compare handlers with a different jet quantity getter function: " + this->to_string() + " versus " + oBounds.to_string() );
  }

  // Loop over the different variables bounds and compare
  for (const auto& [vType, vBounds1] : m_varBinBounds) {
    // Make sure the other handler has the same variable
    const auto it = oBounds.m_varBinBounds.find(vType);
    if (it == oBounds.m_varBinBounds.end()) {
      throw std::logic_error("Cannot compare handlers: variable '" + BTaggingToolUtil::getVariableName(vType) +
                             "' missing in the other handler");
    }
    const varBounds& vBounds2 = it->second;
    // First variable whose bounds differ decides the ordering
    if (!(vBounds1 == vBounds2)) return vBounds1 < vBounds2;
  }
  return false;  // all bounds equal
}

bool BTaggingBoundsHandler::overlaps(const BTaggingBoundsHandler& oBounds) const {
  // Two handlers that are N-dimensional bins overlap if
  // for every variable their bounds overlap
  // If for one variable there is no overlapping bounds then the bins are not overlapping

  // Make sure maps have the same size
  if (m_varBinBounds.size() != oBounds.m_varBinBounds.size()) {
    throw std::logic_error("Cannot check overlap of handlers with a different number of variables: " + this->to_string() + " versus " + oBounds.to_string() );
  }

  // Make sure that the function used to retrieve the jet quantities is also the same 
  // We do not want to have two handlers with different functions 
  // as they may then return different jet quantities e.g. imagine one handler 
  // is returning the regular jet pT while the other one returns the regressed jet pT
  // that would imply migration and so we cannot ensure orthogonality in that case 
  if (m_funcGetJetQuantity != oBounds.m_funcGetJetQuantity){
    throw std::logic_error("Cannot compare handlers with a different jet quantity getter function: " + this->to_string() + " versus " + oBounds.to_string() );
  }

  // Loop over the different variables bounds and check the overlap
  for (const auto& [vType, vBounds1] : m_varBinBounds) {
    // Make sure the other handler has the same variable
    const auto it = oBounds.m_varBinBounds.find(vType);
    if (it == oBounds.m_varBinBounds.end()) {
      throw std::logic_error("Cannot check overlap of handlers: variable '" + BTaggingToolUtil::getVariableName(vType) +
                             "' missing in the other handler");
    }
    // If bounds do not overlap for one variable then bins do not overlap
    if (!vBounds1.overlaps(it->second)) return false;
  }
  // Bounds overlap for all variables
  // so handlers have some overlap
  return true;
}
