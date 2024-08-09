/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
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
  JetDecoratorAlg ::
  JetDecoratorAlg (const std::string& name,
                ISvcLocator* pSvcLocator)
    : AnaAlgorithm (name, pSvcLocator)
  {
  }



  StatusCode JetDecoratorAlg ::
  initialize ()
  {

    ANA_CHECK (m_decorator.retrieve());
    ANA_CHECK (m_jetHandle.initialize (m_systematicsList));
    ANA_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode JetDecoratorAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.getCopy(jets, sys));
      ANA_CHECK (m_decorator->decorate(*jets));
    }

    return StatusCode::SUCCESS;
  }
}
