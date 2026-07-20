/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/MultiElectronFilter.h"
#include "TruthUtils/HepMCHelpers.h"


MultiElectronFilter::MultiElectronFilter(const std::string& name, ISvcLocator* pSvcLocator)
: GenFilter(name,pSvcLocator)
{
  declareProperty("Ptcut",m_Ptmin = 10000.);
  declareProperty("Etacut",m_EtaRange = 10.0);
  declareProperty("NElectrons",m_NElectrons = 2);
}


StatusCode MultiElectronFilter::filterEvent(const EventContext& ctx) {
  McEventCollection::const_iterator itr;
  int numElectrons = 0;
  for (itr = events()->begin(); itr != events()->end(); ++itr) {
    const HepMC::GenEvent* genEvt = (*itr);
    for (const auto& part: *genEvt) {
      if ( !MC::isStable(part)) continue;
      if ( !MC::isElectron(part)) continue;
	  if ( (part->momentum().perp() >= m_Ptmin) && std::abs(part->momentum().pseudoRapidity()) <= m_EtaRange) {
	    numElectrons++;
	  }
	}
  }
  ATH_MSG_DEBUG("Found " << numElectrons << " Electrons");
  setFilterPassed(numElectrons >= m_NElectrons, ctx);
  return StatusCode::SUCCESS;
}
