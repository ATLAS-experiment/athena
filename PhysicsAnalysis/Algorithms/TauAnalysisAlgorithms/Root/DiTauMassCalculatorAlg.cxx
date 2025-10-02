/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#include <TauAnalysisAlgorithms/DiTauMassCalculatorAlg.h>
#include "AthContainers/ConstDataVector.h"

namespace CP {
  using ROOT::Math::PtEtaPhiMVector;

  StatusCode DiTauMassCalculatorAlg::initialize()
  {

    // the Missing Mass Calculator tool
    ANA_CHECK(m_mmc.retrieve());

    // input handles
    ANA_CHECK(m_electronsHandle.initialize(m_systematicsList));
    ANA_CHECK(m_electronSelection.initialize(m_systematicsList, m_electronsHandle, SG::AllowEmpty));

    ANA_CHECK(m_muonsHandle.initialize(m_systematicsList));
    ANA_CHECK(m_muonSelection.initialize(m_systematicsList, m_muonsHandle, SG::AllowEmpty));

    ANA_CHECK(m_jetsHandle.initialize(m_systematicsList));
    ANA_CHECK(m_jetSelection.initialize(m_systematicsList, m_jetsHandle, SG::AllowEmpty));

    ANA_CHECK(m_tausHandle.initialize(m_systematicsList));
    ANA_CHECK(m_tauSelection.initialize(m_systematicsList, m_tausHandle, SG::AllowEmpty));

    ANA_CHECK(m_preselection.initialize(m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_metHandle.initialize(m_systematicsList));
    ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));

    // output handles
    ANA_CHECK(m_fitStatus_decor.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_maxw_mass_decor.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_mlm_mass_decor.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_mlnu3p_mass_decor.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_mlnu3p_res_4vect_decor.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_mlnu3p_nu1_4vect_decor.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_mlnu3p_nu2_4vect_decor.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_mlnu3p_tau1_4vect_decor.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_mlnu3p_tau2_4vect_decor.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_maxw_res_4vect_decor.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_maxw_nu1_4vect_decor.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_maxw_nu2_4vect_decor.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_maxw_tau1_4vect_decor.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_maxw_tau2_4vect_decor.initialize(m_systematicsList, m_eventInfoHandle));

    ANA_CHECK(m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode DiTauMassCalculatorAlg::execute()
  {
    for (const auto &sys : m_systematicsList.systematicsVector())
      {
	// retrieve the EventInfo
	const xAOD::EventInfo *evtInfo = nullptr;
	ANA_CHECK(m_eventInfoHandle.retrieve(evtInfo, sys));

	// check the preselection
	if (m_preselection && !m_preselection.getBool(*evtInfo, sys))
	  continue;

	// retrieve objects
	const xAOD::ElectronContainer *electrons = nullptr;
	ANA_CHECK(m_electronsHandle.retrieve(electrons, sys));
	const xAOD::MuonContainer *muons = nullptr;
	ANA_CHECK(m_muonsHandle.retrieve(muons, sys));
	const xAOD::TauJetContainer *taus = nullptr;
	ANA_CHECK(m_tausHandle.retrieve(taus, sys));
	const xAOD::JetContainer *jets = nullptr;
	ANA_CHECK(m_jetsHandle.retrieve(jets, sys));
	const xAOD::MissingETContainer *met = nullptr;
	ANA_CHECK(m_metHandle.retrieve(met, sys));

	// apply object-wise selection
	ConstDataVector<xAOD::ElectronContainer> selected_electrons(SG::VIEW_ELEMENTS);
	ConstDataVector<xAOD::MuonContainer> selected_muons(SG::VIEW_ELEMENTS);
	ConstDataVector<xAOD::TauJetContainer> selected_taus(SG::VIEW_ELEMENTS);
	ConstDataVector<xAOD::JetContainer> selected_jets(SG::VIEW_ELEMENTS);

	for (const xAOD::Electron *el : *electrons)
	  {
	    if (m_electronSelection.getBool(*el, sys))
	      selected_electrons.push_back(el);
	  }

	for (const xAOD::Muon *mu : *muons)
	  {
	    if (m_muonSelection.getBool(*mu, sys))
	      selected_muons.push_back(mu);
	  }

	for (const xAOD::TauJet *tau : *taus)
	  {
	    if (m_tauSelection.getBool(*tau, sys))
	      selected_taus.push_back(tau);
	  }

	for (const xAOD::Jet *jet : *jets)
	  {
	    if (m_jetSelection.getBool(*jet, sys))
	      selected_jets.push_back(jet);
	  }

	int nJets = selected_jets.size();

	const xAOD::IParticle *vis1 = 0, *vis2 = 0;
	// to assign the visible particles on which to run the MMC, we assume that the user would prefer
	// 1) tau_had + tau_had
	// 2) tau_had + e
	// 3) tau_had + mu
	// 4) e + mu
	// 5) mu + mu
	// 6) e + e
	// To force a custom ordering, simply decorate your desired selection onto the particles before running this algorithm.
	// e.g. you could "book" an OSSF pair of light leptons as originating from a Z boson, and remove them from consideration here.

	if (selected_taus.size() >= 2)
	  {
	    vis1 = selected_taus.at(0);
	    vis2 = selected_taus.at(1);
	  }
	else if (selected_taus.size() == 1 && selected_electrons.size() >= 1)
	  {
	    vis1 = selected_taus.at(0);
	    vis2 = selected_electrons.at(0);
	  }
	else if (selected_taus.size() == 1 && selected_muons.size() >= 1)
	  {
	    vis1 = selected_taus.at(0);
	    vis2 = selected_muons.at(0);
	  }
	else if (selected_electrons.size() >= 1 && selected_muons.size() >= 1)
	  {
	    vis1 = selected_electrons.at(0);
	    vis2 = selected_muons.at(0);
	  }
	else if (selected_muons.size() >= 2)
	  {
	    vis1 = selected_muons.at(0);
	    vis2 = selected_muons.at(1);
	  }
	else if (selected_electrons.size() >= 2)
	  {
	    vis1 = selected_electrons.at(0);
	    vis2 = selected_electrons.at(1);
	  }
	else
	  {
	    ANA_MSG_WARNING("Not enough charged leptons in the event to run the MMC!");
	  }

    if ((*met)["Final"] == nullptr) {
        ANA_MSG_ERROR("The MET term " << "Final" << " doesn't exist! Aborting.");
        return StatusCode::FAILURE;
      }

	ANA_CHECK(m_mmc->apply(*evtInfo, vis1, vis2, (*met)["Final"], nJets));

	// retrieve the output variables and decorate them
	PtEtaPhiMVector null4V(0.0, 0.0, 0.0, 0.0);
	int fitStatus        = m_mmc->GetFitStatus(0);
	double mlm_mass      = fitStatus == 1 ? m_mmc->GetFittedMass(DiTauMassTools::MMCFitMethod::MLM)    : -1;
	if (m_doMAXW) {
	  double maxw_mass   = fitStatus == 1 ? m_mmc->GetFittedMass(DiTauMassTools::MMCFitMethod::MAXW)   : -1;
	  PtEtaPhiMVector maxw_res_4vect    = fitStatus == 1 ? m_mmc->GetResonanceVec(DiTauMassTools::MMCFitMethod::MAXW)      : null4V;
	  PtEtaPhiMVector maxw_nu1_4vect    = fitStatus == 1 ? m_mmc->GetNeutrino4vec(DiTauMassTools::MMCFitMethod::MAXW, 0)   : null4V;
	  PtEtaPhiMVector maxw_nu2_4vect    = fitStatus == 1 ? m_mmc->GetNeutrino4vec(DiTauMassTools::MMCFitMethod::MAXW, 1)   : null4V;
	  PtEtaPhiMVector maxw_tau1_4vect   = fitStatus == 1 ? m_mmc->GetTau4vec(DiTauMassTools::MMCFitMethod::MAXW, 0)        : null4V;
	  PtEtaPhiMVector maxw_tau2_4vect   = fitStatus == 1 ? m_mmc->GetTau4vec(DiTauMassTools::MMCFitMethod::MAXW, 1)        : null4V;
	  m_maxw_mass_decor.set(*evtInfo, maxw_mass, sys);
	  m_maxw_res_4vect_decor.set(*evtInfo, maxw_res_4vect, sys);
	  m_maxw_nu1_4vect_decor.set(*evtInfo, maxw_nu1_4vect, sys);
	  m_maxw_nu2_4vect_decor.set(*evtInfo, maxw_nu2_4vect, sys);
	  m_maxw_tau1_4vect_decor.set(*evtInfo, maxw_tau1_4vect, sys);
	  m_maxw_tau2_4vect_decor.set(*evtInfo, maxw_tau2_4vect, sys);
	}
	if (m_doMLNU3P) {
	  double mlnu3p_mass = fitStatus == 1 ? m_mmc->GetFittedMass(DiTauMassTools::MMCFitMethod::MLNU3P) : -1;
	  PtEtaPhiMVector mlnu3p_res_4vect  = fitStatus == 1 ? m_mmc->GetResonanceVec(DiTauMassTools::MMCFitMethod::MLNU3P)    : null4V;
	  PtEtaPhiMVector mlnu3p_nu1_4vect  = fitStatus == 1 ? m_mmc->GetNeutrino4vec(DiTauMassTools::MMCFitMethod::MLNU3P, 0) : null4V;
	  PtEtaPhiMVector mlnu3p_nu2_4vect  = fitStatus == 1 ? m_mmc->GetNeutrino4vec(DiTauMassTools::MMCFitMethod::MLNU3P, 1) : null4V;
	  PtEtaPhiMVector mlnu3p_tau1_4vect = fitStatus == 1 ? m_mmc->GetTau4vec(DiTauMassTools::MMCFitMethod::MLNU3P, 0)      : null4V;
	  PtEtaPhiMVector mlnu3p_tau2_4vect = fitStatus == 1 ? m_mmc->GetTau4vec(DiTauMassTools::MMCFitMethod::MLNU3P, 1)      : null4V;
	  m_mlnu3p_mass_decor.set(*evtInfo, mlnu3p_mass, sys);
	  m_mlnu3p_res_4vect_decor.set(*evtInfo, mlnu3p_res_4vect, sys);
	  m_mlnu3p_nu1_4vect_decor.set(*evtInfo, mlnu3p_nu1_4vect, sys);
	  m_mlnu3p_nu2_4vect_decor.set(*evtInfo, mlnu3p_nu2_4vect, sys);
	  m_mlnu3p_tau1_4vect_decor.set(*evtInfo, mlnu3p_tau1_4vect, sys);
	  m_mlnu3p_tau2_4vect_decor.set(*evtInfo, mlnu3p_tau2_4vect, sys);
	}
	m_fitStatus_decor.set(*evtInfo, fitStatus, sys);
	m_mlm_mass_decor.set(*evtInfo, mlm_mass, sys);
      }

    return StatusCode::SUCCESS;
  }

} // namespace
