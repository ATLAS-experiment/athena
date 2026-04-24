/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Ported from TDD's JetTruthAssociator (overlapLepton selection) and
// processEvent overlap check.  Computes ftag_hasOverlapLepton per jet
// at derivation time, enabling TruthElectrons and TruthMuons removal
// from the DAOD.

#include "JetOverlapLeptonDecoratorAlg.h"

#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"

#include "xAODTruth/TruthParticle.h"

#include <cmath>
#include <vector>


namespace {
  namespace MC = MCTruthPartClassifier;
}

namespace FlavorTagJetDecorators {

  JetOverlapLeptonDecoratorAlg::JetOverlapLeptonDecoratorAlg(
    const std::string& name, ISvcLocator* loc)
    : AthReentrantAlgorithm(name, loc)
  {
  }

  StatusCode JetOverlapLeptonDecoratorAlg::initialize() {
    ATH_CHECK(m_jetKey.initialize());
    ATH_CHECK(m_truthElectronKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_truthMuonKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_truthElectronOriginKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_truthMuonOriginKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_dec_hasOverlapLepton.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode JetOverlapLeptonDecoratorAlg::execute(
      const EventContext& ctx) const {

    SG::ReadHandle<xAOD::JetContainer> jets(m_jetKey, ctx);
    if (!jets.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve jet container: "
                    << m_jetKey.key());
      return StatusCode::FAILURE;
    }

    SG::WriteDecorHandle<xAOD::JetContainer, char> decOverlap(
        m_dec_hasOverlapLepton, ctx);

    // Collect overlap-candidate leptons from both truth containers.
    // Apply kinematic and origin selection here (mirrors TDD's
    // baseline-truth-kinematics.json + overlapLepton particle selector).
    const float drCut = m_overlapDR.value();
    const float ptMin = m_ptMinimum.value();
    const float absEtaMax = m_absEtaMaximum.value();

    std::vector<const xAOD::TruthParticle*> leptons;

    auto collectLeptons = [&](
        const SG::ReadHandleKey<xAOD::TruthParticleContainer>& key,
        const SG::ReadDecorHandleKey<xAOD::TruthParticleContainer>& originKey) {
      if (key.empty()) return;
      SG::ReadHandle<xAOD::TruthParticleContainer> container(key, ctx);
      if (!container.isValid()) return;  // gracefully skip (e.g. data)
      SG::ReadDecorHandle<xAOD::TruthParticleContainer, unsigned int> origin(originKey, ctx);
      for (const xAOD::TruthParticle* tp : *container) {
        if (tp->status() != 1) continue;
        if (tp->pt() < ptMin) continue;
        if (std::abs(tp->eta()) > absEtaMax) continue;
        unsigned int o = origin(*tp);
        if (o != MC::WBoson && o != MC::ZBoson && o != MC::top) continue;
        leptons.push_back(tp);
      }
    };

    collectLeptons(m_truthElectronKey, m_truthElectronOriginKey);
    collectLeptons(m_truthMuonKey, m_truthMuonOriginKey);

    // Decorate each jet
    for (const xAOD::Jet* jet : *jets) {
      char hasOverlap = 0;
      for (const xAOD::TruthParticle* lep : leptons) {
        if (jet->p4().DeltaR(lep->p4()) < drCut) {
          hasOverlap = 1;
          break;
        }
      }
      decOverlap(*jet) = hasOverlap;
    }

    return StatusCode::SUCCESS;
  }

}  // namespace FlavorTagJetDecorators
