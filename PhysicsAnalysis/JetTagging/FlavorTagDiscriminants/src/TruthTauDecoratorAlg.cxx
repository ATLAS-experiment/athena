/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/TruthTauDecoratorAlg.h"
#include "StoreGate/WriteDecorHandle.h"
#include "TruthUtils/TruthClasses.h"
using namespace MCTruthPartClassifier;
#include <cmath>
#include <limits>


namespace {
  const SG::ConstAccessor<double> acc_pt_vis("pt_vis");
  const SG::ConstAccessor<double> acc_eta_vis("eta_vis");
  const SG::ConstAccessor<double> acc_phi_vis("phi_vis");
  const SG::ConstAccessor<double> acc_m_vis("m_vis");
  const SG::ConstAccessor<unsigned int> acc_classifierType(
    "classifierParticleType");
  const SG::ConstAccessor<unsigned int> acc_classifierOutcome(
    "classifierParticleOutCome");
}


namespace FlavorTagDiscriminants {

  TruthTauDecoratorAlg::TruthTauDecoratorAlg(
    const std::string& name, ISvcLocator* loc)
    : AthReentrantAlgorithm(name, loc)
  {
  }

  StatusCode TruthTauDecoratorAlg::initialize() {
    ATH_CHECK(m_jetKey.initialize());
    ATH_CHECK(m_truthTauKey.initialize());
    ATH_CHECK(m_tauTruthTool.retrieve());

    ATH_CHECK(m_dec_matched.initialize());
    ATH_CHECK(m_dec_isHadTau.initialize());
    ATH_CHECK(m_dec_decayMode.initialize());
    ATH_CHECK(m_dec_deltaPt.initialize());
    ATH_CHECK(m_dec_ptVis.initialize());
    ATH_CHECK(m_dec_classifierOutcome.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode TruthTauDecoratorAlg::execute(const EventContext& ctx) const {
    SG::ReadHandle<JC> jets(m_jetKey, ctx);
    SG::ReadHandle<TPC> truthTaus(m_truthTauKey, ctx);

    SG::WriteDecorHandle<JC, char>         matched(m_dec_matched, ctx);
    SG::WriteDecorHandle<JC, int>          isHadTau(m_dec_isHadTau, ctx);
    SG::WriteDecorHandle<JC, int>          decayMode(m_dec_decayMode, ctx);
    SG::WriteDecorHandle<JC, float>        deltaPt(m_dec_deltaPt, ctx);
    SG::WriteDecorHandle<JC, double>       ptVis(m_dec_ptVis, ctx);
    SG::WriteDecorHandle<JC, unsigned int> outcome(m_dec_classifierOutcome, ctx);

    const float maxDR = m_maxDeltaR.value();

    for (const xAOD::Jet* jet : *jets) {
      const xAOD::TruthParticle* best = nullptr;
      float bestDR = maxDR;

      for (const xAOD::TruthParticle* tau : *truthTaus) {
        if (acc_classifierType(*tau) != IsoTau) continue;

        TLorentzVector tauVis;
        tauVis.SetPtEtaPhiM(
          acc_pt_vis(*tau), acc_eta_vis(*tau),
          acc_phi_vis(*tau), acc_m_vis(*tau));

        float dR = jet->p4().DeltaR(tauVis);
        if (dR < bestDR) {
          bestDR = dR;
          best = tau;
        }
      }

      if (best) {
        matched(*jet) = 1;
        deltaPt(*jet) = static_cast<float>(jet->pt() - acc_pt_vis(*best));
        ptVis(*jet) = acc_pt_vis(*best);
        outcome(*jet) = acc_classifierOutcome(*best);

        unsigned int prong = acc_classifierOutcome(*best);
        if (prong == OneProng || prong == ThreeProng || prong == FiveProng) {
          isHadTau(*jet) = 1;
          decayMode(*jet) = static_cast<int>(
            m_tauTruthTool->getDecayMode(*best));
        } else {
          isHadTau(*jet) = 0;
          decayMode(*jet) = -1;
        }
      } else {
        matched(*jet) = 0;
        deltaPt(*jet) = std::numeric_limits<float>::quiet_NaN();
        ptVis(*jet) = std::numeric_limits<double>::quiet_NaN();
        outcome(*jet) = 0;
        isHadTau(*jet) = -1;
        decayMode(*jet) = -1;
      }
    }

    return StatusCode::SUCCESS;
  }

}
