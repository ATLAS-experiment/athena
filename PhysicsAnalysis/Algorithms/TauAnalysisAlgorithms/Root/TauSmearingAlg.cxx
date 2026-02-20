/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <TauAnalysisAlgorithms/TauSmearingAlg.h>

//
// method implementations
//

namespace CP
{
  // do this accessor at the file level to ensure it gets constructed
  // before we configure the output
  static const SG::Accessor<char> accTESCompatibility("TESCompatibility");

  StatusCode TauSmearingAlg ::
  initialize ()
  {
    ANA_CHECK (m_smearingTool.retrieve());
    ANA_CHECK (m_tauHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_tauHandle, SG::AllowEmpty));
    ANA_CHECK (m_systematicsList.addSystematics (*m_smearingTool));
    ANA_CHECK (m_systematicsList.initialize());
    ANA_CHECK (m_outOfValidity.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode TauSmearingAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      ANA_CHECK (m_smearingTool->applySystematicVariation (sys));
      xAOD::TauJetContainer *taus = nullptr;
      ANA_CHECK (m_tauHandle.getCopy (taus, sys));
      for (xAOD::TauJet *tau : *taus)
      {
        if (m_preselection.getBool (*tau, sys))
        {
          ANA_CHECK_CORRECTION (m_outOfValidity, *tau, m_smearingTool->applyCorrection (*tau));
        } else {
          accTESCompatibility(*tau) = char(0);
	}  
      }
    }
    return StatusCode::SUCCESS;
  }
}
