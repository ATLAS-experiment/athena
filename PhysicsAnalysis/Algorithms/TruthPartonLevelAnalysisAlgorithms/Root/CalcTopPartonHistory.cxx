/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>
/// @author Steffen Korn <steffen.korn@cern.ch>

#include "PartonHistory/CalcPartonHistory.h"
#include "PartonHistory/PartonHistoryUtils.h"
#include "VectorHelpers/DecoratorHelpers.h"

namespace CP {
using ROOT::Math::PtEtaPhiMVector;

void CalcPartonHistory::FillTopPartonHistory() {
  FillGenericPartonHistory("MC_t_beforeFSR", "MC_t_beforeFSR", 0);
  FillGenericPartonHistory("MC_t_b_beforeFSR", "MC_b_beforeFSR_from_t", 0);
  FillGenericPartonHistory("MC_t_afterFSR", "MC_t_afterFSR", 0);
  FillGenericPartonHistory("MC_t_b_afterFSR", "MC_b_afterFSR_from_t", 0);
  FillWPartonHistory("t");
}

void CalcPartonHistory::FillAntiTopPartonHistory() {
  FillGenericPartonHistory("MC_tbar_beforeFSR", "MC_tbar_beforeFSR", 0);
  FillGenericPartonHistory("MC_tbar_bbar_beforeFSR",
                           "MC_bbar_beforeFSR_from_tbar", 0);
  FillGenericPartonHistory("MC_tbar_afterFSR", "MC_tbar_afterFSR", 0);
  FillGenericPartonHistory("MC_tbar_bbar_afterFSR",
                           "MC_bbar_afterFSR_from_tbar", 0);
  FillWPartonHistory("tbar");
}

void CalcPartonHistory::FillTtbarPartonHistory() {
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

  if (Retrievep4(m_prefix + "_" + "MC_t_WDecay1_beforeFSR", WpDecay1) &&
      Retrievep4(m_prefix + "_" + "MC_t_WDecay2_beforeFSR", WpDecay2) &&
      Retrievep4(m_prefix + "_" + "MC_tbar_WDecay1_beforeFSR", WmDecay1) &&
      Retrievep4(m_prefix + "_" + "MC_tbar_WDecay2_beforeFSR", WmDecay2) &&
      Retrievep4(m_prefix + "_" + "MC_t_b_beforeFSR", b) &&
      Retrievep4(m_prefix + "_" + "MC_tbar_bbar_beforeFSR", bbar)) {
    ttbar = WpDecay1 + WpDecay2 + WmDecay1 + WmDecay2 + b + bbar;
    m_dec.decorateParticle("MC_ttbar_fromDecay_beforeFSR", ttbar);
  }

  if (Retrievep4(m_prefix + "_" + "MC_t_WDecay1_afterFSR", WpDecay1) &&
      Retrievep4(m_prefix + "_" + "MC_t_WDecay2_afterFSR", WpDecay2) &&
      Retrievep4(m_prefix + "_" + "MC_tbar_WDecay1_afterFSR", WmDecay1) &&
      Retrievep4(m_prefix + "_" + "MC_tbar_WDecay2_afterFSR", WmDecay2) &&
      Retrievep4(m_prefix + "_" + "MC_t_b_afterFSR", b) &&
      Retrievep4(m_prefix + "_" + "MC_tbar_bbar_afterFSR", bbar)) {
    ttbar = WpDecay1 + WpDecay2 + WmDecay1 + WmDecay2 + b + bbar;
    m_dec.decorateParticle("MC_ttbar_fromDecay_afterFSR", ttbar);
  }
}
}  // namespace CP
