/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelConditionsData/ITkPixSimulationParameters.h"
#include <iostream>
#include <format>

std::ostream & 
operator<<(std::ostream & os, const ITkPixSimulationParameters & c){
  os<<std::format("ToT Threshold = {}; XTalk = {}; p(Disable) = {}; noiseOcc. = {}; ", 
    c.totThreshold(), c.crossTalk(), c.disableProbability(), c.noiseOccupancy());
  os<<"Noise shape: ";
  
  for(const auto& v=c.noiseShape(); const auto & f:v) os<<f<<" ";
  return os;
}