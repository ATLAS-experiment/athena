/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <EgammaAnalysisAlgorithms/EgammaIsolationSelectionAlg.h>
#include "PATCore/AcceptData.h"
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

    const asg::AcceptInfo& acceptInfo = m_isPhoton.value() ? m_selectionTool->getPhotonAcceptInfo() : m_selectionTool->getElectronAcceptInfo();

    if (!m_nameSvc.empty())
    {
      ANA_CHECK (m_nameSvc.retrieve());
      ANA_CHECK (m_nameSvc->addAcceptInfo (m_egammasHandle.getNamePattern(), m_selectionHandle.getLabel(), acceptInfo));
    }

    asg::AcceptData blankAccept {&acceptInfo};
    m_setOnFail = selectionFromAccept(blankAccept);

    return StatusCode::SUCCESS;
  }



  StatusCode EgammaIsolationSelectionAlg ::
  execute (const EventContext& ctx)
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::EgammaContainer *egammas = nullptr;
      ANA_CHECK (m_egammasHandle.retrieve (egammas, sys, ctx));
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
