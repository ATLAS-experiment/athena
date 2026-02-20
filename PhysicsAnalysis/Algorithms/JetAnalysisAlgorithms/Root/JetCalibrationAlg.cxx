/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <JetAnalysisAlgorithms/JetCalibrationAlg.h>

//
// method implementations
//

namespace CP
{

  StatusCode JetCalibrationAlg ::
  initialize ()
  {
    ANA_CHECK (m_calibrationTool.retrieve());
    ANA_CHECK (m_jetHandle.initialize (m_systematicsList));
    ANA_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode JetCalibrationAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.getCopy (jets, sys));

      if (m_HIsetup.value())
      {
        for(auto jet : *jets )
	{
          xAOD::JetFourMom_t jet_4mom_subtracted = (*jet).jetP4("JetSubtractedScaleMomentum");
	  (*jet).setJetP4("JetConstitScaleMomentum",jet_4mom_subtracted);
	  (*jet).setJetP4("JetEMScaleMomentum",jet_4mom_subtracted);
	}
      }

      ANA_CHECK (m_calibrationTool->applyCalibration(*jets));
    }

    return StatusCode::SUCCESS;
  }
}
