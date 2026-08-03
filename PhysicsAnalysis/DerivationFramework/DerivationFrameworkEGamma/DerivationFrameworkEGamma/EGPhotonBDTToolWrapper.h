/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_EGSPHOTONBDTTOOLWRAPPER_H
#define DERIVATIONFRAMEWORK_EGSPHOTONBDTTOOLWRAPPER_H

#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
//
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
//
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "EgammaAnalysisInterfaces/IAsgEGammaIsEMSelector.h"
#include "EgammaAnalysisInterfaces/IPhotonObservableTool.h"
#include "EgammaAnalysisInterfaces/IElectronPhotonShowerShapeFudgeTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODEgamma/EgammaContainer.h"
//
#include <string>

namespace DerivationFramework {

  class EGPhotonBDTToolWrapper : public extends<AthAlgTool, IAugmentationTool>
  {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    // selector tool
    ToolHandle<IAsgEGammaIsEMSelector> m_selectorTool{this, "PhotonBDTSelectionTool", "", "Selector tool",};
    // shower shape correction tool
    ToolHandle<IElectronPhotonShowerShapeFudgeTool> m_fudgeMCTool{ this, "EGammaFudgeMCTool", "", "Fudging tool" };
    // photon container name
    SG::ReadHandleKey<xAOD::EgammaContainer> m_ContainerName{ this, "ContainerName", "", "Input" };

    // Write decoration handle keys
    // these are not really configuarable
    SG::WriteDecorHandleKey<xAOD::EgammaContainer> m_decoratorPass{ this,
      "decoratorPass", m_ContainerName, "", "" };
    SG::WriteDecorHandleKey<xAOD::EgammaContainer> m_decoratorIsEM{ this,
      "decoratorIsEM", m_ContainerName, "", "" };
    Gaudi::Property<std::string> m_cut{ this, "CutType", "", "cut type" };
  };
}

#endif // DERIVATIONFRAMEWORK_EGPHOTONBDTTOOLWRAPPER_H
