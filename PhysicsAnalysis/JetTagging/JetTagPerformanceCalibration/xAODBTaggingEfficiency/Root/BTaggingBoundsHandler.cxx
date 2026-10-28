/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODBTaggingEfficiency/BTaggingBoundsHandler.h"

// Two half-open intervals [low, high) overlap if each one starts before the other ends.
// Touching edges (upper = other lower) are not considered overlapping.
bool BTaggingBoundsHandler::varBounds::overlaps(const varBounds& o) const {
  const bool endsBeforeOther = upperBound < o.lowerBound || BTaggingToolUtil::approxEqual(upperBound, o.lowerBound, varEps);
  const bool otherEndsBefore = o.upperBound < lowerBound || BTaggingToolUtil::approxEqual(o.upperBound, lowerBound, varEps);
  return !endsBeforeOther && !otherEndsBefore;
}
bool BTaggingBoundsHandler::varBounds::operator<(const varBounds& o) const {
  if (!BTaggingToolUtil::approxEqual(lowerBound, o.lowerBound, varEps)) return lowerBound < o.lowerBound;
  if (!BTaggingToolUtil::approxEqual(upperBound, o.upperBound, varEps)) return upperBound < o.upperBound;
  // Both upper and lower bounds are equal within tolerance
  return false;
}
// Define an ordering of varBounds compare first lowerBound then upperBound
// within tolerance
bool BTaggingBoundsHandler::varBounds::operator==(const varBounds& o) const {
  return BTaggingToolUtil::approxEqual(lowerBound, o.lowerBound, varEps) && BTaggingToolUtil::approxEqual(upperBound, o.upperBound, varEps);
}

BTaggingBoundsHandler::BTaggingBoundsHandler(
    const json& jsonConfig, JetQuantityGetter funcGetJetQuantity)
  : m_funcGetJetQuantity(std::move(funcGetJetQuantity))
{
  if (!m_funcGetJetQuantity) {
    throw std::runtime_error("No jet quantity getter provided for jsonConfig=" + jsonConfig.dump());
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
      throw std::runtime_error("Could not retrieve variable type for varName='" + varName + "': " + e.what());
    }

    // Retrieve the corresponding bounds 
    varBounds vBounds;
    vBounds.lowerBound = BTaggingToolUtil::getExtendedFloat(varBinRange[0]);
    vBounds.upperBound = BTaggingToolUtil::getExtendedFloat(varBinRange[1]);

    // Make sure that lowerBounds and upperBounds are not NaNs
    if (!std::isfinite(vBounds.lowerBound)){
      throw std::runtime_error(
        "Bin issue lower bound is not a finite number for varName='" + varName + 
        "' while varBinRange="+ varBinRange.dump() + " for jsonConfig=" + jsonConfig.dump());
    }
    if (!std::isfinite(vBounds.upperBound)){
      throw std::runtime_error(
        "Bin issue upper bound is not a finite number for varName='" + varName + 
        "' while varBinRange="+ varBinRange.dump() + " for jsonConfig=" + jsonConfig.dump());
    }

    // Let's make sure that lowerBound < upperBound
    if (vBounds.lowerBound >= vBounds.upperBound){
      throw std::runtime_error(
        "Bin issue, the min value >= max value for varName='" + varName +"': min=" + std::to_string(vBounds.lowerBound)+ ", max=" + std::to_string(vBounds.upperBound) + " for jsonConfig=" + jsonConfig.dump());
    }

    // Finally add the varBounds to the map of variable bounds
    m_varBinBounds[vType] = vBounds;
  }
}

bool BTaggingBoundsHandler::isJetWithinBounds(const xAOD::Jet& jet) const
{
  // Loop over all variables e.g. pT, eta etc
  for (const auto& [vType, bounds] : m_varBinBounds) {
    // For each variable get the corresponding jet value
    // e.g. get the jet pT if the variable is pT
    const float value = m_funcGetJetQuantity(jet, vType);
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

std::ostream& operator<<(std::ostream& os, const BTaggingBoundsHandler& handler){
  // Add the list of variables with lower and upper bounds
  // e.g. "pT: [250, 500], mass: [50, 100]"
  // The separator ", " is only added between entries, not after the last one
  bool first = true;
  for (const auto& [vType, bounds] : handler.m_varBinBounds) {
    if (!first) os << ", ";
    first = false;
    os << BTaggingToolUtil::getVariableName(vType) << ": [" << bounds.lowerBound << ", " << bounds.upperBound << "]";
  }
  return os;
}

bool BTaggingBoundsHandler::operator<(const BTaggingBoundsHandler& o) const {
  // Ordering of handlers loop over variables as stored in the map
  // and let the first variable whose bounds differ decide of the ordering

  // Make sure maps have the same size
  if (m_varBinBounds.size() != o.m_varBinBounds.size()) {
    throw std::logic_error("Cannot compare handlers with a different number of variables: " + this->to_string() + " versus " + o.to_string() );
  }
  // Loop over the different variables bounds and compare
  for (const auto& [vType, vBounds1] : m_varBinBounds) {
    // Make sure the other handler has the same variable
    const auto it = o.m_varBinBounds.find(vType);
    if (it == o.m_varBinBounds.end()) {
      throw std::logic_error("Cannot compare handlers: variable '" + BTaggingToolUtil::getVariableName(vType) +
                             "' missing in the other handler");
    }
    const varBounds& vBounds2 = it->second;
    // First variable whose bounds differ decides the ordering
    if (!(vBounds1 == vBounds2)) return vBounds1 < vBounds2;
  }
  return false;  // all bounds equal
}

bool BTaggingBoundsHandler::overlaps(const BTaggingBoundsHandler& o) const {
  // Two handlers that are N-dimensional bins overlap if
  // for every variable their bounds overlap
  // If for one variable there is no overlapping bounds then the bins are not overlapping

  // Make sure maps have the same size
  if (m_varBinBounds.size() != o.m_varBinBounds.size()) {
    throw std::logic_error("Cannot check overlap of handlers with a different number of variables: " + this->to_string() + " versus " + o.to_string() );
  }
  // Loop over the different variables bounds and check the overlap
  for (const auto& [vType, vBounds1] : m_varBinBounds) {
    // Make sure the other handler has the same variable
    const auto it = o.m_varBinBounds.find(vType);
    if (it == o.m_varBinBounds.end()) {
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
