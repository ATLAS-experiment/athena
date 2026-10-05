/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
    ANA_CHECK (m_truthVisLeadPtDecor.initialize (m_systematicsList, m_tauHandle));
    ANA_CHECK (m_truthVisLeadEtaDecor.initialize (m_systematicsList, m_tauHandle));
    ANA_CHECK (m_truthVisLeadPhiDecor.initialize (m_systematicsList, m_tauHandle));
    ANA_CHECK (m_truthVisLeadMDecor.initialize (m_systematicsList, m_tauHandle));
    ANA_CHECK (m_truthLeadPdgIDDecor.initialize (m_systematicsList, m_tauHandle));
    ANA_CHECK (m_truthVisSubleadPtDecor.initialize (m_systematicsList, m_tauHandle));
    ANA_CHECK (m_truthVisSubleadEtaDecor.initialize (m_systematicsList, m_tauHandle));
    ANA_CHECK (m_truthVisSubleadPhiDecor.initialize (m_systematicsList, m_tauHandle));
    ANA_CHECK (m_truthVisSubleadMDecor.initialize (m_systematicsList, m_tauHandle));
    ANA_CHECK (m_truthSubleadPdgIDDecor.initialize (m_systematicsList, m_tauHandle));
    ANA_CHECK (m_truthVisDeltaRDecor.initialize (m_systematicsList, m_tauHandle));
    ANA_CHECK (m_truthVisMassDecor.initialize (m_systematicsList, m_tauHandle));
    ANA_CHECK (m_isTruthMatchedDecor.initialize (m_systematicsList, m_tauHandle));
    ANA_CHECK (m_isTruthHadronicDecor.initialize (m_systematicsList, m_tauHandle));
    ANA_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode DiTauTruthMatchingAlg ::
  execute (const EventContext& ctx)
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::DiTauJetContainer *taus = nullptr;
      ANA_CHECK (m_tauHandle.retrieve (taus, sys, ctx));
      // all decorations done within the DiTauTruthMatchingTool and to be saved in output
      for (const xAOD::DiTauJet *tau : *taus)
      {
        if (m_preselection.getBool (*tau, sys))
        {
          m_matchingTool->getTruth (*tau);
        } else {
          // for failed selection, use the same default value as for failed match
          m_truthVisLeadPtDecor.set (*tau, -1234, sys);
          m_truthVisLeadEtaDecor.set (*tau, -1234, sys);
          m_truthVisLeadPhiDecor.set (*tau, -1234, sys);
          m_truthVisLeadMDecor.set (*tau, -1234, sys);
          m_truthLeadPdgIDDecor.set (*tau, -1234, sys);
          m_truthVisSubleadPtDecor.set (*tau, -1234, sys);
          m_truthVisSubleadEtaDecor.set (*tau, -1234, sys);
          m_truthVisSubleadPhiDecor.set (*tau, -1234, sys);
          m_truthVisSubleadMDecor.set (*tau, -1234, sys);
          m_truthSubleadPdgIDDecor.set (*tau, -1234, sys);
          m_truthVisDeltaRDecor.set (*tau, -1234, sys);
          m_truthVisMassDecor.set (*tau, -1234, sys);
          m_isTruthMatchedDecor.set (*tau, char(false), sys);
          m_isTruthHadronicDecor.set (*tau, char(false), sys);
        }
      }
    }
    return StatusCode::SUCCESS;
  }
}
