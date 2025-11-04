/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


//
// includes
//
#include "AsgAnalysisAlgorithms/NJetDecoratorAlg.h"

//
// method implementations
//

namespace CP
{

  StatusCode NJetDecoratorAlg ::
  initialize ()
  {
    ANA_CHECK (m_eventInfoHandle.initialize(m_systematicsList));
    ANA_CHECK(m_jetsHandle.initialize(m_systematicsList));
    ANA_CHECK(m_jetSelection.initialize(m_systematicsList, m_jetsHandle, SG::AllowEmpty));
    ANA_CHECK(m_Njet_decor.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode NJetDecoratorAlg ::
  execute ()
  {
    // Take care of the weight (which is the only thing depending on systematics)
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::EventInfo* systEvtInfo = nullptr;
      ANA_CHECK( m_eventInfoHandle.retrieve(systEvtInfo, sys));
      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK(m_jetsHandle.retrieve(jets, sys));

      int jet_n = 0;
      for (const xAOD::Jet *jet : *jets)
      {
	if (m_jetSelection.getBool(*jet, sys)){
	  jet_n++;
	}
      }
      m_Njet_decor.set(*systEvtInfo, jet_n, sys);
    };
    return StatusCode::SUCCESS;
  }
}
