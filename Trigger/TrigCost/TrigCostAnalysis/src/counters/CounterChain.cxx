/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODTrigger/TrigCompositeContainer.h"
#include "TrigDataAccessMonitoring/ROBDataMonitor.h"

#include "CounterChain.h"
#include <cstdint>


CounterChain::CounterChain(const std::string& name, const MonitorBase* parent) 
  : CounterBase(name, parent), m_isInitialized(false)
{
  regHistogram("Group_perCall", "Chain group/Call;Group;Calls", VariableType::kPerCall, kLinear, -0.5, 9.5, 10);
  regHistogram("Chain_perEvent", "Chain calls/Event;Chain call;Events", VariableType::kPerEvent, kLinear, -0.5, 49.5);
  regHistogram("AlgCalls_perEvent", "Algorithm Calls/Event;Calls;Events", VariableType::kPerEvent, kLinear, -0.5, 999.5, 100);
  regHistogram("Time_perCall", "CPU Time/Call;Time [ms];Calls", VariableType::kPerCall, kLog, 0.01, 100000);
  regHistogram("Time_perEvent", "CPU Time/Event;Time [ms];Events", VariableType::kPerEvent);
  regHistogram("UniqueTime_perCall", "Unique CPU Time/Call;Time [ms];Calls", VariableType::kPerCall, kLog, 0.01, 100000);
  regHistogram("ChainPassed_perEvent", "Passed chain/Event;Passsed;Events", VariableType::kPerEvent, kLinear, -0.5, 1.5, 2);
  regHistogram("Request_perEvent", "Number of requests/Event;Number of requests;Events", VariableType::kPerEvent, LogType::kLinear, -0.5, 299.5, 300);
  regHistogram("NetworkRequest_perEvent", "Number of network requests/Event;Number of requests;Events", VariableType::kPerEvent, LogType::kLinear, -0.5, 149.5, 150);
  regHistogram("CachedROBSize_perEvent", "Cached ROB Size/Event;ROB size;Events", VariableType::kPerEvent, LogType::kLinear, 0, 1024, 50);
  regHistogram("NetworkROBSize_perEvent", "Network ROB Size/Event;ROB size;Events", VariableType::kPerEvent, LogType::kLinear, 0, 1024, 50);
  regHistogram("RequestTime_perEvent", "ROB Elapsed Time/Event;Elapsed Time [ms];Events", VariableType::kPerEvent);
}


StatusCode CounterChain::newEvent(const CostData& data, size_t index, const float weight) {

  ATH_CHECK( increment("Chain_perEvent", weight) );

  if (!m_isInitialized && variableExists("ROSRequests_perEvent")) {
    // Set histograms labels
    for (const auto& rosToRobPair : data.costROSData().getROStoROBMap()) {
      int binForROS = data.costROSData().getBinForROS(rosToRobPair.first) + 1;

    // Fill the bins with groups and add the labels
    int bin = 1;
    for (const std::string& group : data.seededChains()[index].groups){
      ATH_CHECK( getVariable("Group_perCall").setBinLabel(bin, group) );
      ATH_CHECK( getVariable("Group_perCall").fill(group, weight) );
      ++bin;
    }

    m_isInitialized = true;
  }

  if (data.seededChains()[index].isPassRaw){
    ATH_CHECK( increment("ChainPassed_perEvent", weight) );
  }

  // Monitor algorithms associated with chain name
  if (!data.chainToAlgMap().count(getName())) return StatusCode::SUCCESS;

  const std::string slotStr{"slot"};
  const std::string startStr{"start"};
  const std::string stopStr{"stop"};
  for (const size_t algIndex : data.chainToAlgMap().at(getName())){
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
    ATH_CHECK( fill("Time_perCall", cpuTime, weight) );

  // Monitor unique algorithms associated with chain name
  if (!data.chainToUniqAlgMap().count(getName())) return StatusCode::SUCCESS;

  for (const size_t algIndex : data.chainToUniqAlgMap().at(getName())){
    const xAOD::TrigComposite* alg = data.costCollection().at(algIndex);
    const uint32_t slot = alg->getDetail<uint32_t>(slotStr);
    if (slot != data.onlineSlot()) {
      continue;
    }
    const uint64_t start = alg->getDetail<uint64_t>(startStr); // in mus
    const uint64_t stop  = alg->getDetail<uint64_t>(stopStr); // in mus
    const float cpuTime = timeToMilliSec(start, stop);

    ATH_CHECK( fill("UniqueTime_perCall", cpuTime, weight) );
  }

  return StatusCode::SUCCESS;
}
