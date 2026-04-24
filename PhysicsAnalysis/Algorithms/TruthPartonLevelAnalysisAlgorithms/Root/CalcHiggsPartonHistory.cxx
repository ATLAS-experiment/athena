/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>
/// @author Steffen Korn <steffen.korn@cern.ch>

#include "PartonHistory/CalcPartonHistory.h"
#include "VectorHelpers/DecoratorHelpers.h"
#include "VectorHelpers/LorentzHelper.h"
#include "xAODTruth/TruthParticleContainer.h"

namespace CP {
using ROOT::Math::PtEtaPhiMVector;

void CalcPartonHistory::setHiggs(const std::string& fsr) {
  PtEtaPhiMVector H;
  PtEtaPhiMVector Wm, Wm_decay1, Wm_decay2;
  PtEtaPhiMVector Wp, Wp_decay1, Wp_decay2;
  int Wm_decay1_pdgId, Wm_decay2_pdgId;
  int Wp_decay1_pdgId, Wp_decay2_pdgId;

  // W+: pass full m_particleMap keys to getW; bare names to m_dec.*
  bool has_Wp =
      getW(m_prefix + "_" + "MC_lbar_" + fsr, m_prefix + "_" + "MC_nu_" + fsr,
           Wp_decay1, Wp_decay1_pdgId, Wp_decay2, Wp_decay2_pdgId);
  if (has_Wp) {
    Wp = Wp_decay1 + Wp_decay2;
    m_dec.decorateParticle("MC_Hdecay1_" + fsr, Wp, 24);
    m_dec.decorateParticle("MC_Hdecay1_decay1_" + fsr, Wp_decay1,
                           Wp_decay1_pdgId);
    m_dec.decorateParticle("MC_Hdecay1_decay2_" + fsr, Wp_decay2,
                           Wp_decay2_pdgId);
  }
  // W-
  bool has_Wm =
      getW(m_prefix + "_" + "MC_l_" + fsr, m_prefix + "_" + "MC_nubar_" + fsr,
           Wm_decay1, Wm_decay1_pdgId, Wm_decay2, Wm_decay2_pdgId);
  if (has_Wm) {
    Wm = Wm_decay1 + Wm_decay2;
    m_dec.decorateParticle("MC_Hdecay2_" + fsr, Wm, -24);
    m_dec.decorateParticle("MC_Hdecay2_decay1_" + fsr, Wm_decay1,
                           Wm_decay1_pdgId);
    m_dec.decorateParticle("MC_Hdecay2_decay2_" + fsr, Wm_decay2,
                           Wm_decay2_pdgId);
  }
  if (has_Wm && has_Wp) {
    H = Wp + Wm;
    m_dec.decorateParticle("MC_H_" + fsr, H, 25);
  }
}

void CalcPartonHistory::FillHiggsPartonHistory(const std::string& mode) {
  PtEtaPhiMVector v;
  int pdgId = 0;

  // Defaults for all branches regardless of mode.
  m_dec.decorateDefault("MC_H_beforeFSR");
  m_dec.decorateDefault("MC_Hdecay1_beforeFSR");
  m_dec.decorateDefault("MC_Hdecay2_beforeFSR");
  m_dec.decorateDefault("MC_Hdecay1_decay1_beforeFSR");
  m_dec.decorateDefault("MC_Hdecay1_decay2_beforeFSR");
  m_dec.decorateDefault("MC_Hdecay2_decay1_beforeFSR");
  m_dec.decorateDefault("MC_Hdecay2_decay2_beforeFSR");
  m_dec.decorateDefault("MC_H_afterFSR");
  m_dec.decorateDefault("MC_Hdecay1_afterFSR");
  m_dec.decorateDefault("MC_Hdecay2_afterFSR");
  m_dec.decorateDefault("MC_Hdecay1_decay1_afterFSR");
  m_dec.decorateDefault("MC_Hdecay1_decay2_afterFSR");
  m_dec.decorateDefault("MC_Hdecay2_decay1_afterFSR");
  m_dec.decorateDefault("MC_Hdecay2_decay2_afterFSR");

  if (mode == "resonant") {
    // RetrieveParticleInfo uses full m_particleMap keys (with m_prefix).
    // m_dec.decorateParticle uses bare names.
    if (RetrieveParticleInfo(m_prefix + "_" + "MC_H_beforeFSR", v, pdgId))
      m_dec.decorateParticle("MC_H_beforeFSR", v, pdgId);
    if (RetrieveParticleInfo(m_prefix + "_" + "MC_HDecay1_beforeFSR", v, pdgId))
      m_dec.decorateParticle("MC_Hdecay1_beforeFSR", v, pdgId);
    if (RetrieveParticleInfo(m_prefix + "_" + "MC_HDecay2_beforeFSR", v, pdgId))
      m_dec.decorateParticle("MC_Hdecay2_beforeFSR", v, pdgId);
    if (RetrieveParticleInfo(m_prefix + "_" + "MC_HDecay1Decay1_beforeFSR", v,
                             pdgId))
      m_dec.decorateParticle("MC_Hdecay1_decay1_beforeFSR", v, pdgId);
    if (RetrieveParticleInfo(m_prefix + "_" + "MC_HDecay1Decay2_beforeFSR", v,
                             pdgId))
      m_dec.decorateParticle("MC_Hdecay1_decay2_beforeFSR", v, pdgId);
    if (RetrieveParticleInfo(m_prefix + "_" + "MC_HDecay2Decay1_beforeFSR", v,
                             pdgId))
      m_dec.decorateParticle("MC_Hdecay2_decay1_beforeFSR", v, pdgId);
    if (RetrieveParticleInfo(m_prefix + "_" + "MC_HDecay2Decay2_beforeFSR", v,
                             pdgId))
      m_dec.decorateParticle("MC_Hdecay2_decay2_beforeFSR", v, pdgId);
    if (RetrieveParticleInfo(m_prefix + "_" + "MC_H_afterFSR", v, pdgId))
      m_dec.decorateParticle("MC_H_afterFSR", v, pdgId);
    if (RetrieveParticleInfo(m_prefix + "_" + "MC_HDecay1_afterFSR", v, pdgId))
      m_dec.decorateParticle("MC_Hdecay1_afterFSR", v, pdgId);
    if (RetrieveParticleInfo(m_prefix + "_" + "MC_HDecay2_afterFSR", v, pdgId))
      m_dec.decorateParticle("MC_Hdecay2_afterFSR", v, pdgId);
    if (RetrieveParticleInfo(m_prefix + "_" + "MC_HDecay1Decay1_afterFSR", v,
                             pdgId))
      m_dec.decorateParticle("MC_Hdecay1_decay1_afterFSR", v, pdgId);
    if (RetrieveParticleInfo(m_prefix + "_" + "MC_HDecay1Decay2_afterFSR", v,
                             pdgId))
      m_dec.decorateParticle("MC_Hdecay1_decay2_afterFSR", v, pdgId);
    if (RetrieveParticleInfo(m_prefix + "_" + "MC_HDecay2Decay1_afterFSR", v,
                             pdgId))
      m_dec.decorateParticle("MC_Hdecay2_decay1_afterFSR", v, pdgId);
    if (RetrieveParticleInfo(m_prefix + "_" + "MC_HDecay2Decay2_afterFSR", v,
                             pdgId))
      m_dec.decorateParticle("MC_Hdecay2_decay2_afterFSR", v, pdgId);
  } else if (mode == "single_top") {
    // FillGenericPartonHistory retrieval strings are bare suffixes.
    FillGenericPartonHistory(
        {"MC_H_beforeFSR", "MC_b_H_beforeFSR", "MC_bbar_H_beforeFSR"},
        "MC_H_beforeFSR", 0);
    FillGenericPartonHistory({"MC_HDecay1_beforeFSR", "MC_b_HDecay1_beforeFSR",
                              "MC_bbar_HDecay1_beforeFSR"},
                             "MC_Hdecay1_beforeFSR", 0);
    FillGenericPartonHistory({"MC_HDecay2_beforeFSR", "MC_b_HDecay2_beforeFSR",
                              "MC_bbar_HDecay2_beforeFSR"},
                             "MC_Hdecay2_beforeFSR", 0);
    FillGenericPartonHistory(
        {"MC_HDecay1Decay1_beforeFSR", "MC_b_HDecay1Decay1_beforeFSR",
         "MC_bbar_HDecay1Decay1_beforeFSR"},
        "MC_Hdecay1_decay1_beforeFSR", 0);
    FillGenericPartonHistory(
        {"MC_HDecay1Decay2_beforeFSR", "MC_b_HDecay1Decay2_beforeFSR",
         "MC_bbar_HDecay1Decay2_beforeFSR"},
        "MC_Hdecay1_decay2_beforeFSR", 0);
    FillGenericPartonHistory(
        {"MC_HDecay2Decay1_beforeFSR", "MC_b_HDecay2Decay1_beforeFSR",
         "MC_bbar_HDecay2Decay1_beforeFSR"},
        "MC_Hdecay2_decay1_beforeFSR", 0);
    FillGenericPartonHistory(
        {"MC_HDecay2Decay2_beforeFSR", "MC_b_HDecay2Decay2_beforeFSR",
         "MC_bbar_HDecay2Decay2_beforeFSR"},
        "MC_Hdecay2_decay2_beforeFSR", 0);
    FillGenericPartonHistory(
        {"MC_H_afterFSR", "MC_b_H_afterFSR", "MC_bbar_H_afterFSR"},
        "MC_H_afterFSR", 0);
    FillGenericPartonHistory({"MC_HDecay1_afterFSR", "MC_b_HDecay1_afterFSR",
                              "MC_bbar_HDecay1_afterFSR"},
                             "MC_Hdecay1_afterFSR", 0);
    FillGenericPartonHistory({"MC_HDecay2_afterFSR", "MC_b_HDecay2_afterFSR",
                              "MC_bbar_HDecay2_afterFSR"},
                             "MC_Hdecay2_afterFSR", 0);
    FillGenericPartonHistory(
        {"MC_HDecay1Decay1_afterFSR", "MC_b_HDecay1Decay1_afterFSR",
         "MC_bbar_HDecay1Decay1_afterFSR"},
        "MC_Hdecay1_decay1_afterFSR", 0);
    FillGenericPartonHistory(
        {"MC_HDecay1Decay2_afterFSR", "MC_b_HDecay1Decay2_afterFSR",
         "MC_bbar_HDecay1Decay2_afterFSR"},
        "MC_Hdecay1_decay2_afterFSR", 0);
    FillGenericPartonHistory(
        {"MC_HDecay2Decay1_afterFSR", "MC_b_HDecay2Decay1_afterFSR",
         "MC_bbar_HDecay2Decay1_afterFSR"},
        "MC_Hdecay2_decay1_afterFSR", 0);
    FillGenericPartonHistory(
        {"MC_HDecay2Decay2_afterFSR", "MC_b_HDecay2Decay2_afterFSR",
         "MC_bbar_HDecay2Decay2_afterFSR"},
        "MC_Hdecay2_decay2_afterFSR", 0);
  } else if (mode == "non_resonant_WW") {
    setHiggs("beforeFSR");
    setHiggs("afterFSR");
  }
}
}  // namespace CP
