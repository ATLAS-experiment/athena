/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_EGSPHOTONBDTTOOLDECORATOR_H
#define DERIVATIONFRAMEWORK_EGSPHOTONBDTTOOLDECORATOR_H

#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
//
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
//
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "EgammaAnalysisInterfaces/IPhotonObservableTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODEgamma/EgammaContainer.h"
//
#include <string>

namespace DerivationFramework {

  class EGPhotonBDTToolDecorator : public extends<AthAlgTool, IAugmentationTool>
  {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    // photon observable tool (for calculating the BDT score)
    ToolHandle<IPhotonObservableTool> m_observableTool{this, "PhotonObservableTool", "", "Observable tool",};
    // photon container name
    SG::ReadHandleKey<xAOD::EgammaContainer> m_ContainerName{ this, "ContainerName", "", "Input to decorate" };
    // Fudged photon container name
    SG::ReadHandleKey<xAOD::EgammaContainer> m_fudgedContainerName{ this, "FudgedContainerName", "", "Input with fudge factors applied" };

    // Write decoration handle keys
    SG::WriteDecorHandleKey<xAOD::EgammaContainer> m_decoratorScore{ this,
      "decoratorScore", m_ContainerName, "", "" };
  };
}

#endif // DERIVATIONFRAMEWORK_EGPHOTONBDTTOOLDECORATOR_H
