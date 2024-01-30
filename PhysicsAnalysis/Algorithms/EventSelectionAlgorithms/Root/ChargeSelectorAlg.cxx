/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#include "EventSelectionAlgorithms/ChargeSelectorAlg.h"

namespace CP {

  ChargeSelectorAlg::ChargeSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator)
  : EL::AnaAlgorithm(name, pSvcLocator)
  {}

  StatusCode ChargeSelectorAlg::initialize() {
    ANA_CHECK(m_electronsHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_electronSelection.initialize(m_systematicsList, m_electronsHandle, SG::AllowEmpty));
    ANA_CHECK(m_muonsHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_muonSelection.initialize(m_systematicsList, m_muonsHandle, SG::AllowEmpty));
    ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));

    ANA_CHECK(m_preselection.initialize(m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_decoration.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode ChargeSelectorAlg::execute() {
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

      // apply the requested selection and compute the local charge
      int total_charge = 0;
      int total_leptons = 0;
      if (m_electronsHandle) {
	for (const xAOD::Electron *el : *electrons) {
	  if (!m_electronSelection || m_electronSelection.getBool(*el, sys)){
	    total_charge += el->charge();
	    total_leptons++;
	  }
	}
      }
      if (m_muonsHandle) {
	for (const xAOD::Muon *mu : *muons) {
	  if (!m_muonSelection || m_muonSelection.getBool(*mu, sys)){
	    total_charge += mu->charge();
	    total_leptons++; 
	  }
	}
      }

      // check that there are only 2 leptons
      if (total_leptons != 2 ) {
        ATH_MSG_ERROR("Exactly two leptons are required to check whether the event is OS or SS!");
        return StatusCode::FAILURE;
      }

      // compare to the requested mode
      bool decision = m_OSmode ? total_charge == 0 : std::abs(total_charge) == 2;
      m_decoration.setBool(*evtInfo, decision, sys);
    }
    return StatusCode::SUCCESS;
  }
} // namespace CP
