/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#include "EventSelectionAlgorithms/DileptonInvariantMassWindowSelectorAlg.h"
#include "Math/Vector4D.h"

using ROOT::Math::PtEtaPhiEVector;

namespace CP {

  DileptonInvariantMassWindowSelectorAlg::DileptonInvariantMassWindowSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator)
  : EL::AnaAlgorithm(name, pSvcLocator)
  {}

  StatusCode DileptonInvariantMassWindowSelectorAlg::initialize() {
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

  StatusCode DileptonInvariantMassWindowSelectorAlg::execute() {
    // accessors
    static const SG::ConstAccessor<float> acc_pt_dressed("pt_dressed");
    static const SG::ConstAccessor<float> acc_eta_dressed("eta_dressed");
    static const SG::ConstAccessor<float> acc_phi_dressed("phi_dressed");
    static const SG::ConstAccessor<float> acc_e_dressed("e_dressed");

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
      const xAOD::IParticleContainer *electrons = nullptr;
      if (m_electronsHandle)
	ANA_CHECK(m_electronsHandle.retrieve(electrons, sys));
      // retrieve the electron container
      const xAOD::IParticleContainer *muons = nullptr;
      if (m_muonsHandle)
	ANA_CHECK(m_muonsHandle.retrieve(muons, sys));

      // apply the requested selection
      PtEtaPhiEVector lepton0, lepton1;
      int total_leptons = 0;
      bool isfilled0(false), isfilled1(false);
      if (m_electronsHandle) {
	for (const xAOD::IParticle *el : *electrons) {
	  if (!m_electronSelection || m_electronSelection.getBool(*el, sys)) {
	    total_leptons++;
	    if (!isfilled0){
          if (m_useDressedProperties)
            lepton0.SetCoordinates(acc_pt_dressed(*el),
                                   acc_eta_dressed(*el),
                                   acc_phi_dressed(*el),
                                   acc_e_dressed(*el));
          else
            lepton0.SetCoordinates(el->pt(), el->eta(), el->phi(), el->e());
	      isfilled0 = true;
	    } else if (!isfilled1){
          if (m_useDressedProperties)
            lepton1.SetCoordinates(acc_pt_dressed(*el),
                                   acc_eta_dressed(*el),
                                   acc_phi_dressed(*el),
                                   acc_e_dressed(*el));
          else
            lepton1.SetCoordinates(el->pt(), el->eta(), el->phi(), el->e());
	      isfilled1 = true;
	    } else {
	      break;
	    }
	  }
	}
      }
      if (m_muonsHandle) {
	for (const xAOD::IParticle *mu : *muons) {
	  if (!m_muonSelection || m_muonSelection.getBool(*mu, sys)) {
	    total_leptons++;
	    if (!isfilled0){
          if (m_useDressedProperties)
            lepton0.SetCoordinates(acc_pt_dressed(*mu),
                                   acc_eta_dressed(*mu),
                                   acc_phi_dressed(*mu),
                                   acc_e_dressed(*mu));
          else
            lepton0.SetCoordinates(mu->pt(), mu->eta(), mu->phi(), mu->e());
	      isfilled0 = true;
	    } else if (!isfilled1){
          if (m_useDressedProperties)
            lepton1.SetCoordinates(acc_pt_dressed(*mu),
                                   acc_eta_dressed(*mu),
                                   acc_phi_dressed(*mu),
                                   acc_e_dressed(*mu));
          else
            lepton1.SetCoordinates(mu->pt(), mu->eta(), mu->phi(), mu->e());
	      isfilled1 = true;
	    } else {
	      break;
	    }
	  }
	}
      }

      if (total_leptons != 2){
        ATH_MSG_ERROR("Exactly two leptons are required to compute the MLL window!");
        return StatusCode::FAILURE;
      }

       // compute MLL
      float mll = (lepton0 + lepton1).M();

      // calculate decision
      bool in_range = ( mll > m_mlllower && mll < m_mllupper );
      bool decision = m_veto ? (!in_range) : in_range;
      m_decoration.setBool(*evtInfo, decision, sys);
    }
    return StatusCode::SUCCESS;
  }
} // namespace CP
