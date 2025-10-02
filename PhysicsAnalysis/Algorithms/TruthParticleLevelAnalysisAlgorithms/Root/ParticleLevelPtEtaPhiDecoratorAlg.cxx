/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#include "TruthParticleLevelAnalysisAlgorithms/ParticleLevelPtEtaPhiDecoratorAlg.h"

namespace CP {

StatusCode ParticleLevelPtEtaPhiDecoratorAlg::initialize() {

  ANA_CHECK(m_particlesKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode ParticleLevelPtEtaPhiDecoratorAlg::execute(const EventContext &ctx) const {

  SG::ReadHandle<xAOD::TruthParticleContainer> particles(m_particlesKey, ctx);

  // decorators
  static const SG::Decorator<float> dec_pt("pt");
  static const SG::Decorator<float> dec_eta("eta");
  static const SG::Decorator<float> dec_phi("phi");

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
