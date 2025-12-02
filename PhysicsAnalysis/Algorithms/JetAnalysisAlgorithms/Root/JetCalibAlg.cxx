/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author P-A Delsart


//
// includes
//

#include <JetAnalysisAlgorithms/JetCalibAlg.h>

//
// method implementations
//

namespace CP
{

  StatusCode JetCalibAlg ::
  initialize ()
  {
    ATH_MSG_INFO("Initialize for jets: "<< m_jetHandle.getNamePattern() );
    ANA_CHECK (m_calibrationTool.retrieve());
    ANA_CHECK (m_jetHandle.initialize (m_systematicsList));
    ANA_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode JetCalibAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.getCopy (jets, sys));
      ANA_CHECK (m_calibrationTool->calibrate(*jets));
    }

    return StatusCode::SUCCESS;
  }
}
