/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <MuonAnalysisAlgorithms/MuonCalibrationAndSmearingAlg.h>

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
    if (!m_calibrationAndSmearingTool_ZeroPix.empty())
    {
      ANA_CHECK (m_calibrationAndSmearingTool_ZeroPix.retrieve());
      ANA_CHECK (m_systematicsList.addSystematics (*m_calibrationAndSmearingTool_ZeroPix));
    }
    ANA_CHECK (m_systematicsList.initialize());
    ANA_CHECK (m_outOfValidity.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode MuonCalibrationAndSmearingAlg ::
  execute (const EventContext& ctx)
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      // always need to call `getCopy` first to ensure that the shallow copies
      // are all there if requested
      xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.getCopy (muons, sys, ctx));

      if (sys.empty() && m_skipNominal.value())
        continue;

      ANA_CHECK (m_calibrationAndSmearingTool->applySystematicVariation (sys));
      if (!m_calibrationAndSmearingTool_ZeroPix.empty())
        ANA_CHECK (m_calibrationAndSmearingTool_ZeroPix->applySystematicVariation (sys));

      for (xAOD::Muon *muon : *muons)
      {
        if (m_preselection.getBool (*muon, sys))
        {
          // Apply MS-only calibration if ZPH muons are used
          if (!m_calibrationAndSmearingTool_ZeroPix.empty() && muon->muonType() == m_zeroPixMuonType.value())
          {
            ANA_CHECK_CORRECTION (m_outOfValidity, *muon, m_calibrationAndSmearingTool_ZeroPix->applyCorrection (*muon));
          }
          else
          {
            ANA_CHECK_CORRECTION (m_outOfValidity, *muon, m_calibrationAndSmearingTool->applyCorrection (*muon));
          }
        }
      }
    }
    return StatusCode::SUCCESS;
  }
}
