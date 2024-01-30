/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#include "EventSelectionAlgorithms/DileptonInvariantMassSelectorAlg.h"

namespace CP {

  DileptonInvariantMassSelectorAlg::DileptonInvariantMassSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator)
  : EL::AnaAlgorithm(name, pSvcLocator)
  {}

  StatusCode DileptonInvariantMassSelectorAlg::initialize() {
    ANA_CHECK(m_electronsHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_electronSelection.initialize(m_systematicsList, m_electronsHandle, SG::AllowEmpty));
    ANA_CHECK(m_muonsHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_muonSelection.initialize(m_systematicsList, m_muonsHandle, SG::AllowEmpty));
    ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));

    ANA_CHECK(m_preselection.initialize(m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_decoration.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_systematicsList.initialize());

    m_signEnum = SignEnum::stringToOperator.at( m_sign );

    return StatusCode::SUCCESS;
  }

  StatusCode DileptonInvariantMassSelectorAlg::execute() {
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
      // retrieve the electron container
      const xAOD::MuonContainer *muons = nullptr;
      if (m_muonsHandle)
	ANA_CHECK(m_muonsHandle.retrieve(muons, sys));

      // apply the requested selection
      TLorentzVector lepton0, lepton1;
      int total_leptons = 0;
      bool isfilled0(false), isfilled1(false);
      if (m_electronsHandle) {
	for (const xAOD::Electron *el : *electrons) {
	  if (!m_electronSelection || m_electronSelection.getBool(*el, sys)) {
	    total_leptons++;
	    if (!isfilled0){
	      lepton0 = el->p4();
	      isfilled0 = true;
	    } else if (!isfilled1){
	      lepton1 = el->p4();
	      isfilled1 = true;
	    } else {
	      break;
	    }
	  } 
	}
      }
      if (m_muonsHandle) {
	for (const xAOD::Muon *mu : *muons) {
	  if (!m_muonSelection || m_muonSelection.getBool(*mu, sys)) {
	    total_leptons++;
	    if (!isfilled0){
	      lepton0 = mu->p4();
	      isfilled0 = true;
	    } else if (!isfilled1){
	      lepton1 = mu->p4();
	      isfilled1 = true;
	    } else {
	      break;
	    }
	  }
	}
      }

      if (total_leptons != 2){
        ATH_MSG_ERROR("Exactly two leptons are required to compute the MLL!");
        return StatusCode::FAILURE;
      }

       // compute MLL
      float mll = (lepton0 + lepton1).M();

      // calculate decision
      bool decision = SignEnum::checkValue(m_mllref.value(), m_signEnum, mll);
      m_decoration.setBool(*evtInfo, decision, sys);
    }
    return StatusCode::SUCCESS;
  }
} // namespace CP
