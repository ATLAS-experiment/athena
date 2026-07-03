/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MonitorAlgorithmClass.h"
#include "../counters/CounterAlgorithmClass.h"
#include <cstdint>

MonitorAlgorithmClass::MonitorAlgorithmClass(const std::string& name, const MonitoredRange* parent)
  : MonitorBase(name, parent) {
}

StatusCode MonitorAlgorithmClass::newEvent(const CostData& data, const float weight) {
  const std::string slotStr{"slot"};
  const std::string algStr{"alg"};
  for (const xAOD::TrigComposite* tc : data.costCollection()) {
    const uint32_t slot = tc->getDetail<uint32_t>(slotStr);
    if (slot != data.onlineSlot()) {
      continue; // When monitoring the master slot, this Monitor ignores algs running in different slots 
    }
    const uint32_t nameHash = tc->getDetail<TrigConf::HLTHash>(algStr);
    const std::string& name = data.algNameToClassType(nameHash);
    ATH_CHECK( getCounter(name)->newEvent(data, tc->index(), weight) );
  }

  return StatusCode::SUCCESS;
}


std::unique_ptr<CounterBase> MonitorAlgorithmClass::newCounter(const std::string& name) {
  return std::make_unique<CounterAlgorithmClass>(name, this);
} 
