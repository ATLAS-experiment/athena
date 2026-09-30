/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak

#include <EventBookkeeperTools/FilterReporter.h>
#include <TriggerAnalysisAlgorithms/TrigEventSelectionAlg.h>
#include <TriggerAnalysisAlgorithms/TrigChainNameHelpers.h>
#include <AsgDataHandles/ReadHandle.h>

CP::TrigEventSelectionAlg::TrigEventSelectionAlg(const std::string &name,
                                             ISvcLocator *svcLoc)
  : EL::AnaAlgorithm(name, svcLoc),
    m_trigDecisionTool("Trig::TrigDecisionTool/TrigDecisionTool")
{
  declareProperty("tool", m_trigDecisionTool, "trigger decision tool");
}

StatusCode CP::TrigEventSelectionAlg::initialize()
{
  if (m_trigList.empty()) {
    ATH_MSG_ERROR("A list of triggers needs to be provided");
    return StatusCode::FAILURE;
  }

  ANA_CHECK(m_trigDecisionTool.retrieve());

  if (!m_selectionDecoration.empty()) {
    const std::string prefix{m_selectionDecoration.value() + "_"};
    for (const std::string &chain : m_trigList) {
      m_selectionAccessors.emplace_back( prefix + sanitizeTriggerChainName(chain));
    }
  }

  ANA_CHECK (m_eventInfoKey.initialize(!m_selectionDecoration.empty()));
  ANA_CHECK (m_filterParams.initialize());

  return StatusCode::SUCCESS;
}

StatusCode CP::TrigEventSelectionAlg::execute(const EventContext& ctx)
{
  FilterReporter filter (m_filterParams, m_noFilter.value());

  const xAOD::EventInfo *evtInfo = nullptr;
  if (!m_selectionDecoration.empty()) {
    SG::ReadHandle<xAOD::EventInfo> evtInfoHandle(m_eventInfoKey, ctx);
    ANA_CHECK(evtInfoHandle.isValid());
    evtInfo = evtInfoHandle.cptr();
  }

  for (size_t i = 0; i < m_trigList.size(); i++) {
    bool trigPassed = m_noL1.value()
           ? m_trigDecisionTool->isPassed(m_trigList[i], TrigDefs::requireDecision)
           : m_trigDecisionTool->isPassed(m_trigList[i]);
    if (evtInfo) {
      m_selectionAccessors[i](*evtInfo) = trigPassed;
    }
    if (trigPassed)
      filter.setPassed (true);
  }

  return StatusCode::SUCCESS;
}

StatusCode CP::TrigEventSelectionAlg::finalize()
{
  ANA_MSG_INFO (m_filterParams.summary());

  return StatusCode::SUCCESS;
}
