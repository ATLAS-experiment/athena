/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORKBPHY_ANYVERTEXSKIMMINGTOOL_H
#define DERIVATIONFRAMEWORKBPHY_ANYVERTEXSKIMMINGTOOL_H
#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/ISkimmingTool.h"
#include "xAODTracking/VertexContainerFwd.h"
#include <vector>
//*This class is written as a stop gap because the standard expression parser isn't working with the cascade vertices
//***When this is resolved it can be deleted

namespace DerivationFramework {
  class AnyVertexSkimmingTool : public extends<AthAlgTool, ISkimmingTool>{
    public:
      AnyVertexSkimmingTool(const std::string&, const std::string&, const IInterface*);
      StatusCode initialize() override;
      virtual bool eventPassesFilter() const override;
      ~AnyVertexSkimmingTool();
    private:
       Gaudi::Property<std::vector<std::string>> m_containerNames{ this, "VertexContainerNames", {} };
       SG::ReadHandleKeyArray<xAOD::VertexContainer> m_keyArray {this, "ReadHandles", {} };
       Gaudi::Property<bool> m_useHandles{ this, "UseHandles", false };
  };
}

#endif