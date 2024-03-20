/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak

//
// includes
//

#include <MuonAnalysisAlgorithms/MuonSelectionAlg.h>
#include <PATCore/IAsgSelectionTool.h>

//
// method implementations
//

namespace CP
{

  StatusCode MuonSelectionAlgV2 ::
  initialize ()
  {
    ANA_CHECK (m_selectionTool.retrieve());

    ANA_CHECK (m_muonsHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_muonsHandle, SG::AllowEmpty));
    ANA_CHECK (m_selectionHandle.initialize (m_systematicsList, m_muonsHandle));
    ANA_CHECK (m_badMuonVetoHandle.initialize (m_systematicsList, m_muonsHandle, SG::AllowEmpty));
    ANA_CHECK (m_systematicsList.initialize());

    auto *selectionTool = dynamic_cast<IAsgSelectionTool *>(&*m_selectionTool);
    asg::AcceptData blankAccept {&selectionTool->getAcceptInfo()};
    m_setOnFail = selectionFromAccept(blankAccept);

    if (!m_nameSvc.empty())
    {
      ANA_CHECK (m_nameSvc.retrieve());
      ANA_CHECK (m_nameSvc->addAcceptInfo (m_muonsHandle.getNamePattern(), m_selectionHandle.getLabel(),
          m_selectionTool->getAcceptInfo()));
    }

    return StatusCode::SUCCESS;
  }



  StatusCode MuonSelectionAlgV2 ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonsHandle.retrieve (muons, sys));
      for (const xAOD::Muon *muon : *muons)
      {
        if (m_preselection.getBool (*muon, sys))
        {
          m_selectionHandle.setBits
            (*muon, selectionFromAccept (m_selectionTool->accept (*muon)), sys);
          
          if (m_badMuonVetoHandle)
          {
            m_badMuonVetoHandle.setBool (*muon, m_selectionTool->isBadMuon (*muon), sys);
          }
        } else {
          m_selectionHandle.setBits (*muon, m_setOnFail, sys);

          if (m_badMuonVetoHandle)
          {
            m_badMuonVetoHandle.setBool (*muon, false, sys);
          }
        }
      }
    }
    return StatusCode::SUCCESS;
  }
}
