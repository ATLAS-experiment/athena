/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Binbin Dong

#include "EventSelectionAlgorithms/DileptonOSSFInvariantMassWindowSelectorAlg.h"

namespace CP {

    DileptonOSSFInvariantMassWindowSelectorAlg::DileptonOSSFInvariantMassWindowSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator)
  : EL::AnaAlgorithm(name, pSvcLocator)
  {}

  StatusCode DileptonOSSFInvariantMassWindowSelectorAlg::initialize() {
    ANA_CHECK(m_electronsHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_electronSelection.initialize(m_systematicsList, m_electronsHandle, SG::AllowEmpty));
    ANA_CHECK(m_muonsHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_muonSelection.initialize(m_systematicsList, m_muonsHandle, SG::AllowEmpty));
    ANA_CHECK(m_electronsTruthHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_electronTruthSelection.initialize(m_systematicsList, m_electronsHandle, SG::AllowEmpty));
    ANA_CHECK(m_muonsTruthHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_muonTruthSelection.initialize(m_systematicsList, m_muonsHandle, SG::AllowEmpty));
    ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));
 
    ANA_CHECK(m_preselection.initialize(m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_decoration.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_systematicsList.initialize());
 
    return StatusCode::SUCCESS;
  }

  StatusCode DileptonOSSFInvariantMassWindowSelectorAlg::execute() {
    for (const auto &sys : m_systematicsList.systematicsVector()) {
      // retrieve the EventInfo
      const xAOD::EventInfo *evtInfo = nullptr;
      ANA_CHECK(m_eventInfoHandle.retrieve(evtInfo, sys));

      // default-decorate EventInfo
      m_decoration.setBool(*evtInfo, 0, sys);

      // check the preselection
      if (m_preselection && !m_preselection.getBool(*evtInfo, sys))
        continue;

      // retrieve the electron container
      const xAOD::ElectronContainer *electrons = nullptr;
      if (m_electronsHandle)
        ANA_CHECK(m_electronsHandle.retrieve(electrons, sys));
      // retrieve the muon container
      const xAOD::MuonContainer *muons = nullptr;
      if (m_muonsHandle)
        ANA_CHECK(m_muonsHandle.retrieve(muons, sys));
      // retrieve the truth electron container
      const xAOD::TruthParticleContainer *truthElectrons = nullptr;
      if (m_electronsTruthHandle)
	ANA_CHECK(m_electronsTruthHandle.retrieve(truthElectrons, sys));
      // retrieve the truth muon container
      const xAOD::TruthParticleContainer *truthMuons = nullptr;
      if (m_muonsTruthHandle)
	ANA_CHECK(m_muonsTruthHandle.retrieve(truthMuons, sys));

      bool decision = false;

      if (m_electronsHandle || m_muonsHandle) {
	if (electrons->size() >= 2) {
	  for (size_t i = 0; i < electrons->size() - 1 && !decision; ++i) {
	    const xAOD::Electron* firstElectron = (*electrons)[i];
	    if (!m_electronSelection || m_electronSelection.getBool(*firstElectron, sys)) {
	      for (size_t j = i + 1; j < electrons->size() && !decision; ++j) {
		const xAOD::Electron* secondElectron = (*electrons)[j];
		if (!m_electronSelection || m_electronSelection.getBool(*secondElectron, sys)) {
		  if (firstElectron->charge() != secondElectron->charge()){
		    float mll = (firstElectron->p4() + secondElectron->p4()).M();
		    decision |= (mll < m_mll_upper && mll > m_mll_lower);
		  }
		}
	      }
	    }
	  }
	}

	// If a pair of electrons satisfies the mass requirements, there is no need to loop over muon pairs. The event is either kept or vetoed hereafter.
	if (!decision && muons->size() >= 2) {
	  for (size_t i = 0; i < muons->size() - 1 && !decision; ++i) {
	    const xAOD::Muon* firstMuon = (*muons)[i];
	    if (!m_muonSelection || m_muonSelection.getBool(*firstMuon, sys)) {
	      for (size_t j = i + 1; j < muons->size() && !decision; ++j) {
		const xAOD::Muon* secondMuon = (*muons)[j];
		if (!m_muonSelection || m_muonSelection.getBool(*secondMuon, sys)) {
		  if (firstMuon->charge() != secondMuon->charge()){
		    float mll = (firstMuon->p4() + secondMuon->p4()).M();
		    decision |= (mll < m_mll_upper && mll > m_mll_lower);
		  }
		}
	      }
	    }
	  }
	}
      }
      else {
	if (truthElectrons->size() >= 2) {
	  for (size_t i = 0; i < truthElectrons->size() - 1 && !decision; ++i) {
	    const xAOD::TruthParticle* firstElectron = (*truthElectrons)[i];
	    if (!m_electronTruthSelection || m_electronTruthSelection.getBool(*firstElectron, sys)) {
	      for (size_t j = i + 1; j < truthElectrons->size() && !decision; ++j) {
		const xAOD::TruthParticle* secondElectron = (*truthElectrons)[j];
		if (!m_electronTruthSelection || m_electronTruthSelection.getBool(*secondElectron, sys)) {
		  if (firstElectron->charge() != secondElectron->charge()){
		    float mll = (firstElectron->p4() + secondElectron->p4()).M();
		    decision |= (mll < m_mll_upper && mll > m_mll_lower);
		  }
		}
	      }
	    }
	  }
	}

	// If a pair of electrons satisfies the mass requirements, there is no need to loop over muon pairs. The event is either kept or vetoed hereafter.
	if (!decision && truthMuons->size() >= 2) {
	  for (size_t i = 0; i < truthMuons->size() - 1 && !decision; ++i) {
	    const xAOD::TruthParticle* firstMuon = (*truthMuons)[i];
	    if (!m_muonTruthSelection || m_muonTruthSelection.getBool(*firstMuon, sys)) {
	      for (size_t j = i + 1; j < truthMuons->size() && !decision; ++j) {
		const xAOD::TruthParticle* secondMuon = (*truthMuons)[j];
		if (!m_muonTruthSelection || m_muonTruthSelection.getBool(*secondMuon, sys)) {
		  if (firstMuon->charge() != secondMuon->charge()){
		    float mll = (firstMuon->p4() + secondMuon->p4()).M();
		    decision |= (mll < m_mll_upper && mll > m_mll_lower);
		  }
		}
	      }
	    }
	  }
	}
      }

      if (m_veto) decision = !decision;
      m_decoration.setBool(*evtInfo, decision, sys);

    }
    return StatusCode::SUCCESS;
  }
}
