/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <TauAnalysisAlgorithms/DiTauTruthMatchingAlg.h>

//
// method implementations
//

namespace CP
{

  StatusCode DiTauTruthMatchingAlg ::
  initialize ()
  {
    ANA_CHECK (m_matchingTool.retrieve());
    ANA_CHECK (m_tauHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_tauHandle, SG::AllowEmpty));
    ANA_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode DiTauTruthMatchingAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::DiTauJetContainer *taus = nullptr;
      ANA_CHECK (m_tauHandle.retrieve (taus, sys));
      // all decorations done within the DiTauTruthMatchingTool and to be saved in output  
      static const SG::Decorator<float> accTruthVisLeadPt("TruthVisLeadPt");
      static const SG::Decorator<float> accTruthVisLeadEta("TruthVisLeadEta");
      static const SG::Decorator<float> accTruthVisLeadPhi("TruthVisLeadPhi");
      static const SG::Decorator<float> accTruthVisLeadM("TruthVisLeadM");
      static const SG::Decorator<int> accTruthLeadPdgID("TruthLeadPdgID");
      static const SG::Decorator<float> accTruthVisSubleadPt("TruthVisSubleadPt");
      static const SG::Decorator<float> accTruthVisSubleadEta("TruthVisSubleadEta");
      static const SG::Decorator<float> accTruthVisSubleadPhi("TruthVisSubleadPhi");
      static const SG::Decorator<float> accTruthVisSubleadM("TruthVisSubleadM");
      static const SG::Decorator<int> accTruthSubleadPdgID("TruthSubleadPdgID");
      static const SG::Decorator<float> accTruthVisDeltaR("TruthVisDeltaR");
      static const SG::Decorator<float> accTruthVisMass("TruthVisMass");
      static const SG::Decorator<char> accIsTruthMatched("IsTruthMatched");
      static const SG::Decorator<char> accIsTruthHadronic("IsTruthHadronic");

      for (const xAOD::DiTauJet *tau : *taus)
      {
        if (m_preselection.getBool (*tau, sys))
        {
          m_matchingTool->getTruth (*tau);
        } else {
	  // for failed selection, use the same default value as for failed match  	
          accTruthVisLeadPt(*tau) = -1234;
	  accTruthVisLeadEta(*tau) = -1234;
	  accTruthVisLeadPhi(*tau) = -1234;
	  accTruthVisLeadM(*tau)  = -1234;
	  accTruthLeadPdgID(*tau) =  -1234;
	  accTruthVisSubleadPt(*tau) = -1234;
	  accTruthVisSubleadEta(*tau) = -1234;
	  accTruthVisSubleadPhi(*tau) = -1234;
	  accTruthVisSubleadM(*tau) = -1234;
	  accTruthSubleadPdgID(*tau) = -1234;
	  accTruthVisDeltaR(*tau) = -1234;
	  accTruthVisMass(*tau) = -1234;
	  accIsTruthMatched(*tau) = char(false);
	  accIsTruthHadronic(*tau) = char(false); 
	}
      }
    }
    return StatusCode::SUCCESS;
  }
}
