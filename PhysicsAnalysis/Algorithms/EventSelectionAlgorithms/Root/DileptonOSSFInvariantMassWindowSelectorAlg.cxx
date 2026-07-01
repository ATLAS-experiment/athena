/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Binbin Dong
/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#include "EventSelectionAlgorithms/DileptonOSSFInvariantMassWindowSelectorAlg.h"

using ROOT::Math::PtEtaPhiEVector;

namespace CP {

  StatusCode DileptonOSSFInvariantMassWindowSelectorAlg::initialize() {
    ANA_CHECK(m_electronsHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_electronSelection.initialize(m_systematicsList, m_electronsHandle, SG::AllowEmpty));
    ANA_CHECK(m_muonsHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_muonSelection.initialize(m_systematicsList, m_muonsHandle, SG::AllowEmpty));
    ANA_CHECK(m_electronsTruthHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_electronTruthSelection.initialize(m_systematicsList, m_electronsTruthHandle, SG::AllowEmpty));
    ANA_CHECK(m_muonsTruthHandle.initialize(m_systematicsList, SG::AllowEmpty));
    ANA_CHECK(m_muonTruthSelection.initialize(m_systematicsList, m_muonsTruthHandle, SG::AllowEmpty));
    ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));
    ANA_CHECK(m_preselection.initialize(m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK(m_decoration.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }

  // Compute invariant mass of two reco leptons
  template <typename T>
  static float mll_reco(const T* lep0, const T* lep1) {
    return (lep0->p4() + lep1->p4()).M();
  }

  // Compute invariant mass of two truth particles, optionally using dressed kinematics
  static float mll_truth(const xAOD::TruthParticle* lep0,
                          const xAOD::TruthParticle* lep1,
                          bool useDressed) {
    if (!useDressed)
      return (lep0->p4() + lep1->p4()).M();

    static const SG::ConstAccessor<float> acc_pt ("pt_dressed");
    static const SG::ConstAccessor<float> acc_eta("eta_dressed");
    static const SG::ConstAccessor<float> acc_phi("phi_dressed");
    static const SG::ConstAccessor<float> acc_e  ("e_dressed");

    PtEtaPhiEVector v0, v1;
    v0.SetCoordinates(acc_pt(*lep0), acc_eta(*lep0), acc_phi(*lep0), acc_e(*lep0));
    v1.SetCoordinates(acc_pt(*lep1), acc_eta(*lep1), acc_phi(*lep1), acc_e(*lep1));
    return (v0 + v1).M();
  }

  // Check whether mll falls inside the window
  bool DileptonOSSFInvariantMassWindowSelectorAlg::inWindow(float mll) const {
    return mll > m_mll_lower && mll < m_mll_upper;
  }

  StatusCode DileptonOSSFInvariantMassWindowSelectorAlg::execute(const EventContext& ctx) {

    for (const auto &sys : m_systematicsList.systematicsVector()) {

      const xAOD::EventInfo *evtInfo = nullptr;
      ANA_CHECK(m_eventInfoHandle.retrieve(evtInfo, sys, ctx));

      m_decoration.setBool(*evtInfo, false, sys);

      if (m_preselection && !m_preselection.getBool(*evtInfo, sys))
        continue;

      bool decision = false;

      if (m_electronsHandle && !decision) {
        const xAOD::ElectronContainer *electrons = nullptr;
        ANA_CHECK(m_electronsHandle.retrieve(electrons, sys, ctx));
        for (size_t i = 0; i < electrons->size() && !decision; ++i) {
          const xAOD::Electron* e0 = (*electrons)[i];
          if (m_electronSelection && !m_electronSelection.getBool(*e0, sys)) continue;
          for (size_t j = i + 1; j < electrons->size() && !decision; ++j) {
            const xAOD::Electron* e1 = (*electrons)[j];
            if (m_electronSelection && !m_electronSelection.getBool(*e1, sys)) continue;
            if (e0->charge() == e1->charge()) continue;
            decision = inWindow(mll_reco(e0, e1));
          }
        }
      }

      if (m_muonsHandle && !decision) {
        const xAOD::MuonContainer *muons = nullptr;
        ANA_CHECK(m_muonsHandle.retrieve(muons, sys, ctx));
        for (size_t i = 0; i < muons->size() && !decision; ++i) {
          const xAOD::Muon* m0 = (*muons)[i];
          if (m_muonSelection && !m_muonSelection.getBool(*m0, sys)) continue;
          for (size_t j = i + 1; j < muons->size() && !decision; ++j) {
            const xAOD::Muon* m1 = (*muons)[j];
            if (m_muonSelection && !m_muonSelection.getBool(*m1, sys)) continue;
            if (m0->charge() == m1->charge()) continue;
            decision = inWindow(mll_reco(m0, m1));
          }
        }
      }

      if (m_electronsTruthHandle && !decision) {
        const xAOD::TruthParticleContainer *truthElectrons = nullptr;
        ANA_CHECK(m_electronsTruthHandle.retrieve(truthElectrons, sys, ctx));
        for (size_t i = 0; i < truthElectrons->size() && !decision; ++i) {
          const xAOD::TruthParticle* e0 = (*truthElectrons)[i];
          if (m_electronTruthSelection && !m_electronTruthSelection.getBool(*e0, sys)) continue;
          for (size_t j = i + 1; j < truthElectrons->size() && !decision; ++j) {
            const xAOD::TruthParticle* e1 = (*truthElectrons)[j];
            if (m_electronTruthSelection && !m_electronTruthSelection.getBool(*e1, sys)) continue;
            if (e0->charge() == e1->charge()) continue;
            decision = inWindow(mll_truth(e0, e1, m_useDressedProperties));
          }
        }
      }

      if (m_muonsTruthHandle && !decision) {
        const xAOD::TruthParticleContainer *truthMuons = nullptr;
        ANA_CHECK(m_muonsTruthHandle.retrieve(truthMuons, sys, ctx));
        for (size_t i = 0; i < truthMuons->size() && !decision; ++i) {
          const xAOD::TruthParticle* m0 = (*truthMuons)[i];
          if (m_muonTruthSelection && !m_muonTruthSelection.getBool(*m0, sys)) continue;
          for (size_t j = i + 1; j < truthMuons->size() && !decision; ++j) {
            const xAOD::TruthParticle* m1 = (*truthMuons)[j];
            if (m_muonTruthSelection && !m_muonTruthSelection.getBool(*m1, sys)) continue;
            if (m0->charge() == m1->charge()) continue;
            decision = inWindow(mll_truth(m0, m1, m_useDressedProperties));
          }
        }
      }

      if (m_veto) decision = !decision;
      m_decoration.setBool(*evtInfo, decision, sys);
    }

    return StatusCode::SUCCESS;
  }

} // namespace CP
