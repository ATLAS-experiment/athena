/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "DVInferenceAlg.h"

#include "xAODTrigger/TrigCompositeAuxContainer.h"

namespace MuonML {

StatusCode DVInferenceAlg::initialize() {
  ATH_CHECK(m_inferenceTool.retrieve());
  ATH_CHECK(m_trigCompositeKey.initialize(m_writeDVoutput));

  m_thresholdModeName = m_thresholdMode;
  if (m_thresholdModeName != "score" && m_thresholdModeName != "raw") {
    ATH_MSG_ERROR("ThresholdMode must be either 'score' or 'raw', got " << m_thresholdModeName);
    return StatusCode::FAILURE;
  }
  m_useRawThreshold = (m_thresholdModeName == "raw");
  const float scoreThreshold = m_scoreThreshold;
  ATH_MSG_INFO("Initialized DVInferenceAlg with ScoreThreshold=" << scoreThreshold
               << ", ThresholdMode=" << m_thresholdModeName
               << ", WriteDVoutput=" << m_writeDVoutput);
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

  if (m_writeDVoutput) {
    SG::WriteHandle<xAOD::TrigCompositeContainer> outputHandle{m_trigCompositeKey, ctx};
    ATH_CHECK(outputHandle.record(std::make_unique<xAOD::TrigCompositeContainer>(),
                                  std::make_unique<xAOD::TrigCompositeAuxContainer>()));

    xAOD::TrigComposite* trigComposite = outputHandle->push_back(std::make_unique<xAOD::TrigComposite>());

    trigComposite->setDetail<float>("Score", result.valid ? result.probability : -1.f);
    trigComposite->setDetail<float>("RawOutput", result.valid ? result.rawOutput : -1.f);
    trigComposite->setDetail<char>("Pass", pass ? 1 : 0);
    trigComposite->setDetail<unsigned int>("NNodes", static_cast<unsigned int>(result.nNodes));
    trigComposite->setDetail<unsigned int>("NEdges", static_cast<unsigned int>(result.nEdges));
  }

  return StatusCode::SUCCESS;
}

}  // namespace MuonML
