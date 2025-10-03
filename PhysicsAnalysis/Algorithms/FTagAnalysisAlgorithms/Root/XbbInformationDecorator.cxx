/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Binbin Dong

#include <FTagAnalysisAlgorithms/XbbInformationDecoratorAlg.h>

namespace CP
{
  StatusCode XbbInformationDecoratorAlg::initialize()
  {
    if (m_taggerDecisionDecoration.empty())
    {
      ATH_MSG_ERROR ("No tagger decision decoration name provided");
      return StatusCode::FAILURE;
    }

    m_taggerDecisionDecorator = std::make_unique<SG::Decorator<int>>(m_taggerDecisionDecoration);

    ANA_CHECK (m_selectionTool.retrieve());
    ANA_CHECK (m_jetHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_jetHandle, SG::AllowEmpty));
    ANA_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode XbbInformationDecoratorAlg :: execute () 
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    { 
      const xAOD::JetContainer *jets{};
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));

      for (const xAOD::Jet *jet : *jets)
      {
        int taggerDecision = -1;
        if (m_preselection.getBool (*jet, sys))
        {
          taggerDecision = m_selectionTool->accept (*jet);
        } 
        (*m_taggerDecisionDecorator)(*jet) = taggerDecision;
      }
    }
    return StatusCode::SUCCESS;
  }
}
