/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Teng Jian Khoo


//
// includes
//

#include <JetAnalysisAlgorithms/JetDecoratorAlg.h>

//
// method implementations
//

namespace CP
{
  StatusCode JetDecoratorAlg ::
  initialize ()
  {

    ANA_CHECK (m_decorator.retrieve());
    ANA_CHECK (m_jetHandle.initialize (m_systematicsList));
    ANA_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode JetDecoratorAlg ::
  execute (const EventContext& ctx)
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.getCopy(jets, sys, ctx));
      ANA_CHECK (m_decorator->decorate(*jets));
    }

    return StatusCode::SUCCESS;
  }
}
