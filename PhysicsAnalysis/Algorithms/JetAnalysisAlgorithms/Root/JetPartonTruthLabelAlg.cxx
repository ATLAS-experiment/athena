/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


//
// includes
//

#include <JetAnalysisAlgorithms/JetPartonTruthLabelAlg.h>

//
// method implementations
//

namespace CP
{
  JetPartonTruthLabelAlg ::
  JetPartonTruthLabelAlg (const std::string& name,
                              ISvcLocator* pSvcLocator)
    : AnaAlgorithm (name, pSvcLocator)
  {
  }


  StatusCode JetPartonTruthLabelAlg ::
  initialize ()
  {
    ANA_CHECK(m_labelTool.retrieve());
    ANA_CHECK (m_jetHandle.initialize (m_systematicsList));
    ANA_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }


  StatusCode JetPartonTruthLabelAlg ::
  execute ()
  {

    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.getCopy (jets, sys));

      for(xAOD::Jet* jet : *jets) {

        m_labelTool->modifyJet(*jet);


      }
    }

    return StatusCode::SUCCESS;
  }
}
