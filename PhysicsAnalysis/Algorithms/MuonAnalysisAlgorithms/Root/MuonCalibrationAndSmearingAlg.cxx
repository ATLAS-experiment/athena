/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <MuonAnalysisAlgorithms/MuonCalibrationAndSmearingAlg.h>

#include <RootCoreUtils/Assert.h>

//
// method implementations
//

namespace CP
{

  StatusCode MuonCalibrationAndSmearingAlg ::
  initialize ()
  {
    ANA_CHECK (m_calibrationAndSmearingTool.retrieve());
    ANA_CHECK (m_muonHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_muonHandle, SG::AllowEmpty));
    ANA_CHECK (m_systematicsList.addSystematics (*m_calibrationAndSmearingTool));
    ANA_CHECK (m_systematicsList.initialize());
    ANA_CHECK (m_outOfValidity.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode MuonCalibrationAndSmearingAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      // always need to call `getCopy` first to ensure that the shallow copies
      // are all there if requested
      xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.getCopy (muons, sys));

      if (sys.empty() && m_skipNominal.value())
        continue;

      ANA_CHECK (m_calibrationAndSmearingTool->applySystematicVariation (sys));
      for (xAOD::Muon *muon : *muons)
      {
        if (m_preselection.getBool (*muon, sys))
        {
          ANA_CHECK_CORRECTION (m_outOfValidity, *muon, m_calibrationAndSmearingTool->applyCorrection (*muon));
        }
      }
    }
    return StatusCode::SUCCESS;
  }
}
