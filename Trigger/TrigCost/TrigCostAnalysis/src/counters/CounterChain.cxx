/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODTrigger/TrigCompositeContainer.h"
#include "TrigDataAccessMonitoring/ROBDataMonitor.h"

#include "CounterChain.h"


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

CounterChain::CounterChain(const std::string& name, unsigned nRos, const MonitorBase* parent) 
  : CounterChain(name, parent)
{
  regTProfile("ROSRequests_perEvent", "Number of ROS requests;ROS names;Numer of requests to ROS per event", VariableType::kPerCall, LogType::kLinear, 0, nRos, nRos);
  regTProfile("NetworkROSRequests_perEvent", "Number of network ROS requests;ROS names;Numer of requests to ROS over the network per event", VariableType::kPerCall, LogType::kLinear, 0, nRos, nRos);
  regTProfile("ROBRequestsPerROS_perEvent", "Number of ROBs per ROS requests;ROS names;Numer of ROBs requested per request", VariableType::kPerCall, LogType::kLinear, 0, nRos, nRos);
  regTProfile("ROBRequestsPerROSPerEvent_perEvent", "Number of ROBs per ROS requests per event;ROS names;Numer of ROBs requested by ROS per event", VariableType::kPerCall, LogType::kLinear, 0, nRos, nRos);
}


StatusCode CounterChain::newEvent(const CostData& data, size_t index, const float weight) {

  ATH_CHECK( increment("Chain_perEvent", weight) );

  if (!m_isInitialized && variableExists("ROSRequests_perEvent")) {
    // Set histograms labels
    for (const auto& rosToRobPair : data.costROSData().getROStoROBMap()) {
      int binForROS = data.costROSData().getBinForROS(rosToRobPair.first) + 1;
      ATH_CHECK( getVariable("ROSRequests_perEvent").setBinLabel(binForROS, rosToRobPair.first));
      ATH_CHECK( getVariable("NetworkROSRequests_perEvent").setBinLabel(binForROS, rosToRobPair.first));
      ATH_CHECK( getVariable("ROBRequestsPerROS_perEvent").setBinLabel(binForROS, rosToRobPair.first));
      ATH_CHECK( getVariable("ROBRequestsPerROSPerEvent_perEvent").setBinLabel(binForROS, rosToRobPair.first));
    }

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

  std::map<std::string, int> nRosPerEvent; // Accumulate how many times ROS was requested in a request per this event
  std::map<std::string, int> nNetworkRosPerEvent; // Accumulate how many times ROS was requested in a request per this event
  std::map<std::string, int> nRobsPerRosPerEvent; // Accumulate how many ROBs ROS requested per this event
  for (const size_t algIndex : data.chainToAlgMap().at(getName())){
    const xAOD::TrigComposite* alg = data.costCollection().at(algIndex);
    const uint32_t slot = alg->getDetail<uint32_t>("slot");
    if (slot != data.onlineSlot()) {
      continue; // When monitoring the master slot, this Monitor ignores algs running in different slots 
    }

    ATH_CHECK( increment("AlgCalls_perEvent", weight) );

    const uint64_t start = alg->getDetail<uint64_t>("start"); // in mus
    const uint64_t stop  = alg->getDetail<uint64_t>("stop"); // in mus
    const float cpuTime = timeToMilliSec(start, stop);
    ATH_CHECK( fill("Time_perEvent", cpuTime, weight) );
    ATH_CHECK( fill("Time_perCall", cpuTime, weight) );

    // Monitor data requests
    if (!data.algToRequestMap().count(algIndex)) continue;

    for (size_t requestIdx : data.algToRequestMap().at(algIndex)) {
      const xAOD::TrigComposite* request = data.rosCollection().at(requestIdx);
      const std::vector<uint32_t> robIdsPerRequest = request->getDetail<std::vector<uint32_t>>("robs_id");
      const std::vector<unsigned> robs_history = request->getDetail<std::vector<unsigned>>("robs_history");
      const std::vector<uint32_t> robs_size = request->getDetail<std::vector<uint32_t>>("robs_size");

      std::map<std::string, int> nRobsPerRosPerRequest; // Accumulate how many ROBs ROS requested per request

      bool networkRequestIncremented = false;
      std::set<std::string> requestedROSes;
      std::set<std::string> requestedNetworkROSes;
      for (size_t i = 0; i < robs_size.size(); ++i) {
        // ROB request was fetched over the network
        if (robs_history[i] == robmonitor::RETRIEVED) {
          // size is stored in words, should be in kilobytes
          ATH_CHECK( fill("NetworkROBSize_perEvent", robs_size[i] / 500., weight) );
          networkRequestIncremented = true;
        }
        // ROB request was cached
        else if (robs_history[i] == robmonitor::HLT_CACHED || robs_history[i] == robmonitor::DCM_CACHED) {
          ATH_CHECK( fill("CachedROBSize_perEvent", robs_size[i] / 500., weight) );
        }

        uint32_t robId = robIdsPerRequest[i];
        if (variableExists("ROBRequestsPerROS_perEvent")){
          std::string rosForROB = data.costROSData().getROSForROB(robId);
          if (!rosForROB.empty()){
            requestedROSes.insert(rosForROB);
            nRobsPerRosPerRequest[rosForROB] += 1;
            nRobsPerRosPerEvent[rosForROB] += 1;

            if (robs_history[i] == robmonitor::RETRIEVED) {
              requestedNetworkROSes.insert(std::move(rosForROB));
            }
          }
        }
      }

      // Save number of ROBs per ROS per request
      for (const auto& robPerRosPair : nRobsPerRosPerRequest) {
        ATH_CHECK( fill("ROBRequestsPerROS_perEvent", data.costROSData().getBinForROS(robPerRosPair.first), robPerRosPair.second, weight) );
      }

      // Save the requested ROSes per request
      for (const std::string& rosName : requestedROSes){
          nRosPerEvent[rosName] += 1;
      }
    
      for (const std::string& rosName : requestedNetworkROSes){
          nNetworkRosPerEvent[rosName] += 1;
      }

      ATH_CHECK( increment("Request_perEvent", weight) );

      if (networkRequestIncremented) {
        ATH_CHECK( increment("NetworkRequest_perEvent", weight) );
      }

      const float rosTime = timeToMilliSec(request->getDetail<uint64_t>("start"), request->getDetail<uint64_t>("stop"));
      ATH_CHECK( fill("RequestTime_perEvent", rosTime, weight) );
    }
  }

  // Save the requested ROSes per event
  for (const auto& robPerRosPair : nRosPerEvent) {
    ATH_CHECK( fill("ROSRequests_perEvent", data.costROSData().getBinForROS(robPerRosPair.first), robPerRosPair.second, weight) );
  }

  for (const auto& robPerRosPair : nNetworkRosPerEvent) {
    ATH_CHECK( fill("NetworkROSRequests_perEvent", data.costROSData().getBinForROS(robPerRosPair.first), robPerRosPair.second, weight) );
  }

  // Save the number of ROBs per ROS per event
  for (const auto& robPerRosPair : nRobsPerRosPerEvent) {
    ATH_CHECK( fill("ROBRequestsPerROSPerEvent_perEvent", data.costROSData().getBinForROS(robPerRosPair.first), robPerRosPair.second, weight) );
  }

  // Monitor unique algorithms associated with chain name
  if (!data.chainToUniqAlgMap().count(getName())) return StatusCode::SUCCESS;

  for (const size_t algIndex : data.chainToUniqAlgMap().at(getName())){
    const xAOD::TrigComposite* alg = data.costCollection().at(algIndex);
    const uint32_t slot = alg->getDetail<uint32_t>("slot");
    if (slot != data.onlineSlot()) {
      continue;
    }
    const uint64_t start = alg->getDetail<uint64_t>("start"); // in mus
    const uint64_t stop  = alg->getDetail<uint64_t>("stop"); // in mus
    const float cpuTime = timeToMilliSec(start, stop);

    ATH_CHECK( fill("UniqueTime_perCall", cpuTime, weight) );
  }

  return StatusCode::SUCCESS;
}
