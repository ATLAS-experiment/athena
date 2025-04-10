/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/xAODMultiLeptonFilter.h"
#include "TruthUtils/HepMCHelpers.h"

StatusCode xAODMultiLeptonFilter::filterInitialize()
{
  CHECK(m_truthElectronContKey.initialize());
  CHECK(m_truthMuonContKey.initialize());
  return StatusCode::SUCCESS;
}


StatusCode xAODMultiLeptonFilter::filterEvent() {

  // Retrieve TruthElectrons  container
  SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainerElectron{m_truthElectronContKey};
  CHECK(xTruthParticleContainerElectron.isValid());
  // Retrieve TruthMuons container
  SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainerMuon{m_truthMuonContKey};
  CHECK(xTruthParticleContainerMuon.isValid());
  int numLeptons = 0;

  for (const xAOD::TruthParticle* part : *xTruthParticleContainerElectron) {
    if (MC::isStable(part) && MC::isElectron(part)) //electron
        if(  part->pt()>= m_Ptmin && part->abseta() <= m_EtaRange )
        {
          numLeptons += 1;
          if (numLeptons >= m_NLeptons)
          {
            setFilterPassed(true);
            return StatusCode::SUCCESS;
          }
        }       
  }

  for (const xAOD::TruthParticle* part : *xTruthParticleContainerMuon) {
    if (MC::isStable(part) && MC::isMuon(part)) //Muon
        if(  part->pt()>= m_Ptmin && part->abseta() <= m_EtaRange )
        {
          numLeptons += 1;
          if (numLeptons >= m_NLeptons)
          {
            setFilterPassed(true);
            return StatusCode::SUCCESS;
          }
        }      
  }
  
  setFilterPassed(false);
  return StatusCode::SUCCESS;

}
