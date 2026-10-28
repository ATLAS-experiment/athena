/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODBTaggingEfficiency/BTaggingToolUtil.h"

float BTaggingToolUtil::getExtendedFloat(const nlohmann::json& config) {
  if (config.is_string() && config.get<std::string>() == "inf") {
    return std::numeric_limits<float>::infinity();
  } else if (config.is_string() && config.get<std::string>() == "-inf") {
    return -std::numeric_limits<float>::infinity();
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
    return varType::unknown;
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
    return "unknown";
  }
}

