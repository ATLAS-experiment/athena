/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#include "TruthParticleLevelAnalysisAlgorithms/ParticleLevelChargeDecoratorAlg.h"

namespace CP {

StatusCode ParticleLevelChargeDecoratorAlg::initialize() {

  ANA_CHECK(m_particlesKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode ParticleLevelChargeDecoratorAlg::execute(const EventContext &ctx) const {

  SG::ReadHandle<xAOD::TruthParticleContainer> particles(m_particlesKey, ctx);

  // decorators
  static const SG::AuxElement::Decorator<float> dec_charge("charge");

  for (const auto* particle : *particles) {

    // decorate the charge so we can save it later
    dec_charge(*particle) = particle->charge();
  }
  return StatusCode::SUCCESS;
}

}  // namespace CP
