/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGCOSTMONITOR_TRIGCOSTALGORITHMPAYLOAD_H
#define TRIGCOSTMONITOR_TRIGCOSTALGORITHMPAYLOAD_H 1

#include "TrigTimeAlgs/TrigTimeStamp.h"
#include <string>

/** 
 * @class AlgorithmPayload
 * @brief Small structure wrap the various values stored for an algorithm
 * just before it starts to execute.
 */
struct AlgorithmPayload {
  TrigTimeStamp m_algStartTime;
  std::thread::id m_algThreadID;
  int32_t m_algROIID = 0;
  uint32_t m_slot = 0;
};


#endif // TRIGCOSTMONITOR_TRIGCOSTALGORITHMPAYLOAD_H
