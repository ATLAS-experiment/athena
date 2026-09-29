/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>
/// @author Steffen Korn <steffen.korn@cern.ch>

#include "PartonHistory/CalcPartonHistory.h"
#include "VectorHelpers/DecoratorHelpers.h"
#include "VectorHelpers/LorentzHelper.h"
#include "xAODTruth/TruthParticleContainer.h"

#include <array>
#include <utility>

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
  // {particle map key, decoration name}, both without the "MC_" prefix and
  // the FSR-stage suffix.
  static constexpr std::array<std::pair<const char*, const char*>, 7>
      higgsParticles{{{"H", "H"},
                      {"HDecay1", "Hdecay1"},
                      {"HDecay2", "Hdecay2"},
                      {"HDecay1Decay1", "Hdecay1_decay1"},
                      {"HDecay1Decay2", "Hdecay1_decay2"},
                      {"HDecay2Decay1", "Hdecay2_decay1"},
                      {"HDecay2Decay2", "Hdecay2_decay2"}}};
  static constexpr std::array<const char*, 2> fsrStages{
      {"_beforeFSR", "_afterFSR"}};

  // Defaults for all branches regardless of mode.
  for (const char* fsr : fsrStages)
    for (const auto& [key, decoration] : higgsParticles)
      m_dec.decorateDefault("MC_" + std::string(decoration) + fsr);

  if (mode == "resonant" || mode == "single_top") {
    // FillGenericPartonHistory retrieval strings are bare suffixes.
    for (const char* fsr : fsrStages) {
      for (const auto& [key, decoration] : higgsParticles) {
        const std::string decorationName =
            "MC_" + std::string(decoration) + fsr;
        if (mode == "resonant") {
          FillGenericPartonHistory("MC_" + std::string(key) + fsr,
                                   decorationName, 0);
        } else {
          FillGenericPartonHistory({"MC_" + std::string(key) + fsr,
                                    "MC_b_" + std::string(key) + fsr,
                                    "MC_bbar_" + std::string(key) + fsr},
                                   decorationName, 0);
        }
      }
    }
  } else if (mode == "non_resonant_WW") {
    setHiggs("beforeFSR");
    setHiggs("afterFSR");
  }
}
}  // namespace CP
