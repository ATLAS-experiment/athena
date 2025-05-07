/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelConditionsData/PixelClusterdEdxCondData.h"
#include <vector>
#include <tuple>


PixelClusterdEdxCondData::PixelClusterdEdxCondData() : m_var() {
  }

PixelClusterdEdxCondData::~PixelClusterdEdxCondData() = default;

float PixelClusterdEdxCondData::getVar(const std::tuple<int,int,int>& module_coordinates) const {
  if (m_configStatus == 1){
    for (const auto& module_sf_pair : m_var) {
      auto module_location = std::get<0>(module_sf_pair);
      float sf_value = std::get<1>(module_sf_pair);
      if (module_location == module_coordinates) {
        if (sf_value == -1.0) {
          //ATH_MSG_WARNING("No scale factor for this module. Returning 1 so that no scaling is done.");
          return 1.0;
          }
        return sf_value;
        } 
      }
    //ATH_MSG_WARNING("No such module! Returning 1.");
    return 1.0;
    }
    else if (m_configStatus == 0){return 1.0;}
    else {
    //ATH_MSG_WARNING("Bad config flag? -- Rebecca");
    return -999.0;}
  }

void PixelClusterdEdxCondData::setVar(const std::vector<std::tuple<std::tuple<int,int,int>,float>>& value) {
  m_var = value;
  return;
  }

void PixelClusterdEdxCondData::setConfig(const int & value) {
  m_configStatus = value;
  }
