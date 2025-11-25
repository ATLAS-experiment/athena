/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef DERIVATIONFRAMEWORK_ASGSELECTIONTOOLWRAPPER_H
#define DERIVATIONFRAMEWORK_ASGSELECTIONTOOLWRAPPER_H



#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "PATCore/IAsgSelectionTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODBase/IParticleContainer.h"

namespace DerivationFramework {

  class AsgSelectionToolWrapper : public extends<AthAlgTool, IAugmentationTool> {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    PublicToolHandle<IAsgSelectionTool> m_tool{this, "AsgSelectionTool", ""};
    Gaudi::Property<std::string> m_cut{this, "CutType", "" };
    SG::ReadHandleKey<xAOD::IParticleContainer> m_containerKey{this, "ContainerName", ""};
    SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_decorKey{this, "StoreGateEntryName", m_containerKey, ""};
  };
}

#endif // DERIVATIONFRAMEWORK_ASGSELECTIONTOOLWRAPPER_H
