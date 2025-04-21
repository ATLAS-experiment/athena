/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Will pass if there are the specified number of photons with pT and eta in the specified range
#include "GeneratorFilters/xAODPhotonFilter.h"
#include "TruthUtils/HepMCHelpers.h"

StatusCode xAODPhotonFilter::filterInitialize()
{
  CHECK(m_truthPartContKey.initialize());
  return StatusCode::SUCCESS;
}


StatusCode xAODPhotonFilter::filterEvent() {

  // Retrieve Photon container
  SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainer{m_truthPartContKey};
  CHECK(xTruthParticleContainer.isValid());

  int NPhotons = 0;
  for (const xAOD::TruthParticle* part : *xTruthParticleContainer) {
    if (MC::isStable(part) && MC::isPhoton(part)) //photon
      if(  part->pt()>= m_Ptmin && part->pt()< m_Ptmax && part->abseta() <= m_EtaRange )
        {
          NPhotons++;
          if (NPhotons >= m_NPhotons)
            {
              setFilterPassed(true);
              return StatusCode::SUCCESS;
            }
        }

  }
  setFilterPassed(false);
  return StatusCode::SUCCESS;

}
