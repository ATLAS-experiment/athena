/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#include "EventSelectionAlgorithms/TransverseMassSelectorAlg.h"

namespace CP {

  TransverseMassSelectorAlg::TransverseMassSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator)
  : EL::AnaAlgorithm(name, pSvcLocator)
  {}

  StatusCode TransverseMassSelectorAlg::initialize() {
    ANA_CHECK(m_metHandle.initialize(m_systematicsList));
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

  StatusCode TransverseMassSelectorAlg::execute() {
    for (const auto &sys : m_systematicsList.systematicsVector()) {
      // retrieve the EventInfo
      const xAOD::EventInfo *evtInfo = nullptr;
      ANA_CHECK(m_eventInfoHandle.retrieve(evtInfo, sys));

      // default-decorate EventInfo
      m_decoration.setBool(*evtInfo, 0, sys);

      // check the preselection
      if (m_preselection && !m_preselection.getBool(*evtInfo, sys))
        continue;
      
      // retrieve the MET container
      const xAOD::MissingETContainer *met = nullptr;
      ANA_CHECK(m_metHandle.retrieve(met, sys));
      // retrieve the electron container
      const xAOD::ElectronContainer *electrons = nullptr;
      if (m_electronsHandle)
	ANA_CHECK(m_electronsHandle.retrieve(electrons, sys));
      // retrieve the electron container
      const xAOD::MuonContainer *muons = nullptr;
      if (m_muonsHandle)
	ANA_CHECK(m_muonsHandle.retrieve(muons, sys));

      // apply the requested selection, compute the W boson transverse mass
      float etmiss_pt = (*met)["Final"]->met();
      float etmiss_phi = (*met)["Final"]->phi();
      float lep_pt, lep_phi;
      int lep_count = 0;

      if (m_electronsHandle) {
	for (const xAOD::Electron *el : *electrons) {
	  if (!m_electronSelection || m_electronSelection.getBool(*el, sys)){
	    lep_pt = el->pt();
	    lep_phi = el->phi();
	    lep_count++;
	    break;
	  }
	}
      }
      if (m_muonsHandle) {
	for (const xAOD::Muon *mu : *muons) {
	  if (!m_muonSelection || m_muonSelection.getBool(*mu, sys)) {
	    lep_pt = mu->pt();
	    lep_phi = mu->phi();
	    lep_count++;
	    break;
	  }
	}
      }

      // exit 1 if there were no leptons or 2 or more leptons in the event 
      if (lep_count == 0){
        ATH_MSG_ERROR("No charged lepton in the event, cannot compute MWT!");
        return StatusCode::FAILURE;
      }
      if (lep_count > 1){
        ATH_MSG_ERROR("More than one lepton is associated with this event - cannot determine which is the W decay lepton.");
        return StatusCode::FAILURE;
      }

      float mwt = sqrt(2. * lep_pt * etmiss_pt * (1. - std::cos(lep_phi - etmiss_phi)));

      // calculate decision
      bool decision = SignEnum::checkValue(m_mwtref.value(), m_signEnum, mwt);
      m_decoration.setBool(*evtInfo, decision, sys);
    }
    return StatusCode::SUCCESS;
  }
} // namespace CP
