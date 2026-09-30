/*
 Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
    ANA_CHECK (m_outOfValidity.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode XbbEfficiencyAlg :: execute(const EventContext& ctx)
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys, ctx));

      for (const xAOD::Jet *jet : *jets)
      {
        float sf = 0.;
        if (m_preselection.getBool (*jet, sys))
        {
          ANA_CHECK_CORRECTION (m_outOfValidity, *jet, m_efficiencyTool->getScaleFactor (*jet, sf, sys));
          if (!m_outOfValidity.get(*jet))
            sf = invalidScaleFactor();
        }
        m_scaleFactorDecoration.set (*jet, sf, sys);
      }
    }
    return StatusCode::SUCCESS;
  }
}
