/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>
/// @author Steffen Korn <steffen.korn@cern.ch>

#include "PartonHistory/CalcPartonHistory.h"

namespace CP {
namespace {
/// Initialize the kinematics and pdgId decorators of a top quark (e.g.
/// MC_t_beforeFSR) and of its decay products (e.g. MC_W_beforeFSR_from_t).
void initializeTopChainDecorators(PartonDecorator& dec,
                                  const std::string& top,
                                  const std::string& quark,
                                  const std::string& boson) {
  for (const char* fsr : {"_beforeFSR", "_afterFSR"}) {
    dec.initializeParticleDecorators("MC_" + top + fsr);
    for (const std::string& particle :
         {quark, boson, boson + "decay1", boson + "decay2"})
      dec.initializeParticleDecorators("MC_" + particle + fsr + "_from_" +
                                       top);
  }
}
}  // namespace

void CalcPartonHistory::Initialize4TopDecorators() {
  for (int idx = 1; idx <= 2; idx++) {
    initializeTopChainDecorators(m_dec, "t" + std::to_string(idx), "b", "W");
    initializeTopChainDecorators(m_dec, "tbar" + std::to_string(idx), "bbar",
                                 "W");
  }
}

void CalcPartonHistory::InitializeTopDecorators(bool fcnc) {
  initializeTopChainDecorators(m_dec, "t", fcnc ? "q" : "b", fcnc ? "X" : "W");
}

void CalcPartonHistory::InitializeAntiTopDecorators(bool fcnc) {
  initializeTopChainDecorators(m_dec, "tbar", fcnc ? "qbar" : "bbar",
                               fcnc ? "X" : "W");
}

void CalcPartonHistory::InitializeBottomDecorators() {
  m_dec.initializeParticleDecorators("MC_b_beforeFSR");
  m_dec.initializeParticleDecorators("MC_b_afterFSR");
}

void CalcPartonHistory::InitializeVectorBottomDecorators() {
  m_dec.initializeVectorParticleDecorators("MC_b_beforeFSR");
  m_dec.initializeVectorParticleDecorators("MC_b_afterFSR");
}

void CalcPartonHistory::InitializeAntiBottomDecorators() {
  m_dec.initializeParticleDecorators("MC_bbar_beforeFSR");
  m_dec.initializeParticleDecorators("MC_bbar_afterFSR");
}

void CalcPartonHistory::InitializeVectorAntiBottomDecorators() {
  m_dec.initializeVectorParticleDecorators("MC_bbar_beforeFSR");
  m_dec.initializeVectorParticleDecorators("MC_bbar_afterFSR");
}

void CalcPartonHistory::InitializeCharmDecorators() {
  m_dec.initializeParticleDecorators("MC_c_beforeFSR");
  m_dec.initializeParticleDecorators("MC_c_afterFSR");
}

void CalcPartonHistory::InitializeVectorCharmDecorators() {
  m_dec.initializeVectorParticleDecorators("MC_c_beforeFSR");
  m_dec.initializeVectorParticleDecorators("MC_c_afterFSR");
}

void CalcPartonHistory::InitializeAntiCharmDecorators() {
  m_dec.initializeParticleDecorators("MC_cbar_beforeFSR");
  m_dec.initializeParticleDecorators("MC_cbar_afterFSR");
}

void CalcPartonHistory::InitializeVectorAntiCharmDecorators() {
  m_dec.initializeVectorParticleDecorators("MC_cbar_beforeFSR");
  m_dec.initializeVectorParticleDecorators("MC_cbar_afterFSR");
}

void CalcPartonHistory::InitializeTtbarDecorators() {
  m_dec.initializePtEtaPhiMDecorator("MC_ttbar_beforeFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_ttbar_fromDecay_beforeFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_ttbar_afterFSR");
  m_dec.initializePtEtaPhiMDecorator("MC_ttbar_fromDecay_afterFSR");
}

void CalcPartonHistory::InitializePhotonDecorators() {
  m_dec.initializeParticleDecorators("MC_gamma");
  m_dec.initializeIntDecorator("MC_gamma_origin");
}

void CalcPartonHistory::InitializeHiggsDecorators() {
  for (const char* fsr : {"_beforeFSR", "_afterFSR"}) {
    for (const char* particle :
         {"H", "Hdecay1", "Hdecay2", "Hdecay1_decay1", "Hdecay2_decay1",
          "Hdecay1_decay2", "Hdecay2_decay2"})
      m_dec.initializeParticleDecorators("MC_" + std::string(particle) + fsr);
  }
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
    for (const char* fsr : {"_beforeFSR", "_afterFSR"}) {
      m_dec.initializeParticleDecorators("MC_" + Z + fsr);
      for (int i = 1; i <= 2; i++) {
        const std::string zDecay = "MC_" + Z + "decay" + std::to_string(i);
        m_dec.initializeParticleDecorators(zDecay + fsr);
        // Tau decay products: MC_Zdecay<i>_decay<j>_<fsr>
        if (extend) {
          for (int j = 1; j <= 3; j++)
            m_dec.initializeParticleDecorators(
                zDecay + "_decay" + std::to_string(j) + fsr);
        }
      }
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
    for (const char* fsr : {"_beforeFSR", "_afterFSR"}) {
      m_dec.initializeParticleDecorators("MC_" + W + fsr);
      m_dec.initializeParticleDecorators("MC_" + W + "decay1" + fsr);
      m_dec.initializeParticleDecorators("MC_" + W + "decay2" + fsr);
    }

    m_dec.initializeIntDecorator("MC_" + W + "_IsOnShell");
  }
}
}  // namespace CP
