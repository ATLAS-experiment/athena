/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>
/// @author Steffen Korn <steffen.korn@cern.ch>

#include "PartonHistory/CalcPartonHistory.h"
#include "VectorHelpers/DecoratorHelpers.h"
#include "VectorHelpers/LorentzHelper.h"

namespace CP {
using ROOT::Math::PtEtaPhiMVector;

bool CalcPartonHistory::getW(const std::string& str_lep,
                             const std::string& str_nu, PtEtaPhiMVector& p1,
                             int& pdgId1, PtEtaPhiMVector& p2, int& pdgId2) {
  // Off-shell / non-resonant W reconstruction from lepton + neutrino
  // candidates. Loops over all (lepton, neutrino) pairs and returns the first
  // pair that passes the three conditions below. The caller is responsible for
  // passing the correct lepton/neutrino flavour keys (e.g. MC_lbar + MC_nu for
  // W+).
  std::vector<const xAOD::TruthParticle*> W_offshell_decay1_candidates;
  std::vector<const xAOD::TruthParticle*> W_offshell_decay2_candidates;
  // str_lep/str_nu are full m_particleMap keys (caller passes m_prefix + "_" +
  // ...)
  bool has_candidates =
      (RetrieveParticleInfo(str_lep, W_offshell_decay1_candidates) &&
       RetrieveParticleInfo(str_nu, W_offshell_decay2_candidates));
  if (has_candidates) {
    for (const auto* particle_1 : W_offshell_decay1_candidates) {
      for (const auto* particle_2 : W_offshell_decay2_candidates) {
        // Condition 1: Opposite charge — lepton and neutrino must have
        // opposite-sign pdgIds (e.g. e- has pdgId=11, νe has pdgId=12; product
        // is positive, so we require the product to be negative for a valid W
        // decay pair).
        if ((particle_1->pdgId() * particle_2->pdgId()) > 0)
          continue;
        // Condition 2: Same lepton generation — |pdgId| of the neutrino must
        // be that of the lepton plus 1 (11 & 12, 13 & 14, 15 & 16); a
        // symmetric difference would also accept e.g. (mu, nu_e).
        if (particle_2->absPdgId() - particle_1->absPdgId() != 1)
          continue;
        // Condition 3: Reject shower particles — particles produced during
        // parton showering are assigned UIDs above 10100 by the generator.
        // We require at least one of the pair to be a hard-scatter particle
        // (UID ≤ 10100) to avoid picking up shower leptons.
        if (particle_1->uid() > 10100 && particle_2->uid() > 10100)
          continue;

        p1 = GetPtEtaPhiMfromTruth(particle_1);
        pdgId1 = particle_1->pdgId();
        p2 = GetPtEtaPhiMfromTruth(particle_2);
        pdgId2 = particle_2->pdgId();
        return true;
      }
    }
  }
  return false;
}

void CalcPartonHistory::setW(const std::string& fsr, int nWs) {
  PtEtaPhiMVector W;
  PtEtaPhiMVector Wm, Wm_decay1, Wm_decay2;
  PtEtaPhiMVector Wp, Wp_decay1, Wp_decay2;
  int Wm_decay1_pdgId, Wm_decay2_pdgId;
  int Wp_decay1_pdgId, Wp_decay2_pdgId;

  if (nWs == 1) {
    // W+: pass full m_particleMap keys to getW; bare names to m_dec.*
    bool has_Wp =
        getW(m_prefix + "_" + "MC_lbar_" + fsr, m_prefix + "_" + "MC_nu_" + fsr,
             Wp_decay1, Wp_decay1_pdgId, Wp_decay2, Wp_decay2_pdgId);
    if (has_Wp) {
      Wp = Wp_decay1 + Wp_decay2;
      m_dec.decorateParticle("MC_W_" + fsr, Wp, 24);
      m_dec.decorateParticle("MC_Wdecay1_" + fsr, Wp_decay1, Wp_decay1_pdgId);
      m_dec.decorateParticle("MC_Wdecay2_" + fsr, Wp_decay2, Wp_decay2_pdgId);
    }
    // W-
    bool has_Wm =
        getW(m_prefix + "_" + "MC_l_" + fsr, m_prefix + "_" + "MC_nubar_" + fsr,
             Wm_decay1, Wm_decay1_pdgId, Wm_decay2, Wm_decay2_pdgId);
    if (has_Wm) {
      Wm = Wm_decay1 + Wm_decay2;
      m_dec.decorateParticle("MC_W_" + fsr, Wm, -24);
      m_dec.decorateParticle("MC_Wdecay1_" + fsr, Wm_decay1, Wm_decay1_pdgId);
      m_dec.decorateParticle("MC_Wdecay2_" + fsr, Wm_decay2, Wm_decay2_pdgId);
    }
  } else if (nWs == 2) {
    // W+
    bool has_Wp =
        getW(m_prefix + "_" + "MC_lbar_" + fsr, m_prefix + "_" + "MC_nu_" + fsr,
             Wp_decay1, Wp_decay1_pdgId, Wp_decay2, Wp_decay2_pdgId);
    if (has_Wp) {
      Wp = Wp_decay1 + Wp_decay2;
      m_dec.decorateParticle("MC_W1_" + fsr, Wp, 24);
      m_dec.decorateParticle("MC_W1decay1_" + fsr, Wp_decay1, Wp_decay1_pdgId);
      m_dec.decorateParticle("MC_W1decay2_" + fsr, Wp_decay2, Wp_decay2_pdgId);
    }
    // W-
    bool has_Wm =
        getW(m_prefix + "_" + "MC_l_" + fsr, m_prefix + "_" + "MC_nubar_" + fsr,
             Wm_decay1, Wm_decay1_pdgId, Wm_decay2, Wm_decay2_pdgId);
    if (has_Wm) {
      Wm = Wm_decay1 + Wm_decay2;
      m_dec.decorateParticle("MC_W2_" + fsr, Wm, -24);
      m_dec.decorateParticle("MC_W2decay1_" + fsr, Wm_decay1, Wm_decay1_pdgId);
      m_dec.decorateParticle("MC_W2decay2_" + fsr, Wm_decay2, Wm_decay2_pdgId);
    }
  }
}

void CalcPartonHistory::FillWPartonHistory(const std::string& parent, int nWs,
                                           const std::string& mode) {
  std::string parentstring = parent.empty() ? "" : "_from_" + parent;
  // Bare suffix for FillGenericPartonHistory (it prepends m_prefix + "_"
  // internally).
  std::string prefix = "MC_" + (parent.empty() ? "W" : parent + "_W");

  if (mode == "resonant") {
    if (nWs == 1) {
      if (parent.empty())
        m_dec.decorateCustom(
            "MC_W_IsOnShell",
            ExistsInMap(m_prefix + "_" + prefix + "_beforeFSR") ? 1 : 0);
      FillGenericPartonHistory(prefix + "_beforeFSR",
                               "MC_W_beforeFSR" + parentstring, 0);
      FillGenericPartonHistory(prefix + "Decay1_beforeFSR",
                               "MC_Wdecay1_beforeFSR" + parentstring, 0);
      FillGenericPartonHistory(prefix + "Decay2_beforeFSR",
                               "MC_Wdecay2_beforeFSR" + parentstring, 0);
      FillGenericPartonHistory(prefix + "_afterFSR",
                               "MC_W_afterFSR" + parentstring, 0);
      FillGenericPartonHistory(prefix + "Decay1_afterFSR",
                               "MC_Wdecay1_afterFSR" + parentstring, 0);
      FillGenericPartonHistory(prefix + "Decay2_afterFSR",
                               "MC_Wdecay2_afterFSR" + parentstring, 0);
    } else {
      for (int idx = 0; idx < nWs; idx++) {
        const std::string idxStr = std::to_string(idx + 1);
        if (parent.empty())
          m_dec.decorateCustom("MC_W" + idxStr + "_IsOnShell", 1);
        FillGenericPartonHistory(prefix + "_beforeFSR",
                                 "MC_W" + idxStr + "_beforeFSR" + parentstring,
                                 0);
        FillGenericPartonHistory(
            prefix + "Decay1_beforeFSR",
            "MC_W" + idxStr + "decay1_beforeFSR" + parentstring, 0);
        FillGenericPartonHistory(
            prefix + "Decay2_beforeFSR",
            "MC_W" + idxStr + "decay2_beforeFSR" + parentstring, 0);
        FillGenericPartonHistory(prefix + "_afterFSR",
                                 "MC_W" + idxStr + "_afterFSR" + parentstring,
                                 0);
        FillGenericPartonHistory(
            prefix + "Decay1_afterFSR",
            "MC_W" + idxStr + "decay1_afterFSR" + parentstring, 0);
        FillGenericPartonHistory(
            prefix + "Decay2_afterFSR",
            "MC_W" + idxStr + "decay2_afterFSR" + parentstring, 0);
      }
    }
  } else if (mode == "non_resonant") {
    if (parent.empty()) {
      if (nWs == 1) {
        m_dec.decorateCustom("MC_W_IsOnShell", 0);
      } else {
        for (int idx = 0; idx < nWs; idx++)
          m_dec.decorateCustom("MC_W" + std::to_string(idx + 1) + "_IsOnShell",
                               0);
      }
    }
    setW("beforeFSR", nWs);
    setW("afterFSR", nWs);
  }
}
}  // namespace CP
