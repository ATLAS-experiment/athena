/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#include "TruthParticleLevelAnalysisAlgorithms/ParticleLevelPtEtaPhiDecoratorAlg.h"

#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

namespace CP {

StatusCode ParticleLevelPtEtaPhiDecoratorAlg::initialize() {

  ANA_CHECK(m_particlesKey.initialize());
  ANA_CHECK(m_decPtKey.initialize());
  ANA_CHECK(m_decEtaKey.initialize());
  ANA_CHECK(m_decPhiKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode ParticleLevelPtEtaPhiDecoratorAlg::execute(const EventContext &ctx) const {

  SG::ReadHandle<xAOD::TruthParticleContainer> particles(m_particlesKey, ctx);

  // decorators
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, float> dec_pt(m_decPtKey, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, float> dec_eta(m_decEtaKey, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, float> dec_phi(m_decPhiKey, ctx);

  for (const auto* particle : *particles) {

    // decorate pT/eta/phi of the truth particle
    // (Energy is already decorated at DAOD-level!)
    dec_pt(*particle) = particle->pt();
    dec_eta(*particle) = particle->eta();
    dec_phi(*particle) = particle->phi();
  }
  return StatusCode::SUCCESS;
}

}  // namespace CP
