/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>
/// @author Steffen Korn <steffen.korn@cern.ch>

#include <array>
#include <utility>

#include "PartonHistory/CalcPartonHistory.h"
#include "VectorHelpers/DecoratorHelpers.h"

namespace CP {
using ROOT::Math::PtEtaPhiMVector;

void CalcPartonHistory::FillXPartonHistory(const std::string& parent, 
                                           const std::string& symbolX) {
  PtEtaPhiMVector v;
  int pdgId = 0;
  const std::array<std::string, 2> fsrStates{ "beforeFSR", "afterFSR" };
  const std::array<std::pair<std::string, std::string>, 3> particleTags{{
    {"", "X"},
    {"Decay1", "Xdecay1"},
    {"Decay2", "Xdecay2"},
  }};

  for (const auto& fsrState : fsrStates) {
    for (const auto& [sourceSuffix, outputSuffix] : particleTags) {
      const auto outputTag = "MC_" + outputSuffix + "_" + fsrState + "_from_" + parent;
      const auto sourcePrefix = "MC_" + parent + "_";

      if (RetrieveParticleInfo(m_prefix + "_" + sourcePrefix + "W" + sourceSuffix + "_" + fsrState, v, pdgId))
        m_dec.decorateParticle(outputTag, v, pdgId);
      else if (RetrieveParticleInfo(m_prefix + "_" + sourcePrefix + symbolX + sourceSuffix + "_" + fsrState, v, pdgId))
        m_dec.decorateParticle(outputTag, v, pdgId);
      else
        m_dec.decorateDefault(outputTag);
    }
  }
}

void CalcPartonHistory::FillTopPartonHistory(bool fcnc) {
  FillGenericPartonHistory("MC_t_beforeFSR", "MC_t_beforeFSR", 0);
  FillGenericPartonHistory("MC_t_afterFSR", "MC_t_afterFSR", 0);
  if (fcnc) {
    PtEtaPhiMVector v;
    int pdgId = 0;
    const std::array<std::string, 2> fsrStates{ "beforeFSR", "afterFSR" };
    const std::array<std::string, 3> flavors{ "b", "c", "u" }; // The u-quark is currently not stored in the DAOD_PHYS
                                                                           // truth record, but it's kept here for future-proofing.
    for (const auto& fsrState : fsrStates) {
      const auto outputTag = "MC_q_" + fsrState + "_from_t";
      bool found = false;
      for (const auto& flavor : flavors) {
        const auto key = m_prefix + "_" + ("MC_t_" + flavor + "_" + fsrState);
        if (RetrieveParticleInfo(key, v, pdgId)) {
          m_dec.decorateParticle(outputTag, v, pdgId);
          found = true;
          break;
        }
      }
      if (!found) m_dec.decorateDefault(outputTag);
    }
    FillXPartonHistory("t");
  } else {
    FillGenericPartonHistory("MC_t_b_beforeFSR", "MC_b_beforeFSR_from_t", 0);
    FillGenericPartonHistory("MC_t_b_afterFSR", "MC_b_afterFSR_from_t", 0);
    FillWPartonHistory("t");
  }
}

void CalcPartonHistory::FillAntiTopPartonHistory(bool fcnc) {
  FillGenericPartonHistory("MC_tbar_beforeFSR", "MC_tbar_beforeFSR", 0);
  FillGenericPartonHistory("MC_tbar_afterFSR", "MC_tbar_afterFSR", 0);
  if (fcnc) {
    PtEtaPhiMVector v;
    int pdgId = 0;
    const std::array<std::string, 2> fsrStates{ "beforeFSR", "afterFSR" };
    const std::array<std::string, 3> flavors{ "bbar", "cbar", "ubar" }; // "ubar" kept for future-proofing

    for (const auto& fsrState : fsrStates) {
      const auto outputTag = "MC_qbar_" + fsrState + "_from_tbar";
      bool found = false;
      for (const auto& flavor : flavors) {
        const auto key = m_prefix + "_" + ("MC_tbar_" + flavor + "_" + fsrState);
        if (RetrieveParticleInfo(key, v, pdgId)) {
          m_dec.decorateParticle(outputTag, v, pdgId);
          found = true;
          break;
        }
      }
      if (!found) m_dec.decorateDefault(outputTag);
    }
    FillXPartonHistory("tbar");
  } else {
    FillGenericPartonHistory("MC_tbar_bbar_beforeFSR",
                             "MC_bbar_beforeFSR_from_tbar", 0);
    FillGenericPartonHistory("MC_tbar_bbar_afterFSR",
                             "MC_bbar_afterFSR_from_tbar", 0);
    FillWPartonHistory("tbar");
  }
}

void CalcPartonHistory::FillTtbarPartonHistory(bool fcnc) {
  std:: string qSymbol = fcnc ? "q" : "b";
  std:: string qbarSymbol = fcnc ? "qbar" : "bbar";
  std::string bosonSymbol = fcnc ? "X" : "W";
  // Assumes FillTopPartonHistory and FillAntiTopPartonHistory have already run.
  PtEtaPhiMVector ttbar;
  PtEtaPhiMVector t_beforeFSR, tbar_beforeFSR, t_afterFSR, tbar_afterFSR;
  PtEtaPhiMVector WpDecay1, WpDecay2, WmDecay1, WmDecay2, b, bbar;

  // m_dec.decorate* takes bare names; Retrievep4 takes full m_particleMap keys.
  m_dec.decorateDefaultNoPdgId("MC_ttbar_beforeFSR");
  m_dec.decorateDefaultNoPdgId("MC_ttbar_afterFSR");
  m_dec.decorateDefaultNoPdgId("MC_ttbar_fromDecay_beforeFSR");
  m_dec.decorateDefaultNoPdgId("MC_ttbar_fromDecay_afterFSR");

  if (Retrievep4(m_prefix + "_" + "MC_t_beforeFSR", t_beforeFSR) &&
      Retrievep4(m_prefix + "_" + "MC_tbar_beforeFSR", tbar_beforeFSR)) {
    ttbar = t_beforeFSR + tbar_beforeFSR;
    m_dec.decorateParticle("MC_ttbar_beforeFSR", ttbar);
  }

  if (Retrievep4(m_prefix + "_" + "MC_t_afterFSR", t_afterFSR) &&
      Retrievep4(m_prefix + "_" + "MC_tbar_afterFSR", tbar_afterFSR)) {
    ttbar = t_afterFSR + tbar_afterFSR;
    m_dec.decorateParticle("MC_ttbar_afterFSR", ttbar);
  }

  if (Retrievep4(m_prefix + "_" + "MC_t_" + bosonSymbol + "Decay1_beforeFSR", WpDecay1) &&
      Retrievep4(m_prefix + "_" + "MC_t_" + bosonSymbol + "Decay2_beforeFSR", WpDecay2) &&
      Retrievep4(m_prefix + "_" + "MC_tbar_" + bosonSymbol + "Decay1_beforeFSR", WmDecay1) &&
      Retrievep4(m_prefix + "_" + "MC_tbar_" + bosonSymbol + "Decay2_beforeFSR", WmDecay2) &&
      Retrievep4(m_prefix + "_" + "MC_t_" + qSymbol + "_beforeFSR", b) &&
      Retrievep4(m_prefix + "_" + "MC_tbar_" + qbarSymbol + "_beforeFSR", bbar)) {
    ttbar = WpDecay1 + WpDecay2 + WmDecay1 + WmDecay2 + b + bbar;
    m_dec.decorateParticle("MC_ttbar_fromDecay_beforeFSR", ttbar);
  }

  if (Retrievep4(m_prefix + "_" + "MC_t_" + bosonSymbol + "Decay1_afterFSR", WpDecay1) &&
      Retrievep4(m_prefix + "_" + "MC_t_" + bosonSymbol + "Decay2_afterFSR", WpDecay2) &&
      Retrievep4(m_prefix + "_" + "MC_tbar_" + bosonSymbol + "Decay1_afterFSR", WmDecay1) &&
      Retrievep4(m_prefix + "_" + "MC_tbar_" + bosonSymbol + "Decay2_afterFSR", WmDecay2) &&
      Retrievep4(m_prefix + "_" + "MC_t_" + qSymbol + "_afterFSR", b) &&
      Retrievep4(m_prefix + "_" + "MC_tbar_" + qbarSymbol + "_afterFSR", bbar)) {
    ttbar = WpDecay1 + WpDecay2 + WmDecay1 + WmDecay2 + b + bbar;
    m_dec.decorateParticle("MC_ttbar_fromDecay_afterFSR", ttbar);
  }
}
}  // namespace CP
