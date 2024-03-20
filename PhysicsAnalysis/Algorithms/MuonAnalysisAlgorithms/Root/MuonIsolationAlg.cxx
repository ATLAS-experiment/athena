/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <MuonAnalysisAlgorithms/MuonIsolationAlg.h>

#include <RootCoreUtils/Assert.h>

//
// method implementations
//

namespace CP
{

  StatusCode MuonIsolationAlg ::
  initialize ()
  {

    ANA_CHECK (m_isolationTool.retrieve());
    ANA_CHECK (m_muonHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_preselection, SG::AllowEmpty));
    ANA_CHECK (m_isolationHandle.initialize (m_systematicsList, m_preselection));
    ANA_CHECK (m_systematicsList.initialize());

    if (!m_nameSvc.empty())
    {
      ANA_CHECK (m_nameSvc.retrieve());
      ANA_CHECK (m_nameSvc->addAcceptInfo (m_muonHandle.getNamePattern(), m_isolationHandle.getLabel(),
          m_isolationTool->getMuonAcceptInfo()));
    }

    asg::AcceptData blankAccept {&m_isolationTool->getObjAcceptInfo()};
    m_setOnFail = selectionFromAccept(blankAccept);

    return StatusCode::SUCCESS;
  }



  StatusCode MuonIsolationAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));
      for (const xAOD::Muon *muon : *muons)
      {
        if (m_preselection.getBool (*muon, sys))
        {
          m_isolationHandle.setBits
            (*muon, selectionFromAccept (m_isolationTool->accept (*muon)), sys);
        } else {
          m_isolationHandle.setBits (*muon, m_setOnFail, sys);
        }
      }
    }
    return StatusCode::SUCCESS;
  }
}
