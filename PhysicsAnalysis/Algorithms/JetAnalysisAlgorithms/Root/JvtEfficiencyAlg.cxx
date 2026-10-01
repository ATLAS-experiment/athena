/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <JetAnalysisAlgorithms/JvtEfficiencyAlg.h>

//
// method implementations
//

namespace CP
{


  StatusCode JvtEfficiencyAlg ::
  initialize ()
  {

    ANA_CHECK (m_efficiencyTool.retrieve());
    ANA_CHECK (m_jetHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_jetHandle, SG::AllowEmpty));
    ANA_CHECK (m_selectionHandle.initialize (m_systematicsList, m_jetHandle, SG::AllowEmpty));
    ANA_CHECK (m_scaleFactorDecoration.initialize (m_systematicsList, m_jetHandle));
    ANA_CHECK (m_systematicsList.addSystematics (*m_efficiencyTool));
    ANA_CHECK (m_systematicsList.initialize());
    ANA_CHECK (m_outOfValidity.initialize());

    return StatusCode::SUCCESS;
  }



  StatusCode JvtEfficiencyAlg ::
  execute (const EventContext& ctx)
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      ANA_CHECK (m_efficiencyTool->applySystematicVariation (sys));
      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys, ctx));

      for (const xAOD::Jet *jet : *jets)
      {
        if (m_preselection.getBool (*jet, sys))
        {
          const bool goodJet = m_selectionHandle.getBool(*jet, sys);
          float sf = 1;
          if (goodJet) {
            ANA_CHECK_CORRECTION (m_outOfValidity, *jet, m_efficiencyTool->getEfficiencyScaleFactor (*jet, sf));
          } else if (!m_skipBadEfficiency) {
            ANA_CHECK_CORRECTION (m_outOfValidity, *jet, m_efficiencyTool->getInefficiencyScaleFactor (*jet, sf));
          }
          m_scaleFactorDecoration.set (*jet, sf, sys);
        } else {
          m_scaleFactorDecoration.set (*jet, invalidScaleFactor(), sys);
        }
      }
    }

    return StatusCode::SUCCESS;
  }
}
