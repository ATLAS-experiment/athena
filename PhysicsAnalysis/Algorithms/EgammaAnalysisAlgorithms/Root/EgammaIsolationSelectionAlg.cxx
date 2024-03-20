/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <EgammaAnalysisAlgorithms/EgammaIsolationSelectionAlg.h>

//
// method implementations
//

namespace CP
{

  StatusCode EgammaIsolationSelectionAlg ::
  initialize ()
  {
    ANA_CHECK (m_selectionTool.retrieve());
      
    ANA_CHECK (m_egammasHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_egammasHandle, SG::AllowEmpty));
    ANA_CHECK (m_selectionHandle.initialize (m_systematicsList, m_egammasHandle));
    ANA_CHECK (m_systematicsList.initialize());

    if (!m_nameSvc.empty())
    {
      ANA_CHECK (m_nameSvc.retrieve());
      ANA_CHECK (m_nameSvc->addAcceptInfo (m_egammasHandle.getNamePattern(), m_selectionHandle.getLabel(),
          m_isPhoton.value() ? m_selectionTool->getPhotonAcceptInfo() : m_selectionTool->getElectronAcceptInfo()));
    }

    asg::AcceptData blankAccept {&(m_isPhoton.value() ? m_selectionTool->getPhotonAcceptInfo() : m_selectionTool->getElectronAcceptInfo())};
    m_setOnFail = selectionFromAccept(blankAccept);

    return StatusCode::SUCCESS;
  }



  StatusCode EgammaIsolationSelectionAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::EgammaContainer *egammas = nullptr;
      ANA_CHECK (m_egammasHandle.retrieve (egammas, sys));
      for (const xAOD::Egamma *egamma : *egammas)
      {
        if (m_preselection.getBool (*egamma, sys))
        {
          m_selectionHandle.setBits
            (*egamma, selectionFromAccept (m_selectionTool->accept (*egamma)), sys);
        } else {
          m_selectionHandle.setBits (*egamma, m_setOnFail, sys);
        }
      }
    }
    return StatusCode::SUCCESS;
  }
}
