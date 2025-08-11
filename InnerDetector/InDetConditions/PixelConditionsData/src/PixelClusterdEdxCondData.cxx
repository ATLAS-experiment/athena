/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "PixelConditionsData/PixelClusterdEdxCondData.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include <vector>
#include <tuple>
#include <stdexcept>

PixelClusterdEdxCondData::PixelClusterdEdxCondData() : m_clusterScaleFactors(), m_configFlag() {
  }

PixelClusterdEdxCondData::~PixelClusterdEdxCondData() = default;

float PixelClusterdEdxCondData::getScaleFactor(const IdentifierHash& input_wafer_hashID) const {
//Config status == True
//Iterate through vector of tuples to find correct SF.
//Config status == False
//Do nothing, always return 1.
  if(m_configFlag) { 
    for (const auto& hash_sf_pair : m_clusterScaleFactors) {
      IdentifierHash module_hash = std::get<0>(hash_sf_pair);
      float sf_value = std::get<1>(hash_sf_pair);
      if (input_wafer_hashID == module_hash) {
        return sf_value;
        } 
      }
    throw std::invalid_argument("Input module coordinates in PixelToTPIDTool is invalid. Cannot find scale factor.");
    return -999.0;
  }
  else if (!m_configFlag) {
    float sf_default = 1.0;
    return sf_default;
  }
  else {
  throw std::invalid_argument("ERROR configuration flag for PixelClusterdEdxCondData is invalid!");
  return -999.0;
  }
}

void PixelClusterdEdxCondData::setScaleFactors(const std::vector<std::tuple<IdentifierHash,float>>& inputScaleFactors) {
  m_clusterScaleFactors = inputScaleFactors;
  return;
  }

void PixelClusterdEdxCondData::setConfig(const bool & flagValue) {
  m_configFlag = flagValue;
  return;
  }
