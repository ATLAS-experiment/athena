/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>
/// @author Steffen Korn <steffen.korn@cern.ch>

#include "PartonHistory/CalcPartonHistory.h"

namespace CP {
void CalcPartonHistory::Initialize4TopDecorators() {
  for (int idx = 1; idx <= 2; idx++) {
    m_dec.initializePtEtaPhiMDecorator("MC_t" + std::to_string(idx) +
                                       "_beforeFSR");
    m_dec.initializePtEtaPhiMDecorator("MC_b_beforeFSR_from_t" +
                                       std::to_string(idx));
    m_dec.initializePtEtaPhiMDecorator("MC_W_beforeFSR_from_t" +
                                       std::to_string(idx));
    m_dec.initializePtEtaPhiMDecorator("MC_Wdecay1_beforeFSR_from_t" +
                                       std::to_string(idx));
    m_dec.initializePtEtaPhiMDecorator("MC_Wdecay2_beforeFSR_from_t" +
                                       std::to_string(idx));

    m_dec.initializePtEtaPhiMDecorator("MC_t" + std::to_string(idx) +
                                       "_afterFSR");
    m_dec.initializePtEtaPhiMDecorator("MC_b_afterFSR_from_t" +
                                       std::to_string(idx));
    m_dec.initializePtEtaPhiMDecorator("MC_W_afterFSR_from_t" +
                                       std::to_string(idx));
    m_dec.initializePtEtaPhiMDecorator("MC_Wdecay1_afterFSR_from_t" +
                                       std::to_string(idx));
    m_dec.initializePtEtaPhiMDecorator("MC_Wdecay2_afterFSR_from_t" +
                                       std::to_string(idx));

    m_dec.initializePtEtaPhiMDecorator("MC_tbar" + std::to_string(idx) +
                                       "_beforeFSR");
    m_dec.initializePtEtaPhiMDecorator("MC_bbar_beforeFSR_from_tbar" +
                                       std::to_string(idx));
    m_dec.initializePtEtaPhiMDecorator("MC_W_beforeFSR_from_tbar" +
                                       std::to_string(idx));
    m_dec.initializePtEtaPhiMDecorator("MC_Wdecay1_beforeFSR_from_tbar" +
                                       std::to_string(idx));
    m_dec.initializePtEtaPhiMDecorator("MC_Wdecay2_beforeFSR_from_tbar" +
                                       std::to_string(idx));

    m_dec.initializePtEtaPhiMDecorator("MC_tbar" + std::to_string(idx) +
                                       "_afterFSR");
    m_dec.initializePtEtaPhiMDecorator("MC_bbar_afterFSR_from_tbar" +
                                       std::to_string(idx));
    m_dec.initializePtEtaPhiMDecorator("MC_W_afterFSR_from_tbar" +
                                       std::to_string(idx));
    m_dec.initializePtEtaPhiMDecorator("MC_Wdecay1_afterFSR_from_tbar" +
                                       std::to_string(idx));
    m_dec.initializePtEtaPhiMDecorator("MC_Wdecay2_afterFSR_from_tbar" +
                                       std::to_string(idx));

    m_dec.initializeIntDecorator("MC_t" + std::to_string(idx) +
                                 "_beforeFSR_pdgId");
    m_dec.initializeIntDecorator("MC_b_beforeFSR_from_t" + std::to_string(idx) +
                                 "_pdgId");
    m_dec.initializeIntDecorator("MC_W_beforeFSR_from_t" + std::to_string(idx) +
                                 "_pdgId");
    m_dec.initializeIntDecorator("MC_Wdecay1_beforeFSR_from_t" +
                                 std::to_string(idx) + "_pdgId");
    m_dec.initializeIntDecorator("MC_Wdecay2_beforeFSR_from_t" +
                                 std::to_string(idx) + "_pdgId");

    m_dec.initializeIntDecorator("MC_t" + std::to_string(idx) +
                                 "_afterFSR_pdgId");
    m_dec.initializeIntDecorator("MC_b_afterFSR_from_t" + std::to_string(idx) +
                                 "_pdgId");
    m_dec.initializeIntDecorator("MC_W_afterFSR_from_t" + std::to_string(idx) +
                                 "_pdgId");
    m_dec.initializeIntDecorator("MC_Wdecay1_afterFSR_from_t" +
                                 std::to_string(idx) + "_pdgId");
    m_dec.initializeIntDecorator("MC_Wdecay2_afterFSR_from_t" +
                                 std::to_string(idx) + "_pdgId");

    m_dec.initializeIntDecorator("MC_tbar" + std::to_string(idx) +
                                 "_beforeFSR_pdgId");
    m_dec.initializeIntDecorator("MC_bbar_beforeFSR_from_tbar" +
                                 std::to_string(idx) + "_pdgId");
    m_dec.initializeIntDecorator("MC_W_beforeFSR_from_tbar" +
                                 std::to_string(idx) + "_pdgId");
    m_dec.initializeIntDecorator("MC_Wdecay1_beforeFSR_from_tbar" +
                                 std::to_string(idx) + "_pdgId");
    m_dec.initializeIntDecorator("MC_Wdecay2_beforeFSR_from_tbar" +
                                 std::to_string(idx) + "_pdgId");

    m_dec.initializeIntDecorator("MC_tbar" + std::to_string(idx) +
                                 "_afterFSR_pdgId");
    m_dec.initializeIntDecorator("MC_bbar_afterFSR_from_tbar" +
                                 std::to_string(idx) + "_pdgId");
    m_dec.initializeIntDecorator("MC_W_afterFSR_from_tbar" +
                                 std::to_string(idx) + "_pdgId");
    m_dec.initializeIntDecorator("MC_Wdecay1_afterFSR_from_tbar" +
                                 std::to_string(idx) + "_pdgId");
    m_dec.initializeIntDecorator("MC_Wdecay2_afterFSR_from_tbar" +
                                 std::to_string(idx) + "_pdgId");
  }
}

void CalcPartonHistory::InitializeTopDecorators() {
  m_dec.initializePtEtaPhiMDecorator("MC_t_beforeFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_b_beforeFSR_from_t");
  m_dec.initializePtEtaPhiMDecorator("MC_W_beforeFSR_from_t");
  m_dec.initializePtEtaPhiMDecorator("MC_Wdecay1_beforeFSR_from_t");
  m_dec.initializePtEtaPhiMDecorator("MC_Wdecay2_beforeFSR_from_t");

  m_dec.initializePtEtaPhiMDecorator("MC_t_afterFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_b_afterFSR_from_t");
  m_dec.initializePtEtaPhiMDecorator("MC_W_afterFSR_from_t");
  m_dec.initializePtEtaPhiMDecorator("MC_Wdecay1_afterFSR_from_t");
  m_dec.initializePtEtaPhiMDecorator("MC_Wdecay2_afterFSR_from_t");

  m_dec.initializeIntDecorator("MC_t_beforeFSR_pdgId");
  m_dec.initializeIntDecorator("MC_b_beforeFSR_from_t_pdgId");
  m_dec.initializeIntDecorator("MC_W_beforeFSR_from_t_pdgId");
  m_dec.initializeIntDecorator("MC_Wdecay1_beforeFSR_from_t_pdgId");
  m_dec.initializeIntDecorator("MC_Wdecay2_beforeFSR_from_t_pdgId");

  m_dec.initializeIntDecorator("MC_t_afterFSR_pdgId");
  m_dec.initializeIntDecorator("MC_b_afterFSR_from_t_pdgId");
  m_dec.initializeIntDecorator("MC_W_afterFSR_from_t_pdgId");
  m_dec.initializeIntDecorator("MC_Wdecay1_afterFSR_from_t_pdgId");
  m_dec.initializeIntDecorator("MC_Wdecay2_afterFSR_from_t_pdgId");
}

void CalcPartonHistory::InitializeAntiTopDecorators() {
  m_dec.initializePtEtaPhiMDecorator("MC_tbar_beforeFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_bbar_beforeFSR_from_tbar");
  m_dec.initializePtEtaPhiMDecorator("MC_W_beforeFSR_from_tbar");
  m_dec.initializePtEtaPhiMDecorator("MC_Wdecay1_beforeFSR_from_tbar");
  m_dec.initializePtEtaPhiMDecorator("MC_Wdecay2_beforeFSR_from_tbar");

  m_dec.initializePtEtaPhiMDecorator("MC_tbar_afterFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_bbar_afterFSR_from_tbar");
  m_dec.initializePtEtaPhiMDecorator("MC_W_afterFSR_from_tbar");
  m_dec.initializePtEtaPhiMDecorator("MC_Wdecay1_afterFSR_from_tbar");
  m_dec.initializePtEtaPhiMDecorator("MC_Wdecay2_afterFSR_from_tbar");

  m_dec.initializeIntDecorator("MC_tbar_beforeFSR_pdgId");
  m_dec.initializeIntDecorator("MC_bbar_beforeFSR_from_tbar_pdgId");
  m_dec.initializeIntDecorator("MC_W_beforeFSR_from_tbar_pdgId");
  m_dec.initializeIntDecorator("MC_Wdecay1_beforeFSR_from_tbar_pdgId");
  m_dec.initializeIntDecorator("MC_Wdecay2_beforeFSR_from_tbar_pdgId");

  m_dec.initializeIntDecorator("MC_tbar_afterFSR_pdgId");
  m_dec.initializeIntDecorator("MC_bbar_afterFSR_from_tbar_pdgId");
  m_dec.initializeIntDecorator("MC_W_afterFSR_from_tbar_pdgId");
  m_dec.initializeIntDecorator("MC_Wdecay1_afterFSR_from_tbar_pdgId");
  m_dec.initializeIntDecorator("MC_Wdecay2_afterFSR_from_tbar_pdgId");
}

void CalcPartonHistory::InitializeBottomDecorators() {
  m_dec.initializePtEtaPhiMDecorator("MC_b_beforeFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_b_afterFSR");

  m_dec.initializeIntDecorator("MC_b_beforeFSR_pdgId");
  m_dec.initializeIntDecorator("MC_b_afterFSR_pdgId");
}

void CalcPartonHistory::InitializeVectorBottomDecorators() {
  m_dec.initializeVectorPtEtaPhiMDecorator("MC_b_beforeFSR");
  m_dec.initializeVectorPtEtaPhiMDecorator("MC_b_afterFSR");

  m_dec.initializeVectorIntDecorator("MC_b_beforeFSR_pdgId");
  m_dec.initializeVectorIntDecorator("MC_b_afterFSR_pdgId");
}

void CalcPartonHistory::InitializeAntiBottomDecorators() {
  m_dec.initializePtEtaPhiMDecorator("MC_bbar_beforeFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_bbar_afterFSR");

  m_dec.initializeIntDecorator("MC_bbar_beforeFSR_pdgId");
  m_dec.initializeIntDecorator("MC_bbar_afterFSR_pdgId");
}

void CalcPartonHistory::InitializeVectorAntiBottomDecorators() {
  m_dec.initializeVectorPtEtaPhiMDecorator("MC_bbar_beforeFSR");
  m_dec.initializeVectorPtEtaPhiMDecorator("MC_bbar_afterFSR");

  m_dec.initializeVectorIntDecorator("MC_bbar_beforeFSR_pdgId");
  m_dec.initializeVectorIntDecorator("MC_bbar_afterFSR_pdgId");
}

void CalcPartonHistory::InitializeCharmDecorators() {
  m_dec.initializePtEtaPhiMDecorator("MC_c_beforeFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_c_afterFSR");

  m_dec.initializeIntDecorator("MC_c_beforeFSR_pdgId");
  m_dec.initializeIntDecorator("MC_c_afterFSR_pdgId");
}

void CalcPartonHistory::InitializeVectorCharmDecorators() {
  m_dec.initializeVectorPtEtaPhiMDecorator("MC_c_beforeFSR");
  m_dec.initializeVectorPtEtaPhiMDecorator("MC_c_afterFSR");

  m_dec.initializeVectorIntDecorator("MC_c_beforeFSR_pdgId");
  m_dec.initializeVectorIntDecorator("MC_c_afterFSR_pdgId");
}

void CalcPartonHistory::InitializeAntiCharmDecorators() {
  m_dec.initializePtEtaPhiMDecorator("MC_cbar_beforeFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_cbar_afterFSR");

  m_dec.initializeIntDecorator("MC_cbar_beforeFSR_pdgId");
  m_dec.initializeIntDecorator("MC_cbar_afterFSR_pdgId");
}

void CalcPartonHistory::InitializeVectorAntiCharmDecorators() {
  m_dec.initializeVectorPtEtaPhiMDecorator("MC_cbar_beforeFSR");
  m_dec.initializeVectorPtEtaPhiMDecorator("MC_cbar_afterFSR");

  m_dec.initializeVectorIntDecorator("MC_cbar_beforeFSR_pdgId");
  m_dec.initializeVectorIntDecorator("MC_cbar_afterFSR_pdgId");
}

void CalcPartonHistory::InitializeTtbarDecorators() {
  m_dec.initializeFloatDecorator(
      {"MC_ttbar_beforeFSR_m", "MC_ttbar_beforeFSR_pt",
       "MC_ttbar_beforeFSR_eta", "MC_ttbar_beforeFSR_phi"});
  m_dec.initializeFloatDecorator(
      {"MC_ttbar_fromDecay_beforeFSR_m", "MC_ttbar_fromDecay_beforeFSR_pt",
       "MC_ttbar_fromDecay_beforeFSR_eta", "MC_ttbar_fromDecay_beforeFSR_phi"});

  m_dec.initializeFloatDecorator({"MC_ttbar_afterFSR_m", "MC_ttbar_afterFSR_pt",
                                  "MC_ttbar_afterFSR_eta",
                                  "MC_ttbar_afterFSR_phi"});
  m_dec.initializeFloatDecorator(
      {"MC_ttbar_fromDecay_afterFSR_m", "MC_ttbar_fromDecay_afterFSR_pt",
       "MC_ttbar_fromDecay_afterFSR_eta", "MC_ttbar_fromDecay_afterFSR_phi"});
}

void CalcPartonHistory::InitializePhotonDecorators() {
  m_dec.initializePtEtaPhiMDecorator("MC_gamma");
  m_dec.initializeIntDecorator("MC_gamma_origin");
  m_dec.initializeIntDecorator("MC_gamma_pdgId");
}

void CalcPartonHistory::InitializeHiggsDecorators() {
  m_dec.initializePtEtaPhiMDecorator("MC_H_beforeFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_Hdecay1_beforeFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_Hdecay2_beforeFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_Hdecay1_decay1_beforeFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_Hdecay2_decay1_beforeFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_Hdecay1_decay2_beforeFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_Hdecay2_decay2_beforeFSR");

  m_dec.initializePtEtaPhiMDecorator("MC_H_afterFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_Hdecay1_afterFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_Hdecay2_afterFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_Hdecay1_decay1_afterFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_Hdecay2_decay1_afterFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_Hdecay1_decay2_afterFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_Hdecay2_decay2_afterFSR");

  m_dec.initializeIntDecorator("MC_H_beforeFSR_pdgId");
  m_dec.initializeIntDecorator("MC_Hdecay1_beforeFSR_pdgId");
  m_dec.initializeIntDecorator("MC_Hdecay2_beforeFSR_pdgId");
  m_dec.initializeIntDecorator("MC_Hdecay1_decay1_beforeFSR_pdgId");
  m_dec.initializeIntDecorator("MC_Hdecay2_decay1_beforeFSR_pdgId");
  m_dec.initializeIntDecorator("MC_Hdecay1_decay2_beforeFSR_pdgId");
  m_dec.initializeIntDecorator("MC_Hdecay2_decay2_beforeFSR_pdgId");

  m_dec.initializeIntDecorator("MC_H_afterFSR_pdgId");
  m_dec.initializeIntDecorator("MC_Hdecay1_afterFSR_pdgId");
  m_dec.initializeIntDecorator("MC_Hdecay2_afterFSR_pdgId");
  m_dec.initializeIntDecorator("MC_Hdecay1_decay1_afterFSR_pdgId");
  m_dec.initializeIntDecorator("MC_Hdecay2_decay1_afterFSR_pdgId");
  m_dec.initializeIntDecorator("MC_Hdecay1_decay2_afterFSR_pdgId");
  m_dec.initializeIntDecorator("MC_Hdecay2_decay2_afterFSR_pdgId");
}

void CalcPartonHistory::InitializeZDecorators(int n_Zs, bool extend) {
  std::vector<std::string> Zs;
  if (n_Zs == 1)
    Zs.push_back("Z");
  else {
    for (int i = 1; i <= n_Zs; i++)
      Zs.push_back("Z" + std::to_string(i));
  }
  for (auto& Z : Zs) {
    m_dec.initializePtEtaPhiMDecorator("MC_" + Z + "_beforeFSR");
    m_dec.initializePtEtaPhiMDecorator("MC_" + Z + "decay1_beforeFSR");
    m_dec.initializePtEtaPhiMDecorator("MC_" + Z + "decay2_beforeFSR");

    m_dec.initializePtEtaPhiMDecorator("MC_" + Z + "_afterFSR");
    m_dec.initializePtEtaPhiMDecorator("MC_" + Z + "decay1_afterFSR");
    m_dec.initializePtEtaPhiMDecorator("MC_" + Z + "decay2_afterFSR");

    m_dec.initializeIntDecorator("MC_" + Z + "_beforeFSR_pdgId");
    m_dec.initializeIntDecorator("MC_" + Z + "decay1_beforeFSR_pdgId");
    m_dec.initializeIntDecorator("MC_" + Z + "decay2_beforeFSR_pdgId");

    m_dec.initializeIntDecorator("MC_" + Z + "_afterFSR_pdgId");
    m_dec.initializeIntDecorator("MC_" + Z + "decay1_afterFSR_pdgId");
    m_dec.initializeIntDecorator("MC_" + Z + "decay2_afterFSR_pdgId");

    if (extend) {
      m_dec.initializePtEtaPhiMDecorator("MC_" + Z + "decay1_decay1_beforeFSR");
      m_dec.initializePtEtaPhiMDecorator("MC_" + Z + "decay1_decay2_beforeFSR");
      m_dec.initializePtEtaPhiMDecorator("MC_" + Z + "decay1_decay3_beforeFSR");
      m_dec.initializePtEtaPhiMDecorator("MC_" + Z + "decay2_decay1_beforeFSR");
      m_dec.initializePtEtaPhiMDecorator("MC_" + Z + "decay2_decay2_beforeFSR");
      m_dec.initializePtEtaPhiMDecorator("MC_" + Z + "decay2_decay3_beforeFSR");

      m_dec.initializeIntDecorator("MC_" + Z + "decay1_decay1_beforeFSR_pdgId");
      m_dec.initializeIntDecorator("MC_" + Z + "decay1_decay2_beforeFSR_pdgId");
      m_dec.initializeIntDecorator("MC_" + Z + "decay1_decay3_beforeFSR_pdgId");
      m_dec.initializeIntDecorator("MC_" + Z + "decay2_decay1_beforeFSR_pdgId");
      m_dec.initializeIntDecorator("MC_" + Z + "decay2_decay2_beforeFSR_pdgId");
      m_dec.initializeIntDecorator("MC_" + Z + "decay2_decay3_beforeFSR_pdgId");

      m_dec.initializePtEtaPhiMDecorator("MC_" + Z + "decay1_decay1_afterFSR");
      m_dec.initializePtEtaPhiMDecorator("MC_" + Z + "decay1_decay2_afterFSR");
      m_dec.initializePtEtaPhiMDecorator("MC_" + Z + "decay1_decay3_afterFSR");
      m_dec.initializePtEtaPhiMDecorator("MC_" + Z + "decay2_decay1_afterFSR");
      m_dec.initializePtEtaPhiMDecorator("MC_" + Z + "decay2_decay2_afterFSR");
      m_dec.initializePtEtaPhiMDecorator("MC_" + Z + "decay2_decay3_afterFSR");

      m_dec.initializeIntDecorator("MC_" + Z + "decay1_decay1_afterFSR_pdgId");
      m_dec.initializeIntDecorator("MC_" + Z + "decay1_decay2_afterFSR_pdgId");
      m_dec.initializeIntDecorator("MC_" + Z + "decay1_decay3_afterFSR_pdgId");
      m_dec.initializeIntDecorator("MC_" + Z + "decay2_decay1_afterFSR_pdgId");
      m_dec.initializeIntDecorator("MC_" + Z + "decay2_decay2_afterFSR_pdgId");
      m_dec.initializeIntDecorator("MC_" + Z + "decay2_decay3_afterFSR_pdgId");
    }

    m_dec.initializeIntDecorator("MC_" + Z + "_IsOnShell");
  }
}

void CalcPartonHistory::InitializeWDecorators(int n_Ws) {
  std::vector<std::string> Ws;
  if (n_Ws == 1)
    Ws.push_back("W");
  else {
    for (int i = 1; i <= n_Ws; i++) {
      Ws.push_back("W" + std::to_string(i));
    }
  }
  for (auto& W : Ws) {
    m_dec.initializePtEtaPhiMDecorator("MC_" + W + "_beforeFSR");
    m_dec.initializePtEtaPhiMDecorator("MC_" + W + "decay1_beforeFSR");
    m_dec.initializePtEtaPhiMDecorator("MC_" + W + "decay2_beforeFSR");

    m_dec.initializePtEtaPhiMDecorator("MC_" + W + "_afterFSR");
    m_dec.initializePtEtaPhiMDecorator("MC_" + W + "decay1_afterFSR");
    m_dec.initializePtEtaPhiMDecorator("MC_" + W + "decay2_afterFSR");

    m_dec.initializeIntDecorator("MC_" + W + "_beforeFSR_pdgId");
    m_dec.initializeIntDecorator("MC_" + W + "decay1_beforeFSR_pdgId");
    m_dec.initializeIntDecorator("MC_" + W + "decay2_beforeFSR_pdgId");

    m_dec.initializeIntDecorator("MC_" + W + "_afterFSR_pdgId");
    m_dec.initializeIntDecorator("MC_" + W + "decay1_afterFSR_pdgId");
    m_dec.initializeIntDecorator("MC_" + W + "decay2_afterFSR_pdgId");

    m_dec.initializeIntDecorator("MC_" + W + "_IsOnShell");
  }
}
}  // namespace CP
