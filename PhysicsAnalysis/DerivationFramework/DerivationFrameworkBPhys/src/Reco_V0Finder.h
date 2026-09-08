/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
///////////////////////////////////////////////////////////////////
// Reco_V0Finder.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_V0FINDER_H
#define DERIVATIONFRAMEWORK_V0FINDER_H

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "InDetV0Finder/InDetV0FinderTool.h"
#include "InDetV0Finder/V0MainDecorator.h"


namespace DerivationFramework {

  class Reco_V0Finder : public AthReentrantAlgorithm {
    public: 
      Reco_V0Finder(const std::string& name, ISvcLocator* svcLoc);

      StatusCode initialize() override;
      
      virtual StatusCode execute(const EventContext& ctx) const override;
      
    private:
      
      std::vector<std::string> m_CollectionsToCheck;
      ToolHandle <InDet::InDetV0FinderTool> m_v0FinderTool;
      SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo_key{this, "EventInfo", "EventInfo", "Input event information"};
      SG::ReadHandleKey<xAOD::VertexContainer>        m_vertexKey { this, "VxPrimaryCandidateName", "PrimaryVertices", 
                                                                  "key for retrieving vertices" };

      SG::WriteHandleKey<xAOD::VertexContainer>       m_v0Key { this, "V0ContainerName", "V0Candidates", "V0 container" };
      SG::WriteHandleKey<xAOD::VertexContainer>       m_ksKey { this, "KshortContainerName", "KshortCandidates", "Ks container" };
      SG::WriteHandleKey<xAOD::VertexContainer>       m_laKey { this, "LambdaContainerName", "LambdaCandidates",
                                                                "Lambda container" };
      SG::WriteHandleKey<xAOD::VertexContainer>       m_lbKey { this, "LambdabarContainerName", "LambdabarCandidates", 
                                                                "Lambdabar container" };

      ToolHandle<InDet::V0MainDecorator> m_v0DecoTool{this, "Decorator", "InDet::V0MainDecorator"};
  }; 
}

#endif // DERIVATIONFRAMEWORK_Reco_dimuTrk_H

