/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/MissingEtFilter.h"
#include "GeneratorFilters/Common.h"
#include "TruthUtils/HepMCHelpers.h"
#include "TruthUtils/HepMCHelpers.h"


MissingEtFilter::MissingEtFilter(const std::string& name, ISvcLocator* pSvcLocator)
  : GenFilter(name,pSvcLocator)
{
}


StatusCode MissingEtFilter::filterEvent() {
  double sumx(0), sumy(0);

#ifdef HEPMC3

if (! m_allowOld) {  
  ATH_MSG_ERROR(" For HEPMC3 releases xAOD filters should be used. Exiting with ERROR. ");
  return StatusCode::FAILURE;
}
  
#endif

  McEventCollection::const_iterator itr;
  for (itr = events()->begin(); itr != events()->end(); ++itr) {
    const HepMC::GenEvent* genEvt = (*itr);
    for (const auto& pitr: *genEvt) {
      if (!MC::isGenStable(pitr)) continue;
      // Consider all non-interacting particles
      // We want Missing Transverse Momentum, not "Missing Transverse Energy"
      if (!MC::isInteracting(pitr) || (m_useChargedNonShowering && MC::isChargedNonShowering(pitr))) {
        bool addpart = true;
        if(!m_useHadronicNu && MC::isNeutrino(pitr) && !(Common::fromWZorTau(pitr)) ) {
          addpart = false; // ignore neutrinos from hadron decays
        }
        if(addpart) {
          ATH_MSG_VERBOSE("Found noninteracting particle: ID = " << pitr->pdg_id() << " PX = " << pitr->momentum().px() << " PY = "<< pitr->momentum().py());
          sumx += pitr->momentum().px();
          sumy += pitr->momentum().py();
        }
      }
    }
  }

  // Now see what the total missing Et is and compare to minimum
  double met = std::hypot(sumx,sumy);
  ATH_MSG_DEBUG("Totals for event: EX = " << sumx << ", EY = "<< sumy << ", ET = " << met);
  setFilterPassed(met >= m_METmin);
  return StatusCode::SUCCESS;
}
