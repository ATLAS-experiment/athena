/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#include "TruthParticleLevelAnalysisAlgorithms/ParticleLevelChargeDecoratorAlg.h"

#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

namespace CP {

StatusCode ParticleLevelChargeDecoratorAlg::initialize() {

  ANA_CHECK(m_particlesKey.initialize());
  ANA_CHECK(m_decChargeKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode ParticleLevelChargeDecoratorAlg::execute(const EventContext &ctx) const {

  SG::ReadHandle<xAOD::TruthParticleContainer> particles(m_particlesKey, ctx);

  // decorators
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, float> dec_charge(m_decChargeKey, ctx);

  for (const auto* particle : *particles) {

    // decorate the charge so we can save it later
    dec_charge(*particle) = particle->charge();
  }
  return StatusCode::SUCCESS;
}

}  // namespace CP
