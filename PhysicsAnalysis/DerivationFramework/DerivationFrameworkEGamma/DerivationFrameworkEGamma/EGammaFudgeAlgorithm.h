/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAFUDGEALGORITHM_H
#define EGAMMAFUDGEALGORITHM_H

// Gaudi/Athena include(s):
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AsgTools/ToolHandle.h"

// Local include(s):
#include "EgammaAnalysisInterfaces/IElectronPhotonShowerShapeFudgeTool.h"
#include "xAODEgamma/EgammaContainer.h"

namespace DerivationFramework {

  class EGammaFudgeAlgorithm : public AthReentrantAlgorithm {

  public:
    /// Regular Algorithm constructor
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    /// Function initialising the algorithm
    virtual StatusCode initialize() override;
    /// Function executing the algorithm
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:
    SG::ReadHandleKey<xAOD::EgammaContainer> m_inputKey{this, "Input", ""};
    SG::WriteHandleKey<xAOD::EgammaContainer> m_outputKey{this, "Output", ""};
    ToolHandle<IElectronPhotonShowerShapeFudgeTool> m_fudgeTool{ this, "EGammaFudgeTool", ""};

  }; // class 

} //namespace
#endif // EGAMMAFUDGEALGORITHM_H
