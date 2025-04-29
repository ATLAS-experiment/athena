/*
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Binbin Dong


#include <FTagAnalysisAlgorithms/XbbEfficiencyAlg.h>

namespace CP
{
  StatusCode XbbEfficiencyAlg :: initialize()
  {
    if (m_scaleFactorDecoration.empty())
    {
      ANA_MSG_ERROR ("no scale factor decoration name set");
      return StatusCode::FAILURE;
    }

    ANA_CHECK (m_efficiencyTool.retrieve());
    ANA_CHECK (m_jetHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_jetHandle, SG::AllowEmpty));
    ANA_CHECK (m_scaleFactorDecoration.initialize (m_systematicsList, m_jetHandle));
    ANA_CHECK (m_systematicsList.addSystematics (*m_efficiencyTool));
    ANA_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode XbbEfficiencyAlg :: execute()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      for (const xAOD::Jet *jet : *jets)
      {
        float sf = 0.;
        if (m_preselection.getBool (*jet, sys))
        {
          (void)m_efficiencyTool->getScaleFactor (*jet, sf, sys);
        }
        m_scaleFactorDecoration.set (*jet, sf, sys);
      }
    }
    return StatusCode::SUCCESS;
  }
}
