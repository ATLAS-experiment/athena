/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/xAODMultiMuonFilter.h"
#include "TruthUtils/HepMCHelpers.h"

StatusCode xAODMultiMuonFilter::filterInitialize()
{
  CHECK(m_truthPartContKey.initialize());
  return StatusCode::SUCCESS;
}


StatusCode xAODMultiMuonFilter::filterEvent() {
  // Retrieve TruthMuons container
  SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainer{m_truthPartContKey};
  CHECK(xTruthParticleContainer.isValid());
  int numMuons = 0;
  for (const xAOD::TruthParticle* part : *xTruthParticleContainer) {
    if (MC::isStable(part) && MC::isMuon(part)) //muon
      if(  part->pt()>= m_Ptmin && part->abseta() <= m_EtaRange )
      {
        numMuons++;
        if (numMuons >= m_NMuons)
        {
          setFilterPassed(true);
          return StatusCode::SUCCESS;
        }
      }
  }

  setFilterPassed(false);
  return StatusCode::SUCCESS;

}
