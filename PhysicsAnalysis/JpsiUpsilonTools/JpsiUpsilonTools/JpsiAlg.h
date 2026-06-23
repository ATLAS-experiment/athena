/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/


#include "AthenaBaseComps/AthAlgorithm.h"
#include "JpsiUpsilonTools/ICandidateSearch.h"
#include "xAODTracking/VertexContainerFwd.h"
/////////////////////////////////////////////////////////////////////////////
class JpsiAlg : public AthAlgorithm {

public:
  JpsiAlg (const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) override;
  StatusCode finalize() override;

private:

  int m_eventCntr;
  int m_jpsiCntr;
 
  ToolHandle<Analysis::ICandidateSearch>      m_jpsiFinder;
  SG::WriteHandleKey<xAOD::VertexContainer>    m_jpsiContainerName; //!< Name of output container to store results

};
 
