/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/xAODMETFilter.h"
#include "TruthUtils/HepMCHelpers.h"
#include "AthContainers/ConstAccessor.h"


StatusCode xAODMETFilter::filterInitialize()
{
  CHECK(m_truthPartContKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode xAODMETFilter::filterEvent() {
    
  // Retrieve TruthMET container from xAOD MET slimmer, contains (MC::isGenStable() && !MC::isInteracting()) particles
  SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainer{m_truthPartContKey};
  CHECK(xTruthParticleContainer.isValid());

  double sumx(0), sumy(0);
  for (const xAOD::TruthParticle* missingETparticle : *xTruthParticleContainer) {
    static const SG::ConstAccessor<bool> isPromptAcc ("isPrompt");
    if (!m_useHadronicNu && MC::isNeutrino(missingETparticle) &&
        !(isPromptAcc(*missingETparticle))) continue; // ignore neutrinos from hadron decays
      
      sumx += missingETparticle->px();
      sumy += missingETparticle->py();
       
  }

  double met = std::sqrt(sumx*sumx + sumy*sumy);
#ifdef HEPMC3
  const McEventCollection* mecc = 0;
  if ( evtStore()->retrieve( mecc ).isFailure() || !mecc ){ // FIXME keyless retrieve
      setFilterPassed(false);
      ATH_MSG_ERROR("Could not retrieve MC Event Collection - might not work");
      return StatusCode::SUCCESS;
    }

  McEventCollection* mec = const_cast<McEventCollection*> (&(*mecc));
  for (unsigned int i = 0; i < mec->size(); ++i) {
      if (!(*mec)[i]) continue;
    
      //for test filterHT->filterWeight
      (*mec)[i]->add_attribute("filterMET", std::make_shared<HepMC3::DoubleAttribute>(met/1000.));
  }
 
  setFilterPassed(met >= m_METmin || keepAll());
#else
  setFilterPassed(met >= m_METmin);
#endif
  return StatusCode::SUCCESS;
}

 

