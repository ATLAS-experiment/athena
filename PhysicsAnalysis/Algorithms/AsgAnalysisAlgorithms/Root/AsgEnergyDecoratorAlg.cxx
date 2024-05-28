/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#include <AsgAnalysisAlgorithms/AsgEnergyDecoratorAlg.h>

namespace CP {

  StatusCode AsgEnergyDecoratorAlg::initialize() {

    ANA_CHECK(m_particlesHandle.initialize (m_systematicsList));
    ANA_CHECK(m_energyDecor.initialize (m_systematicsList, m_particlesHandle));
    ANA_CHECK(m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode AsgEnergyDecoratorAlg::execute() {

    for (const auto &sys : m_systematicsList.systematicsVector()) {

      const xAOD::IParticleContainer *particles = nullptr;
      ANA_CHECK(m_particlesHandle.retrieve (particles, sys));

      for (const xAOD::IParticle *particle : *particles) {
        m_energyDecor.set(*particle, particle->e(), sys);
      }

    }

    return StatusCode::SUCCESS;
  }

} // namespace
