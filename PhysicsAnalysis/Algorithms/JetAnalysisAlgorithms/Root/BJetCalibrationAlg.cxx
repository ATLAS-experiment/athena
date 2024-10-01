/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Thomas Strebler


//
// includes
//

#include <JetAnalysisAlgorithms/BJetCalibrationAlg.h>

//
// method implementations
//

namespace CP
{

  StatusCode BJetCalibrationAlg ::
  initialize ()
  {
    ANA_CHECK (m_jetHandle.initialize (m_systematicsList));
    ANA_CHECK (m_jetPreselection.initialize
	       (m_systematicsList, m_jetHandle, SG::AllowEmpty));
    ANA_CHECK (m_muonHandle.initialize (m_systematicsList));
    ANA_CHECK (m_muonPreselection.initialize
	       (m_systematicsList, m_muonHandle, SG::AllowEmpty));

    ANA_CHECK (m_nmuons.initialize(m_systematicsList, m_jetHandle));

    if(!m_muonSelectionTool.empty()){
      ANA_CHECK (m_muonSelectionTool.retrieve());
      if(!m_muonPreselection.empty()){
        ATH_MSG_ERROR ("muonSelectionTool and muonPreselection should not be configured simultaneously");
        return StatusCode::FAILURE;
      }
    }

    ANA_CHECK (m_muonInJetTool.retrieve());
    if(!m_bJetTool.empty()) ANA_CHECK (m_bJetTool.retrieve());
    
    ANA_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode BJetCalibrationAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    { 
      xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.getCopy (jets, sys));

      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK (m_muonHandle.retrieve (muons, sys));

      std::vector<const xAOD::Muon*> muons_for_correction;
      muons_for_correction.reserve(muons->size());
      for(const xAOD::Muon* muon : *muons){
	if(// For analysis frameworks
	   (m_muonSelectionTool.empty() && m_muonPreselection.getBool(*muon, sys)) ||
	   // For other frameworks
	   (!m_muonSelectionTool.empty() && m_muonSelectionTool->accept(*muon)))
	  muons_for_correction.emplace_back(muon);
      }

      for(xAOD::Jet* jet : *jets) {

        jet->setJetP4("NoBJetCalibMomentum", jet->jetP4());
        jet->setJetP4("MuonCorrMomentum", jet->jetP4());

        int nmuons = 0;
        if(m_jetPreselection.getBool(*jet, sys)) {
          ANA_CHECK (m_muonInJetTool->applyMuonInJetCorrection(*jet, muons_for_correction, nmuons));
          jet->setJetP4("MuonCorrMomentum", jet->jetP4());
          if(!m_bJetTool.empty()){
            ANA_CHECK (m_bJetTool->applyBJetCorrection(*jet, nmuons>0));
          }
        }
        m_nmuons.set(*jet, nmuons, sys);

      }
    }

    return StatusCode::SUCCESS;
  }
}
