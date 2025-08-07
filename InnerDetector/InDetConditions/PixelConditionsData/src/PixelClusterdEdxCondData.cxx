/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelConditionsData/PixelClusterdEdxCondData.h"
#include <vector>
#include <tuple>
#include <stdexcept>

PixelClusterdEdxCondData::PixelClusterdEdxCondData() : m_var() {
  }

PixelClusterdEdxCondData::~PixelClusterdEdxCondData() = default;

float PixelClusterdEdxCondData::getVar(const std::tuple<int,int,int>& module_coordinates) const {
//Config status == True
//Iterate through vector of tuples to find correct SF.

//Config status == False
//Do nothing, always return 1.
  if(m_configFlag) { 
    for (const auto& module_sf_pair : m_var) {
      std::tuple<int,int,int> module_location = std::get<0>(module_sf_pair);
      float sf_value = std::get<1>(module_sf_pair);
      if (module_location == module_coordinates) {return sf_value;} 
      else {
        throw std::invalid_argument("Input module coordinates in PixelToTPIDTool is invalid. Cannot find scale factor.");
        }
      }
  }
  if(!m_configFlag) {
    float sf_default = 1.0;
    return sf_default;
  }
}

void PixelClusterdEdxCondData::setVar(const std::vector<std::tuple<std::tuple<int,int,int>,float>>& value) {
  m_var = value;
  return;
  }

void PixelClusterdEdxCondData::setConfig(const bool & value) {
  m_configFlag = value;
  return;
  }
