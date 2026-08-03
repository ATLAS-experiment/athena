/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MonitorAlgorithm.h"
#include "../counters/CounterAlgorithm.h"
#include <cstdint>
#include <algorithm>

MonitorAlgorithm::MonitorAlgorithm(const std::string& name, const MonitoredRange* parent)
  : MonitorBase(name, parent) {
}

StatusCode MonitorAlgorithm::newEvent(const CostData& data, const float weight) {
  const std::string slotStr{"slot"};
  const std::string algStr{"alg"};
  const std::string ALGStr{"ALG"};
  for (const xAOD::TrigComposite* tc : data.costCollection()) {
    const uint32_t slot = tc->getDetail<uint32_t>(slotStr);
    if (slot != data.onlineSlot()) {
      continue; // When monitoring the master slot, this Monitor ignores algs running in different slots 
    }
    const uint32_t nameHash = tc->getDetail<TrigConf::HLTHash>(algStr);
    std::string name = TrigConf::HLTUtils::hash2string(nameHash, ALGStr);
    std::replace(name.begin(), name.end(), ':', '_');
    ATH_CHECK( getCounter(name)->newEvent(data, tc->index(), weight) );
  }

  return StatusCode::SUCCESS;
}


std::unique_ptr<CounterBase> MonitorAlgorithm::newCounter(const std::string& name) {
  return std::make_unique<CounterAlgorithm>(name, this);
} 
