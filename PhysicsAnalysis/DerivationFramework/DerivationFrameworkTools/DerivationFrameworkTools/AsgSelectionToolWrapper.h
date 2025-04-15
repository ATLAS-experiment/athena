/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
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
      AsgSelectionToolWrapper(const std::string& t, const std::string& n, const IInterface* p);

      StatusCode initialize() override final;
      virtual StatusCode addBranches() const override final;

    private:

      PublicToolHandle<IAsgSelectionTool> m_tool{this, "AsgSelectionTool", ""};
      Gaudi::Property<std::string> m_cut{this, "CutType", "" };
      Gaudi::Property<std::string> m_sgName{this, "StoreGateEntryName", ""};
      
      SG::ReadHandleKey<xAOD::IParticleContainer> m_containerKey{this, "ContainerName", ""};

      SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_decorKey{this, "DecorationKey", "", 
                                        "Will be composed by <ContainerName>.<StoreGateEntryName>"};
      
      
  }; 
}

#endif // DERIVATIONFRAMEWORK_ASGSELECTIONTOOLWRAPPER_H
