/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODTrigger/TrigCompositeContainer.h"
#include "TrigDataAccessMonitoring/ROBDataMonitor.h"

#include "CounterSequence.h"
#include <cstdint>


CounterSequence::CounterSequence(const std::string& name, const MonitorBase* parent) 
  : CounterBase(name, parent)
{
  regHistogram("Sequence_perEvent", "Sequnece calls/Event;Sequence call;Events", VariableType::kPerEvent, kLinear, -0.5, 49.5);
  regHistogram("AlgCalls_perEvent", "Algorithm Calls/Event;Calls;Events", VariableType::kPerEvent, kLinear, -0.5, 499.5, 100);
  regHistogram("Time_perCall", "CPU Time/Call;Time [ms];Calls", VariableType::kPerCall);
  regHistogram("Time_perEvent", "CPU Time/Event;Time [ms];Events", VariableType::kPerEvent);
  regHistogram("Request_perEvent", "Number of requests/Event;Number of requests;Events", VariableType::kPerEvent, LogType::kLinear, -0.5, 299.5, 300);
}


StatusCode CounterSequence::newEvent(const CostData& data, size_t index, const float weight) {

  ATH_CHECK( increment("Sequence_perEvent", weight) );
  float viewTime = 0;
  // Monitor algorithms associated with sequence name
  const std::string slotStr{"slot"};
  const std::string startStr{"start"};
  const std::string stopStr{"stop"};
  for (const size_t algIndex :  data.sequencersMap().at(getName()).at(index)){
    
    const xAOD::TrigComposite* alg = data.costCollection().at(algIndex);
    const uint32_t slot = alg->getDetail<uint32_t>(slotStr);
    if (slot != data.onlineSlot()) {
      continue; // When monitoring the master slot, this Monitor ignores algs running in different slots 
    }

    ATH_CHECK( increment("AlgCalls_perEvent", weight) );

    const uint64_t start = alg->getDetail<uint64_t>(startStr); // in mus
    const uint64_t stop  = alg->getDetail<uint64_t>(stopStr); // in mus
    const float cpuTime = timeToMilliSec(start, stop);
    ATH_CHECK( fill("Time_perEvent", cpuTime, weight) );
    viewTime += cpuTime;

  }

  ATH_CHECK( fill("Time_perCall", viewTime, weight) );

  return StatusCode::SUCCESS;
}
