/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelConditionsData/PixelClusterdEdxCondData.h"

/*Read in json file, store as a map that takes [run, eta, layer, barrel/endcap/etc] = scaleFactor*/


/*Get Calibration Constants Method*/
std::vector<float> PixelClusterdEdxCondData::GetConstants(){
  /*Will fill this with more flexible method of returning constants later*/
  std::vector<float> constants = m_Map;
  return constants
}
/*Set Calibration Constants Method*/
void PixelClusterdEdxCondData::SetConstants(const std::vector<float> &constants){
  /*Will fill this with method to extract from json file later*/
  m_Map = {1,2,3,4,5}; 
}

