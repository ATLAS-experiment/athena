/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FwdAFPCountMonitoringAlg.h"

#include <AthenaBaseComps/AthMsgStreamMacros.h>
#include <AthenaMonitoringKernel/MonitoredCollection.h>
#include "TrigDecisionTool/TrigDecisionTool.h"

FwdAFPCountMonitoringAlg::FwdAFPCountMonitoringAlg(const std::string& name, ISvcLocator* pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator) {}

StatusCode FwdAFPCountMonitoringAlg::initialize() {
  return AthMonitorAlgorithm::initialize();
}

StatusCode FwdAFPCountMonitoringAlg::fillHistograms([[maybe_unused]] const EventContext& context) const {
  using namespace Monitored;

  const auto& trigDecTool = getTrigDecisionTool();

  std::vector<std::string> passedChains{};
  std::copy_if(m_chains.begin(), m_chains.end(), std::back_inserter(passedChains),
               [&trigDecTool](const auto& chain) { return trigDecTool->isPassed(chain, TrigDefs::requireDecision); });

  if (passedChains.size() > 0) {
    ATH_MSG_DEBUG("Passed chains (" << passedChains.size() << "):");

    for (const auto& chain : passedChains) {
      ATH_MSG_DEBUG('\t' << chain);
    }
  }

  auto counts = Collection("counts", passedChains);
  fill("AFPCount", counts);

  return StatusCode::SUCCESS;
}
