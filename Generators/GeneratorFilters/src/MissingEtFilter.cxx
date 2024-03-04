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
  declareProperty("METCut",m_METmin = 10000.);
  // Normally we'd include them, but this is unstable if using EvtGen
  declareProperty("UseNeutrinosFromHadrons",m_useHadronicNu = false);
  declareProperty("UseChargedNonShowering",m_useChargedNonShowering = false);
}


StatusCode MissingEtFilter::filterEvent() {
  double sumx(0), sumy(0);

#ifdef HEPMC3
  
ATH_MSG_ERROR(" For HEPMC3 releases xAOD filters should be used. Exiting with ERROR. ");
return StatusCode::FAILURE;
  
#endif

  McEventCollection::const_iterator itr;
  for (itr = events()->begin(); itr != events()->end(); ++itr) {
    const HepMC::GenEvent* genEvt = (*itr);
    for (const auto& pitr: *genEvt) {
      if (!MC::isGenStable(pitr)) continue;
      // Consider all non-interacting particles
      // We want Missing Transverse Momentum, not "Missing Transverse Energy"
      if (!MC::isInteracting(pitr->pdg_id()) || (m_useChargedNonShowering && MC::isChargedNonShowering(pitr->pdg_id()))) {
        bool addpart = true;
        if(!m_useHadronicNu && MC::isNeutrino(pitr->pdg_id()) && !(Common::fromWZ(pitr) || Common::fromTau(pitr)) ) {
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
  double met = std::sqrt(sumx*sumx + sumy*sumy);
  ATH_MSG_DEBUG("Totals for event: EX = " << sumx << ", EY = "<< sumy << ", ET = " << met);
  setFilterPassed(met >= m_METmin);
  return StatusCode::SUCCESS;
}
