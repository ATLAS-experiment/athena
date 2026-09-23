/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_EGSPHOTONBDTTOOLWRAPPER_H
#define DERIVATIONFRAMEWORK_EGSPHOTONBDTTOOLWRAPPER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
//
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
//
#include "EgammaAnalysisInterfaces/IAsgEGammaIsEMSelector.h"
#include "EgammaAnalysisInterfaces/IPhotonObservableTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODEgamma/EgammaContainer.h"
//
#include <string>

namespace DerivationFramework {

  class EGPhotonBDTToolWrapper : public AthReentrantAlgorithm
  {
  public:

    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override final;
    virtual StatusCode excute(const EventContext& ctx) const override final;

  private:
    // selector tool
    ToolHandle<IAsgEGammaIsEMSelector> m_selectorTool{this, "PhotonBDTSelectionTool", "", "Selector tool",};
    // photon container name
    SG::ReadHandleKey<xAOD::EgammaContainer> m_ContainerName{ this, "ContainerName", "", "Input" };
    // Fudged photon container name
    SG::ReadHandleKey<xAOD::EgammaContainer> m_fudgedContainerName{ this, "FudgedContainerName", "", "Input with fudge factors applied" };

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
