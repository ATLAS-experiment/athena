/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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

  JetCalibrationAlg ::
  JetCalibrationAlg (const std::string& name, ISvcLocator* pSvcLocator)
    : EL::AnaAlgorithm (name, pSvcLocator)
  {
    declareProperty ("calibrationTool", m_calibrationTool, "The calibration tool we apply");
  }

  StatusCode JetCalibrationAlg ::
  initialize ()
  {
    ANA_CHECK (m_calibrationTool.retrieve());
    ANA_CHECK (m_jetHandle.initialize (m_systematicsList));
    ANA_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode JetCalibrationAlg ::
  execute (const EventContext& ctx)
  {
    static const std::string subtractedStr{"JetSubtractedScaleMomentum"};
    static const std::string constitStr{"JetConstitScaleMomentum"};
    static const std::string emScaleStr{"JetEMScaleMomentum"};
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.getCopy (jets, sys, ctx));

      if (m_HIsetup.value())
      {
        for(auto jet : *jets )
	{
          xAOD::JetFourMom_t jet_4mom_subtracted = (*jet).jetP4(subtractedStr);
	  (*jet).setJetP4(constitStr,jet_4mom_subtracted);
	  (*jet).setJetP4(emScaleStr,jet_4mom_subtracted);
	}
      }

      ANA_CHECK (m_calibrationTool->applyCalibration(*jets));
    }

    return StatusCode::SUCCESS;
  }
}
