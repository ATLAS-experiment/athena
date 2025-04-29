/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#include "TruthParticleLevelAnalysisAlgorithms/ParticleLevelOverlapRemovalAlg.h"

#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>
#include <FourMomUtils/xAODP4Helpers.h>

namespace CP {

StatusCode ParticleLevelOverlapRemovalAlg::initialize() {

  ANA_CHECK(m_jetsKey.initialize());
  ANA_CHECK(m_electronsKey.initialize(SG::AllowEmpty));
  ANA_CHECK(m_muonsKey.initialize(SG::AllowEmpty));
  ANA_CHECK(m_photonsKey.initialize(SG::AllowEmpty));

  if (!m_electronsKey.empty())
    m_decORelectron = m_electronsKey.key() + "." + m_decLabelOR.value();
  if (!m_muonsKey.empty())
    m_decORmuon = m_muonsKey.key() + "." + m_decLabelOR.value();
  if (!m_photonsKey.empty())
    m_decORphoton = m_photonsKey.key() + "." + m_decLabelOR.value();
  m_decORjet = m_jetsKey.key() + "." + m_decLabelOR.value();

  ANA_CHECK(m_decORelectron.initialize(SG::AllowEmpty));
  ANA_CHECK(m_decORmuon.initialize(SG::AllowEmpty));
  ANA_CHECK(m_decORphoton.initialize(SG::AllowEmpty));
  ANA_CHECK(m_decORjet.initialize());

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

float ParticleLevelOverlapRemovalAlg::dressedDeltaR(const xAOD::Jet* p1,
                                                    TLorentzVector& p2,
                                                    bool useRapidity) const {
  if (useRapidity)
    return xAOD::P4Helpers::deltaR(p1->rapidity(), p1->phi(), p2.Rapidity(),
                                   p2.Phi());
  else
    return xAOD::P4Helpers::deltaR(p1->eta(), p1->phi(), p2.Eta(), p2.Phi());
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

  SG::WriteDecorHandle<xAOD::TruthParticleContainer, char> dec_electrons_OR(
      m_decORelectron, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, char> dec_muons_OR(
      m_decORmuon, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, char> dec_photons_OR(
      m_decORphoton, ctx);
  SG::WriteDecorHandle<xAOD::JetContainer, char> dec_jets_OR(m_decORjet, ctx);

  // accessors
  static const SG::AuxElement::ConstAccessor<float> acc_pt_dressed(
      "pt_dressed");
  static const SG::AuxElement::ConstAccessor<float> acc_eta_dressed(
      "eta_dressed");
  static const SG::AuxElement::ConstAccessor<float> acc_phi_dressed(
      "phi_dressed");
  static const SG::AuxElement::ConstAccessor<float> acc_e_dressed("e_dressed");

  // Default decorations: all objects pass!
  for (const auto* jet : *jets) {
    if (m_jetSelection)
      dec_jets_OR(*jet) = m_jetSelection.getBool(*jet);
    else
      dec_jets_OR(*jet) = true;
  }
  if (m_doJetElectronOR) {
    for (const auto* electron : *electrons) {
      if (m_electronSelection)
        dec_electrons_OR(*electron) = m_electronSelection.getBool(*electron);
      else
        dec_electrons_OR(*electron) = true;
    }
  }
  if (m_doJetMuonOR) {
    for (const auto* muon : *muons) {
      if (m_muonSelection)
        dec_muons_OR(*muon) = m_muonSelection.getBool(*muon);
      else
        dec_muons_OR(*muon) = true;
    }
  }
  if (m_doJetPhotonOR) {
    for (const auto* photon : *photons) {
      if (m_photonSelection)
        dec_photons_OR(*photon) = m_photonSelection.getBool(*photon);
      else
        dec_photons_OR(*photon) = true;
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

  for (const auto* jet : *jets) {
    if (m_jetSelection && !m_jetSelection.getBool(*jet))
      continue;
    if (m_doJetMuonOR) {
      for (const auto* muon : *muons) {
        if (m_muonSelection && !m_muonSelection.getBool(*muon))
          continue;
        if (dec_muons_OR(*muon)) {
          if (m_useDressedProperties) {
            TLorentzVector dressed_muon;
            dressed_muon.SetPtEtaPhiE(
                acc_pt_dressed(*muon), acc_eta_dressed(*muon),
                acc_phi_dressed(*muon), acc_e_dressed(*muon));
            if (dressedDeltaR(jet, dressed_muon, m_useRapidity) < 0.4)
              dec_muons_OR(*muon) = false;
          } else {
            if (xAOD::P4Helpers::deltaR(jet, muon, m_useRapidity) < 0.4)
              dec_muons_OR(*muon) = false;
          }
        }
      }
    }
    if (m_doJetElectronOR) {
      for (const auto* electron : *electrons) {
        if (m_electronSelection && !m_electronSelection.getBool(*electron))
          continue;
        if (dec_electrons_OR(*electron)) {
          if (m_useDressedProperties) {
            TLorentzVector dressed_electron;
            dressed_electron.SetPtEtaPhiE(
                acc_pt_dressed(*electron), acc_eta_dressed(*electron),
                acc_phi_dressed(*electron), acc_e_dressed(*electron));
            if (dressedDeltaR(jet, dressed_electron, m_useRapidity) < 0.4)
              dec_electrons_OR(*electron) = false;
          } else {
            if (xAOD::P4Helpers::deltaR(jet, electron, m_useRapidity) < 0.4)
              dec_electrons_OR(*electron) = false;
          }
        }
      }
    }
    if (m_doJetPhotonOR) {
      for (const auto* photon : *photons) {
        if (m_photonSelection && !m_photonSelection.getBool(*photon))
          continue;
        if (dec_photons_OR(*photon)) {
          if (xAOD::P4Helpers::deltaR(jet, photon) < 0.4)
            dec_jets_OR(*jet) = false;
        }
      }
    }
  }

  return StatusCode::SUCCESS;
}

}  // namespace CP
