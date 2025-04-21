/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/xAODMultiElectronFilter.h"

StatusCode xAODMultiElectronFilter::filterInitialize()
{
  CHECK(m_truthPartContKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode xAODMultiElectronFilter::filterEvent() {
  // Retrieve TruthElectron container
  SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainer{m_truthPartContKey};
  CHECK(xTruthParticleContainer.isValid());
  int numElectrons = 0;

    for (auto *part: *xTruthParticleContainer) {
      if(  part->pt()>= m_ptmin && part->abseta() <= m_etaRange )
      {
        numElectrons++;
        if (numElectrons >= m_nElectrons)
        {
          setFilterPassed(true);
          return StatusCode::SUCCESS;
        }
      }
  }


  setFilterPassed(false);
  return StatusCode::SUCCESS;
}
