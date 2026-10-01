/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak

//
// includes
//

#include <JetAnalysisAlgorithms/JetGhostMuonAssociationAlg.h>
#include <METUtilities/METHelpers.h>

//
// method implementations
//

namespace CP
{
  StatusCode JetGhostMuonAssociationAlg ::
  initialize ()
  {
    ANA_CHECK (m_jetHandle.initialize (m_systematicsList));
    ANA_CHECK (m_muonHandle.initialize (m_systematicsList));
    ANA_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }


  StatusCode JetGhostMuonAssociationAlg ::
  execute (const EventContext& ctx)
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.getCopy (jets, sys, ctx));

      // associate the ghost muons to the jets (needed by MET muon-jet OR later)
      const xAOD::MuonContainer* muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys, ctx));
      met::addGhostMuonsToJets(*muons, *jets);
    }

    return StatusCode::SUCCESS;
  }
}
