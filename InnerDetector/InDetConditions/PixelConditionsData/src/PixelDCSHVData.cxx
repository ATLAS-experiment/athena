/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelConditionsData/PixelDCSHVData.h"

namespace{
  bool beyondLimits(const std::pair<float, float> & limits, float value){
    return (value<limits.first or value>limits.second);
  }
}

void 
PixelDCSHVData::defaultVoltage(float v){
  m_defaultVoltage = v;
}

void 
PixelDCSHVData::setChannelToDefault(int chanNum){
  m_biasVoltage[chanNum] = m_defaultVoltage;
}

void 
PixelDCSHVData::useDefault(bool b){
  m_alwaysUseDefault = b;
}



void 
PixelDCSHVData::setBiasVoltage(const int chanNum, const float value) {
  m_biasVoltage[chanNum] = beyondLimits(m_valueLimits, value) ? m_defaultVoltage : value;
}

float 
PixelDCSHVData::getBiasVoltage(const int chanNum) const {
  if (m_alwaysUseDefault) return m_defaultVoltage;
  auto itr = m_biasVoltage.find(chanNum);
  if (itr!=m_biasVoltage.end()) { return itr->second; }
  return 0;
}

