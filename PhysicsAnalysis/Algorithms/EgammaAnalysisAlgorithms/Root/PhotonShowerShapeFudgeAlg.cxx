/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <EgammaAnalysisAlgorithms/PhotonShowerShapeFudgeAlg.h>

//
// method implementations
//

namespace CP
{

  StatusCode PhotonShowerShapeFudgeAlg ::
  initialize ()
  {
    ANA_CHECK (m_showerShapeFudgeTool.retrieve());
    ANA_CHECK (m_photonHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_photonHandle, SG::AllowEmpty));
    ANA_CHECK (m_systematicsList.initialize());
    ANA_CHECK (m_outOfValidity.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode PhotonShowerShapeFudgeAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      xAOD::PhotonContainer *photons = nullptr;
      ANA_CHECK (m_photonHandle.getCopy (photons, sys));
      for (xAOD::Photon *photon : *photons)
      {
        if (m_preselection.getBool (*photon, sys))
        {
          ANA_CHECK_CORRECTION (m_outOfValidity, *photon, m_showerShapeFudgeTool->applyCorrection (*photon));
        }
      }
    }
    return StatusCode::SUCCESS;
  }
}
