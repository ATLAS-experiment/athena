/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MonitorROS.h"
#include "../counters/CounterROS.h"

#include <algorithm>

MonitorROS::MonitorROS(const std::string& name, const MonitoredRange* parent)
  : MonitorBase(name, parent) {
}


StatusCode MonitorROS::newEvent(const CostData& data, const float weight) {

  if (data.rosCollection().empty()){
    ATH_MSG_DEBUG("The ROS collection is empty!");
  }

  for (const xAOD::TrigComposite* tc : data.rosCollection()) {
    auto robIds = tc->getDetail<std::vector<uint32_t>>("robs_id");
    // Create set of unique ROS for this request
    std::set<std::string> rosPerRequest;
    for (uint32_t robId : robIds) {
      if (data.costROSData().getROSForROB(robId).empty()){
        ATH_MSG_WARNING("ROS for ROB 0x" << std::hex << robId << " is missing");
        continue;
      }
      std::string rosForROB = data.costROSData().getROSForROB(robId);
      if (!rosForROB.empty()){
        rosPerRequest.insert(std::move(rosForROB));
      }
    }

    for (const std::string& rosName : rosPerRequest) {
      if (!counterExists(rosName)){
        // Create a new counter using specialized constructor in order to pass number of bins for some of the histograms
        unsigned nRobs = data.costROSData().getROBForROS(rosName).size(); // Number of all possible ROBs for this ROS
        m_counters.insert( std::make_pair(rosName, newCounter(rosName, nRobs)) );
      } 

      ATH_CHECK( getCounter(rosName)->newEvent(data, tc->index(), weight) );
    }
  }

  return StatusCode::SUCCESS;
}


std::unique_ptr<CounterBase> MonitorROS::newCounter(const std::string& name) {
  return std::make_unique<CounterROS>(name, this);
} 

std::unique_ptr<CounterBase> MonitorROS::newCounter(const std::string& name, unsigned nRobs) {
  return std::make_unique<CounterROS>(name, nRobs, this);
} 