/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODBTaggingEfficiency/BTaggingToolUtil.h"
#include <limits>
#include <stdexcept>
#include <cmath> //std::abs
#include <algorithm> //std::max

float BTaggingToolUtil::getExtendedFloat(const nlohmann::json& config) {
  if (config.is_string() && (config.get<std::string>() == "inf" || config.get<std::string>() == "+inf")) {
    // Positive infinity: it is greater than every finite float
    return std::numeric_limits<float>::infinity();
  } else if (config.is_string() && config.get<std::string>() == "-inf") {
    // Negative infinity: it is lower than every finite float
    return -std::numeric_limits<float>::infinity();
  }
  else if (config.is_string() && (config.get<std::string>() == "fmax" || config.get<std::string>() == "+fmax")) {
    // Largest finite positive float number
    return std::numeric_limits<float>::max();
  }
  else if (config.is_string() && config.get<std::string>() == "-fmax") {
    // Lowest finite negative float number possible 
    return std::numeric_limits<float>::lowest();
  }

  return config.get<float>();
}

std::string BTaggingToolUtil::getExtendedString(const nlohmann::json& config) {
  if (config.is_string()){
    return config.get<std::string>();
  }
  
  return config.dump();
}

bool BTaggingToolUtil::approxEqual(const float a, const float b, const float eps) {
  // Exact equality also covers equal infinities e.g. two "inf" upper bounds
  if (a == b) return true;
  // An infinite value is never approximately equal to a different value
  // otherwise |inf - x| <= eps * inf would be true for any finite x
  if (!std::isfinite(a) || !std::isfinite(b)) return false;
  return std::abs(a - b) <= eps * std::max(std::abs(a), std::abs(b));
}


BTaggingToolUtil::varType BTaggingToolUtil::getVariableType(const std::string& varName)
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
    throw std::runtime_error("Variable '" + varName + "' not defined in BTaggingToolUtil.");
    
  }
}

std::string BTaggingToolUtil::getVariableName(const varType vType)
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
    throw std::runtime_error("Variable type " + std::to_string(static_cast<int>(vType)) + " not defined in BTaggingToolUtil.");
    
  }
}

