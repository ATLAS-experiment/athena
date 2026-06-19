/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "DVInferenceAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"

namespace MuonML {

StatusCode DVInferenceAlg::initialize() {
  ATH_CHECK(m_inferenceTool.retrieve());
  const bool decorateEventInfo = m_decorateEventInfo;
  ATH_CHECK(m_eventInfoKey.initialize(decorateEventInfo));
  ATH_CHECK(m_scoreDecorKey.initialize(decorateEventInfo));
  ATH_CHECK(m_rawDecorKey.initialize(decorateEventInfo));
  ATH_CHECK(m_passDecorKey.initialize(decorateEventInfo));
  ATH_CHECK(m_nNodesDecorKey.initialize(decorateEventInfo));
  ATH_CHECK(m_nEdgesDecorKey.initialize(decorateEventInfo));

  m_thresholdModeName = m_thresholdMode;
  if (m_thresholdModeName != "score" && m_thresholdModeName != "raw") {
    ATH_MSG_ERROR("ThresholdMode must be either 'score' or 'raw', got " << m_thresholdModeName);
    return StatusCode::FAILURE;
  }
  m_useRawThreshold = (m_thresholdModeName == "raw");
  const float scoreThreshold = m_scoreThreshold;
  ATH_MSG_INFO("Initialized DVInferenceAlg with ScoreThreshold=" << scoreThreshold
               << ", ThresholdMode=" << m_thresholdModeName
               << ", DecorateEventInfo=" << decorateEventInfo);
  if (decorateEventInfo) {
    ATH_MSG_INFO("EventInfo DV decorations are enabled for validation/debug output only");
  }
  return StatusCode::SUCCESS;
}

StatusCode DVInferenceAlg::execute(const EventContext& ctx) const {
  DVInferenceResult result{};
  ATH_CHECK(m_inferenceTool->inferEvent(ctx, result));

  if (!result.valid) {
    ATH_MSG_WARNING("DV event classifier did not produce a finite score for event "
                    << ctx.eventID().event_number());
  }

  const float decisionValue = m_useRawThreshold ? result.rawOutput : result.probability;
  const float cutValue = m_scoreThreshold;
  const bool pass = result.valid && decisionValue >= cutValue;
  if (m_printEveryEvent) {
    ATH_MSG_INFO("DV event classifier: event=" << ctx.eventID().event_number()
                 << " score=" << result.probability
                 << " raw=" << result.rawOutput
                 << " decisionValue=" << decisionValue
                 << " cutValue=" << cutValue
                 << " thresholdMode=" << m_thresholdModeName
                 << " pass=" << pass
                 << " nodes=" << result.nNodes
                 << " edges=" << result.nEdges);
  } else {
    ATH_MSG_DEBUG("DV event classifier: event=" << ctx.eventID().event_number()
                  << " score=" << result.probability
                  << " raw=" << result.rawOutput
                  << " decisionValue=" << decisionValue
                  << " cutValue=" << cutValue
                  << " thresholdMode=" << m_thresholdModeName
                  << " pass=" << pass
                  << " nodes=" << result.nNodes
                  << " edges=" << result.nEdges);
  }

  if (m_decorateEventInfo) {
    const xAOD::EventInfo* eventInfo{};
    ATH_CHECK(SG::get(eventInfo, m_eventInfoKey, ctx));

    SG::WriteDecorHandle<xAOD::EventInfo, float> scoreDecor{m_scoreDecorKey, ctx};
    SG::WriteDecorHandle<xAOD::EventInfo, float> rawDecor{m_rawDecorKey, ctx};
    SG::WriteDecorHandle<xAOD::EventInfo, char> passDecor{m_passDecorKey, ctx};
    SG::WriteDecorHandle<xAOD::EventInfo, unsigned int> nNodesDecor{m_nNodesDecorKey, ctx};
    SG::WriteDecorHandle<xAOD::EventInfo, unsigned int> nEdgesDecor{m_nEdgesDecorKey, ctx};

    scoreDecor(*eventInfo) = result.valid ? result.probability : -1.f;
    rawDecor(*eventInfo) = result.valid ? result.rawOutput : -1.f;
    passDecor(*eventInfo) = pass ? 1 : 0;
    nNodesDecor(*eventInfo) = static_cast<unsigned int>(result.nNodes);
    nEdgesDecor(*eventInfo) = static_cast<unsigned int>(result.nEdges);
  }

  return StatusCode::SUCCESS;
}

}  // namespace MuonML
