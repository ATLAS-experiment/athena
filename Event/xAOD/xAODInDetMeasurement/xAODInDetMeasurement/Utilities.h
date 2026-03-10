/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_INDETMEASUREMENT_UTILITIES_H
#define XAOD_INDETMEASUREMENT_UTILITIES_H

#include "AthContainers/AuxElement.h"
#include <vector>

namespace xAOD::xAODInDetMeasurement::Utilities {
  
  float computeTotalCharge( const SG::AuxElement& cluster);
  int computeTotalToT( const SG::AuxElement& cluster);
}

#endif 
