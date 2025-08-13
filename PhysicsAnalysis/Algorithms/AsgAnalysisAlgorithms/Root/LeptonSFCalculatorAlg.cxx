/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#include "AsgAnalysisAlgorithms/LeptonSFCalculatorAlg.h"

namespace CP {

  StatusCode LeptonSFCalculatorAlg::initialize() {

    ANA_CHECK(m_electronsHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_muonsHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_photonsHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_tausHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));

    ANA_CHECK(m_electronSelection.initialize(m_systematicsList, m_electronsHandle, SG::AllowEmpty));
    ANA_CHECK(m_muonSelection.initialize(m_systematicsList, m_muonsHandle, SG::AllowEmpty));
    ANA_CHECK(m_photonSelection.initialize(m_systematicsList, m_photonsHandle, SG::AllowEmpty));
    ANA_CHECK(m_tauSelection.initialize(m_systematicsList, m_tausHandle, SG::AllowEmpty));

    ANA_CHECK(m_electronSFs.initialize(m_systematicsList, m_electronsHandle));
    ANA_CHECK(m_muonSFs.initialize(m_systematicsList, m_muonsHandle));
    ANA_CHECK(m_photonSFs.initialize(m_systematicsList, m_photonsHandle));
    ANA_CHECK(m_tauSFs.initialize(m_systematicsList, m_tausHandle));

    ANA_CHECK(m_event_leptonSF.initialize(m_systematicsList, m_eventInfoHandle));

    ANA_CHECK(m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode LeptonSFCalculatorAlg::execute() {
    for (const auto& syst : m_systematicsList.systematicsVector()) {
      const xAOD::EventInfo *evtInfo {nullptr};
      ANA_CHECK(m_eventInfoHandle.retrieve(evtInfo, syst));

      const xAOD::ElectronContainer *electrons {nullptr};
      if (m_electronsHandle) ANA_CHECK(m_electronsHandle.retrieve(electrons, syst));

      const xAOD::MuonContainer *muons {nullptr};
      if (m_muonsHandle) ANA_CHECK(m_muonsHandle.retrieve(muons, syst));

      const xAOD::PhotonContainer *photons {nullptr};
      if (m_photonsHandle) ANA_CHECK(m_photonsHandle.retrieve(photons, syst));

      const xAOD::TauJetContainer *taus {nullptr};
      if (m_tausHandle) ANA_CHECK(m_tausHandle.retrieve(taus, syst));

      double leptonSF {1.};
      if (m_electronsHandle){
	for (const xAOD::Electron *el : *electrons) {
	  if (m_electronSelection.getBool(*el, syst)) {
	    for (size_t i{}; i < m_electronSFs.size(); i++) {
	      leptonSF *= m_electronSFs.at(i).get(*el, syst);
	    }
	  }
	}
      }
      if (m_muonsHandle){
        for (const xAOD::Muon *mu : *muons) {
          if (m_muonSelection.getBool(*mu, syst)) {
            for (size_t i{}; i < m_muonSFs.size(); i++) {
              leptonSF *= m_muonSFs.at(i).get(*mu, syst);
            }
          }
        }
      }
      if (m_photonsHandle){
        for (const xAOD::Photon *ph : *photons) {
          if (m_photonSelection.getBool(*ph, syst)) {
            for (size_t i{}; i < m_photonSFs.size(); i++) {
              leptonSF *= m_photonSFs.at(i).get(*ph, syst);
            }
          }
        }
      }
      if (m_tausHandle){
        for (const xAOD::TauJet *tau : *taus) {
          if (m_tauSelection.getBool(*tau, syst)) {
            for (size_t i{}; i < m_tauSFs.size(); i++) {
              leptonSF *= m_tauSFs.at(i).get(*tau, syst);
            }
          }
        }
      }

      m_event_leptonSF.set(*evtInfo, leptonSF, syst);
    }
    return StatusCode::SUCCESS;
  }

} // namespace
