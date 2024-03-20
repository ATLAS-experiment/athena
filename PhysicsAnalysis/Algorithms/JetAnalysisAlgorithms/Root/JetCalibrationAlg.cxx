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
      ANA_CHECK (m_calibrationTool->applyCalibration(*jets));
    }

    return StatusCode::SUCCESS;
  }
}
