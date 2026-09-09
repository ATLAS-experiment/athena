/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MonitorChain.h"
#include "../counters/CounterChain.h"

MonitorChain::MonitorChain(const std::string& name, const MonitoredRange* parent)
  : MonitorBase(name, parent) {
}

StatusCode MonitorChain::newEvent(const CostData& data, const float weight) {

  const std::vector<TrigCompositeUtils::AlgToChainTool::ChainInfo>& seededChains = data.seededChains();
  for (size_t i = 0; i < seededChains.size(); ++i){
    std::string chainName = seededChains[i].name;
    if (!counterExists(chainName)){
      // Create a new counter using specialized constructor in order to pass number of bins for some of the histograms
      m_counters.insert( std::make_pair(chainName, newCounter(chainName)) );
    } 
    ATH_CHECK( getCounter(chainName)->newEvent(data, i, weight) );
  }

  return StatusCode::SUCCESS;
}

std::unique_ptr<CounterBase> MonitorChain::newCounter(const std::string& name) {
  return std::make_unique<CounterChain>(name, this);
} 

