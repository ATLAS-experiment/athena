/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCE_DVINFERENCEALG_H
#define MUONINFERENCE_DVINFERENCEALG_H

#include "DVInferenceToolBase.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "xAODTrigger/TrigCompositeContainer.h"

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

  Gaudi::Property<bool> m_writeDVoutput{
      this, "WriteDVoutput", false, "Whether to write the DV inference output into storegate"};

  SG::WriteHandleKey<xAOD::TrigCompositeContainer> m_trigCompositeKey{
      this, "TrigCompositeKey", "EF_MuonDVInferenceOutput", "Optional: Dummy xAOD::TrigComposite object to be decorated with DV inference outputs"};

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
