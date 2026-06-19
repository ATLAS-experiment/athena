/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
                          InDetV0Finder.h  -  Description
                             -------------------
    begin   : 20-07-2005
    authors : Evelina Bouhova-Thacker (Lancaster University), Rob Henderson (Lancater University)
    email   : e.bouhova@cern.ch, r.henderson@lancaster.ac.uk
    changes :

 ***************************************************************************/

#ifndef INDETV0FINDER_INDETV0FINDER_H
#define INDETV0FINDER_INDETV0FINDER_H

#include "InDetV0Finder/V0MainDecorator.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODTracking/VertexContainerFwd.h"


/**
   @class InDetV0Finder
   Execute method for the main V0 finding module.
   InDetV0Finder uses the InDetV0FinderTool and records the V0 containers:
   V0UnconstrVertices, V0KshortVertices, V0LambdaVertices and V0LambdabarVertices.
   If decorateV0 = True, the mass, pT and Rxy with corresponding errors are stored.
*/

/* Forward declarations */

namespace InDet
{
  class InDetV0FinderTool;
  
  class InDetV0Finder : public AthAlgorithm
  {
  public:
    InDetV0Finder(const std::string &name, ISvcLocator *pSvcLocator);
    virtual ~InDetV0Finder();
    StatusCode initialize();
    StatusCode execute(const EventContext& ctx);
    StatusCode finalize();
    StatusCode resetStatistics();
    

  protected:
    // input primary vertices
    SG::ReadHandleKey<xAOD::VertexContainer>        m_vertexKey { this, "VxPrimaryCandidateName", "PrimaryVertices", 
                                                                  "key for retrieving vertices" };
    // V0 candidate output containers
    SG::WriteHandleKey<xAOD::VertexContainer>       m_v0Key { this, "V0ContainerName", "V0Candidates", "V0 container" };
    SG::WriteHandleKey<xAOD::VertexContainer>       m_ksKey { this, "KshortContainerName", "KshortCandidates", "Ks container" };
    SG::WriteHandleKey<xAOD::VertexContainer>       m_laKey { this, "LambdaContainerName", "LambdaCandidates",
                                                              "Lambda container" };
    SG::WriteHandleKey<xAOD::VertexContainer>       m_lbKey { this, "LambdabarContainerName", "LambdabarCandidates", 
                                                              "Lambdabar container" };

    // Tools

    ToolHandle<InDet::InDetV0FinderTool> m_v0FinderTool{this, "InDetV0FinderToolName", "InDet::InDetV0FinderTool"};
    ToolHandle<InDet::V0MainDecorator> m_v0DecoTool{this, "Decorator", "InDet::V0MainDecorator"};

    // Other members

    BooleanProperty m_decorate{this, "decorateV0", true}; //!< decorate V0 containers

    long m_events_processed = 0;
    long m_V0s_stored = 0;
    long m_Kshort_stored = 0;
    long m_Lambda_stored = 0;
    long m_Lambdabar_stored = 0;

  };

}//end of namespace InDet

#endif

