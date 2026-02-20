/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_EGSELECTIONTOOLWRAPPER_H
#define DERIVATIONFRAMEWORK_EGSELECTIONTOOLWRAPPER_H

#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
//
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/WriteHandleKey.h"
//
#include "AsgTools/IAsgTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "EgammaAnalysisInterfaces/IAsgEGammaIsEMSelector.h"
#include "EgammaAnalysisInterfaces/IElectronPhotonShowerShapeFudgeTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODEgamma/EgammaContainer.h"
//
#include <string>

namespace DerivationFramework {

  class EGSelectionToolWrapper : public extends<AthAlgTool, IAugmentationTool>
  {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    ToolHandle<IAsgEGammaIsEMSelector> m_tool{
      this,
        "EGammaSelectionTool",
        "",
        "Selector tool",
        };
    ToolHandle<IElectronPhotonShowerShapeFudgeTool>
    m_fudgeMCTool{ this, "EGammaFudgeMCTool", "", "Fudging tool" };

    SG::ReadHandleKey<xAOD::EgammaContainer> m_ContainerName{ this,
      "ContainerName", "", "Input" };

    // Write decoration handle keys
    // these are not really configuarable
    SG::WriteDecorHandleKey<xAOD::EgammaContainer> m_decoratorPass{ this,
      "decoratorPass", m_ContainerName, "", "" };
    SG::WriteDecorHandleKey<xAOD::EgammaContainer> m_decoratorIsEM{ this,
      "decoratorIsEM", m_ContainerName, "", "" };
    Gaudi::Property<std::string> m_cut{ this, "CutType", "", "cut type" };

  };
}

#endif // DERIVATIONFRAMEWORK_EGSELECTIONTOOLWRAPPER_H
