/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/xAODElectronFilter.h"

StatusCode xAODElectronFilter::filterInitialize()
{
  CHECK(m_truthPartContKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode xAODElectronFilter::filterEvent() {
  // Retrieve full TruthParticle container
  SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainer{m_truthPartContKey};
  CHECK(xTruthParticleContainer.isValid());

  for (const xAOD::TruthParticle* part : *xTruthParticleContainer) {
    //electron
    if (part->pt() >= m_Ptmin && part->abseta() <= m_EtaRange)
      return StatusCode::SUCCESS;
  }

  setFilterPassed(false);
  return StatusCode::SUCCESS;
}

