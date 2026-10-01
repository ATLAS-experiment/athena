/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#include "TruthParticleLevelAnalysisAlgorithms/ParticleLevelOverlapRemovalAlg.h"

#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>
#include <FourMomUtils/xAODP4Helpers.h>
#include <TLorentzVector.h>

#include <optional>
#include <utility>
#include <vector>

namespace CP {

StatusCode ParticleLevelOverlapRemovalAlg::initialize() {

  if (m_doJetElectronOR.value() == m_electronsKey.empty()) {
    ANA_MSG_ERROR("doJetElectronOR is " << (m_doJetElectronOR.value() ? "true" : "false")
                  << " but the input container is "
                  << (m_electronsKey.empty() ? "empty" : "set"));
    return StatusCode::FAILURE;
  }
  if (m_doJetMuonOR.value() == m_muonsKey.empty()) {
    ANA_MSG_ERROR("doJetMuonOR is " << (m_doJetMuonOR.value() ? "true" : "false")
                  << " but the input container is "
                  << (m_muonsKey.empty() ? "empty" : "set"));
    return StatusCode::FAILURE;
  }
  if (m_doJetPhotonOR.value() == m_photonsKey.empty()) {
    ANA_MSG_ERROR("doJetPhotonOR is " << (m_doJetPhotonOR.value() ? "true" : "false")
                  << " but the input container is "
                  << (m_photonsKey.empty() ? "empty" : "set"));
    return StatusCode::FAILURE;
  }

  ANA_CHECK(m_jetsKey.initialize());
  ANA_CHECK(m_electronsKey.initialize(SG::AllowEmpty));
  ANA_CHECK(m_muonsKey.initialize(SG::AllowEmpty));
  ANA_CHECK(m_photonsKey.initialize(SG::AllowEmpty));

  ANA_CHECK(m_decORelectron.initialize(SG::AllowEmpty));
  ANA_CHECK(m_decORmuon.initialize(SG::AllowEmpty));
  ANA_CHECK(m_decORphoton.initialize(SG::AllowEmpty));
  ANA_CHECK(m_decORjet.initialize());

  ANA_CHECK(m_ptDressedElectronKey.initialize(m_doJetElectronOR && m_useDressedProperties));
  ANA_CHECK(m_etaDressedElectronKey.initialize(m_doJetElectronOR && m_useDressedProperties));
  ANA_CHECK(m_phiDressedElectronKey.initialize(m_doJetElectronOR && m_useDressedProperties));
  ANA_CHECK(m_eDressedElectronKey.initialize(m_doJetElectronOR && m_useDressedProperties));

  ANA_CHECK(m_ptDressedMuonKey.initialize(m_doJetMuonOR && m_useDressedProperties));
  ANA_CHECK(m_etaDressedMuonKey.initialize(m_doJetMuonOR && m_useDressedProperties));
  ANA_CHECK(m_phiDressedMuonKey.initialize(m_doJetMuonOR && m_useDressedProperties));
  ANA_CHECK(m_eDressedMuonKey.initialize(m_doJetMuonOR && m_useDressedProperties));

  if (!m_jetSelection.empty())
    ANA_CHECK(m_jetSelection.initialize());
  if (!m_electronSelection.empty())
    ANA_CHECK(m_electronSelection.initialize());
  if (!m_muonSelection.empty())
    ANA_CHECK(m_muonSelection.initialize());
  if (!m_photonSelection.empty())
    ANA_CHECK(m_photonSelection.initialize());

  return StatusCode::SUCCESS;
}

float ParticleLevelOverlapRemovalAlg::dressedDeltaR(const xAOD::Jet* jet,
                                                    double rapidityOrEta,
                                                    double phi) const {
  if (m_useRapidity)
    return xAOD::P4Helpers::deltaR(jet->rapidity(), jet->phi(), rapidityOrEta,
                                   phi);
  else
    return xAOD::P4Helpers::deltaR(jet->eta(), jet->phi(), rapidityOrEta, phi);
}

StatusCode ParticleLevelOverlapRemovalAlg::execute(const EventContext &ctx) const {
  SG::ReadHandle<xAOD::TruthParticleContainer> electrons, muons, photons;
  if (m_doJetElectronOR)
    electrons = SG::makeHandle(m_electronsKey, ctx);
  if (m_doJetMuonOR)
    muons = SG::makeHandle(m_muonsKey, ctx);
  if (m_doJetPhotonOR)
    photons = SG::makeHandle(m_photonsKey, ctx);
  SG::ReadHandle<xAOD::JetContainer> jets(m_jetsKey, ctx);

  // the lepton/photon decoration handles only exist if the respective OR is
  // enabled (their keys are empty otherwise)
  std::optional<SG::WriteDecorHandle<xAOD::TruthParticleContainer, char>>
      dec_electrons_OR, dec_muons_OR, dec_photons_OR;
  if (m_doJetElectronOR)
    dec_electrons_OR.emplace(m_decORelectron, ctx);
  if (m_doJetMuonOR)
    dec_muons_OR.emplace(m_decORmuon, ctx);
  if (m_doJetPhotonOR)
    dec_photons_OR.emplace(m_decORphoton, ctx);
  SG::WriteDecorHandle<xAOD::JetContainer, char> dec_jets_OR(m_decORjet, ctx);

  // accessors for the dressed lepton kinematics (only bound when needed)
  std::optional<SG::ReadDecorHandle<xAOD::TruthParticleContainer, float>>
      acc_pt_dressed_e, acc_eta_dressed_e, acc_phi_dressed_e, acc_e_dressed_e;
  if (m_doJetElectronOR && m_useDressedProperties) {
    acc_pt_dressed_e.emplace(m_ptDressedElectronKey, ctx);
    acc_eta_dressed_e.emplace(m_etaDressedElectronKey, ctx);
    acc_phi_dressed_e.emplace(m_phiDressedElectronKey, ctx);
    acc_e_dressed_e.emplace(m_eDressedElectronKey, ctx);
  }
  std::optional<SG::ReadDecorHandle<xAOD::TruthParticleContainer, float>>
      acc_pt_dressed_m, acc_eta_dressed_m, acc_phi_dressed_m, acc_e_dressed_m;
  if (m_doJetMuonOR && m_useDressedProperties) {
    acc_pt_dressed_m.emplace(m_ptDressedMuonKey, ctx);
    acc_eta_dressed_m.emplace(m_etaDressedMuonKey, ctx);
    acc_phi_dressed_m.emplace(m_phiDressedMuonKey, ctx);
    acc_e_dressed_m.emplace(m_eDressedMuonKey, ctx);
  }

  // dressed (rapidity or eta, phi) of a lepton, used for the DeltaR
  auto dressedRapidityOrEtaPhi =
      [&](const xAOD::TruthParticle& lepton,
          const SG::ReadDecorHandle<xAOD::TruthParticleContainer, float>& acc_pt,
          const SG::ReadDecorHandle<xAOD::TruthParticleContainer, float>& acc_eta,
          const SG::ReadDecorHandle<xAOD::TruthParticleContainer, float>& acc_phi,
          const SG::ReadDecorHandle<xAOD::TruthParticleContainer, float>& acc_e) {
        TLorentzVector dressed;
        dressed.SetPtEtaPhiE(acc_pt(lepton), acc_eta(lepton), acc_phi(lepton),
                             acc_e(lepton));
        return std::pair<double, double>{
            m_useRapidity ? dressed.Rapidity() : dressed.Eta(), dressed.Phi()};
      };
  // per-lepton dressed kinematics, indexed by position in the container
  // (only filled for selected leptons when using dressed properties)
  std::vector<std::pair<double, double>> dressed_electrons, dressed_muons;

  // Default decorations: all objects pass!
  for (const auto* jet : *jets) {
    if (m_jetSelection)
      dec_jets_OR(*jet) = m_jetSelection.getBool(*jet);
    else
      dec_jets_OR(*jet) = true;
  }
  if (m_doJetElectronOR) {
    dressed_electrons.resize(electrons->size());
    for (std::size_t i = 0; i < electrons->size(); ++i) {
      const auto* electron = (*electrons)[i];
      if (m_electronSelection)
        (*dec_electrons_OR)(*electron) = m_electronSelection.getBool(*electron);
      else
        (*dec_electrons_OR)(*electron) = true;
      if (m_useDressedProperties && (*dec_electrons_OR)(*electron))
        dressed_electrons[i] = dressedRapidityOrEtaPhi(
            *electron, *acc_pt_dressed_e, *acc_eta_dressed_e,
            *acc_phi_dressed_e, *acc_e_dressed_e);
    }
  }
  if (m_doJetMuonOR) {
    dressed_muons.resize(muons->size());
    for (std::size_t i = 0; i < muons->size(); ++i) {
      const auto* muon = (*muons)[i];
      if (m_muonSelection)
        (*dec_muons_OR)(*muon) = m_muonSelection.getBool(*muon);
      else
        (*dec_muons_OR)(*muon) = true;
      if (m_useDressedProperties && (*dec_muons_OR)(*muon))
        dressed_muons[i] = dressedRapidityOrEtaPhi(
            *muon, *acc_pt_dressed_m, *acc_eta_dressed_m, *acc_phi_dressed_m,
            *acc_e_dressed_m);
    }
  }
  if (m_doJetPhotonOR) {
    for (const auto* photon : *photons) {
      if (m_photonSelection)
        (*dec_photons_OR)(*photon) = m_photonSelection.getBool(*photon);
      else
        (*dec_photons_OR)(*photon) = true;
    }
  }

  // ----------------------
  // OVERLAP REMOVAL
  // ----------------------
  // Removal Steps:
  //   1. Jets & Muons:
  //      Remove Muons with dR < 0.4
  //   2. Jets & Electrons:
  //      Remove Electrons with dR < 0.4
  //   3. Photons & Jets:
  //      Remove Jets with dR < 0.4
  // The steps are interleaved per jet (not run sequentially over all jets),
  // and the lepton removal uses all selected jets, regardless of whether
  // the jet itself is removed by the photon-jet overlap removal.

  for (const auto* jet : *jets) {
    if (m_jetSelection && !m_jetSelection.getBool(*jet))
      continue;
    if (m_doJetMuonOR) {
      for (std::size_t i = 0; i < muons->size(); ++i) {
        const auto* muon = (*muons)[i];
        if ((*dec_muons_OR)(*muon)) {
          if (m_useDressedProperties) {
            const auto& [rapidityOrEta, phi] = dressed_muons[i];
            if (dressedDeltaR(jet, rapidityOrEta, phi) < 0.4)
              (*dec_muons_OR)(*muon) = false;
          } else {
            if (xAOD::P4Helpers::deltaR(jet, muon, m_useRapidity) < 0.4)
              (*dec_muons_OR)(*muon) = false;
          }
        }
      }
    }
    if (m_doJetElectronOR) {
      for (std::size_t i = 0; i < electrons->size(); ++i) {
        const auto* electron = (*electrons)[i];
        if ((*dec_electrons_OR)(*electron)) {
          if (m_useDressedProperties) {
            const auto& [rapidityOrEta, phi] = dressed_electrons[i];
            if (dressedDeltaR(jet, rapidityOrEta, phi) < 0.4)
              (*dec_electrons_OR)(*electron) = false;
          } else {
            if (xAOD::P4Helpers::deltaR(jet, electron, m_useRapidity) < 0.4)
              (*dec_electrons_OR)(*electron) = false;
          }
        }
      }
    }
    if (m_doJetPhotonOR) {
      for (const auto* photon : *photons) {
        if ((*dec_photons_OR)(*photon)) {
          if (xAOD::P4Helpers::deltaR(jet, photon, m_useRapidity) < 0.4) {
            dec_jets_OR(*jet) = false;
            break;
          }
        }
      }
    }
  }

  return StatusCode::SUCCESS;
}

}  // namespace CP
