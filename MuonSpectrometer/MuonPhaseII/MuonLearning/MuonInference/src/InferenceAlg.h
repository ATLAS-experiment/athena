/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCE_INFERENCEALG_H
#define MUONINFERENCE_INFERENCEALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

#include "MuonInferenceInterfaces/IGraphInferenceTool.h"
#include "MuonInferenceInterfaces/GraphData.h"

namespace MuonML {

class InferenceAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:
  ToolHandleArray<MuonML::IGraphInferenceTool> m_inferenceTools{
      this, "InferenceTools", {}
  };
};

} // namespace MuonML

#endif
