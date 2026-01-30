/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Author: Giovanni Marchiori (giovanni.marchiori@cern.ch)
// Note: while EGSelectionToolWrapper permits to only store the boolean
// accept and the isEM-like mask (works for both isEM and likelihood selectors),
// this tool (EGElectronLikelihoodToolWrapper) allows also to store
// the double TResult output (i.e. the value of the likelihood or the ECIDS BDT)
// if StoreTResult is set to true. Otherwise one can simply use the other tool.

#ifndef DERIVATIONFRAMEWORK_EGELECTRONLIKELIHOODTOOLWRAPPER_H
#define DERIVATIONFRAMEWORK_EGELECTRONLIKELIHOODTOOLWRAPPER_H
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
//
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
//
#include "AsgTools/IAsgTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "EgammaAnalysisInterfaces/IAsgElectronLikelihoodTool.h"
#include "EgammaAnalysisInterfaces/IElectronPhotonShowerShapeFudgeTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODEgamma/EgammaContainer.h"
//
#include <string>

namespace DerivationFramework {

  class EGElectronLikelihoodToolWrapper : public extends<AthAlgTool, IAugmentationTool>
  {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    ToolHandle<IAsgElectronLikelihoodTool> m_tool{
      this,
        "EGammaElectronLikelihoodTool",
        "",
        "Electron  Likelihood Selector"
        };
    ToolHandle<IElectronPhotonShowerShapeFudgeTool>
    m_fudgeMCTool{ this, "EGammaFudgeMCTool", "", "Fudging tool" };

    SG::ReadHandleKey<xAOD::EgammaContainer> m_ContainerName{ this,
      "ContainerName",
      "",
      "Input" };

    // Write decoration handle keys
    SG::WriteDecorHandleKey<xAOD::EgammaContainer>
    m_decoratorPass{ this, "decoratorPass", m_ContainerName, "", "" };
    SG::WriteDecorHandleKey<xAOD::EgammaContainer>
    m_decoratorIsEM{ this, "decoratorIsEM", m_ContainerName, "", "" };
    SG::WriteDecorHandleKey<xAOD::EgammaContainer>
    m_decoratorResult{ this, "decoratorResult", m_ContainerName, "", "" };
    SG::WriteDecorHandleKeyArray<xAOD::EgammaContainer, float>
    m_decoratorMultipleOutputs{this, "decoratorMultipleOutputs", m_ContainerName, {}, ""};
    Gaudi::Property<std::string> m_cut{this, "CutType", ""};
    Gaudi::Property<bool> m_storeTResult{this, "StoreTResult", false};
    Gaudi::Property<bool> m_storeMultipleOutputs{this, "StoreMultipleOutputs", false};
  };
}

#endif // DERIVATIONFRAMEWORK_EGELECTRONLIKELIHOODTOOLWRAPPER_H
