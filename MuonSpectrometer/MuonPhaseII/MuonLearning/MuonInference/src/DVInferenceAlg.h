/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCE_DVINFERENCEALG_H
#define MUONINFERENCE_DVINFERENCEALG_H

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "xAODEventInfo/EventInfo.h"

#include "DVInferenceToolBase.h"

namespace MuonML {

/**
 * @brief Run the DV event classifier and publish one score per event.
 * 
 * EventInfo decorations are kept as an opt-in validation/debug output.
 */
class DVInferenceAlg final : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;

private:
  ToolHandle<MuonML::DVInferenceToolBase> m_inferenceTool{
      this, "InferenceTool", "MuonML::DVInferenceToolBase/DisplacedVertexInferenceTool"};

  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{
      this, "EventInfoKey", "EventInfo", "Optional validation EventInfo object to decorate with DV inference output"};

  Gaudi::Property<bool> m_decorateEventInfo{
      this, "DecorateEventInfo", false, "Opt-in: Decorate EventInfo with DV classifier outputs"};

  SG::WriteDecorHandleKey<xAOD::EventInfo> m_scoreDecorKey{
      this, "ScoreDecoration", "EventInfo.dv_score", "Optional: Event-level DV signal probability"};
  SG::WriteDecorHandleKey<xAOD::EventInfo> m_rawDecorKey{
      this, "RawOutputDecoration", "EventInfo.dv_rawOutput", "Optional: Raw event-level DV ONNX output"};
  SG::WriteDecorHandleKey<xAOD::EventInfo> m_passDecorKey{
      this, "PassDecoration", "EventInfo.dv_pass", "Optional: Whether DV score passes ScoreThreshold"};
  SG::WriteDecorHandleKey<xAOD::EventInfo> m_nNodesDecorKey{
      this, "NNodesDecoration", "EventInfo.dv_nNodes", "Optional: Number of nodes in the DV event graph"};
  SG::WriteDecorHandleKey<xAOD::EventInfo> m_nEdgesDecorKey{
      this, "NEdgesDecoration", "EventInfo.dv_nEdges", "Optional: Number of directed edges in the DV event graph"};

  Gaudi::Property<float> m_scoreThreshold{
      this, "ScoreThreshold", 0.5f, "DV event score threshold used for the pass decision"};
  Gaudi::Property<std::string> m_thresholdMode{
      this, "ThresholdMode", "score", "Quantity compared to ScoreThreshold."};
  Gaudi::Property<bool> m_printEveryEvent{
      this, "PrintEveryEvent", false, "Print the DV inference result for every event at INFO level"};
  bool m_useRawThreshold{false};
  std::string m_thresholdModeName{"score"};
};

}  // namespace MuonML

#endif
