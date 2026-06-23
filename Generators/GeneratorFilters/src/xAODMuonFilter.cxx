/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/xAODMuonFilter.h"
#include "TruthUtils/HepMCHelpers.h"

StatusCode xAODMuonFilter::filterInitialize()
{
  CHECK(m_truthPartContKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode xAODMuonFilter::filterEvent(const EventContext& ctx) {
  // Retrieve TruthMuons container
  SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainer{m_truthPartContKey, ctx};
  CHECK(xTruthParticleContainer.isValid());

  for (const xAOD::TruthParticle* part : *xTruthParticleContainer) {
    if (MC::isStable(part) && MC::isMuon(part)) //muon
        if(  part->pt()>= m_Ptmin && part->abseta() <= m_EtaRange )
            return StatusCode::SUCCESS;
  }
  setFilterPassed(false, ctx);
  return StatusCode::SUCCESS;
}
