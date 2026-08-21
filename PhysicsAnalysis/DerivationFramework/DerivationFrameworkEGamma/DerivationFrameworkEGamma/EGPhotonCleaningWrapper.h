/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_EGPHOTONCLEANINGWRAPPER_H
#define DERIVATIONFRAMEWORK_EGPHOTONCLEANINGWRAPPER_H

#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
//
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
//
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODEgamma/EgammaContainer.h"
//
#include <string>
namespace DerivationFramework {

  class EGPhotonCleaningWrapper : public extends<AthAlgTool, IAugmentationTool>
  {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    SG::ReadHandleKey<xAOD::EgammaContainer> m_containerName{ this,
      "ContainerName",
      "",
      "Input" };
    SG::ReadHandleKey<xAOD::EgammaContainer> m_fudgedContainerName{ this,
      "FudgedContainerName", "", "Input with fudge factors applied" };

    // Write decoration handle keys
    SG::WriteDecorHandleKey<xAOD::EgammaContainer>
    m_decoratorPass{ this, "decoratorPass", m_containerName, "", "" };
    SG::WriteDecorHandleKey<xAOD::EgammaContainer>
    m_decoratorPassDelayed{ this, "decoratorPassDelayed", m_containerName, "", "" };
  };
}

#endif // DERIVATIONFRAMEWORK_EGSELECTIONTOOLWRAPPER_H
