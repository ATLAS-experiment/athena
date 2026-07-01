/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/MultiLeptonFilter.h"
#include "TruthUtils/HepMCHelpers.h"


MultiLeptonFilter::MultiLeptonFilter(const std::string& name, ISvcLocator* pSvcLocator)
: GenFilter(name, pSvcLocator)
{
  declareProperty("Ptcut", m_Ptmin = 10000.);
  declareProperty("Etacut", m_EtaRange = 10.0);
  declareProperty("NLeptons", m_NLeptons = 4);
}


StatusCode MultiLeptonFilter::filterEvent(const EventContext& ctx) {
  McEventCollection::const_iterator itr;
  int numLeptons = 0;
  for (itr = events()->begin(); itr != events()->end(); ++itr) {
    const HepMC::GenEvent* genEvt = *itr;
    for (const auto& part: *genEvt) {
      if ( !MC::isStable(part)) continue;
	  if ( MC::isElectron(part) || MC::isMuon(part) ) {
	    if (part->momentum().perp() >= m_Ptmin && std::abs(part->momentum().pseudoRapidity()) <= m_EtaRange) {
	      numLeptons += 1;
	    }
	  }
	}
  }
  ATH_MSG_DEBUG("Found " << numLeptons << " Leptons");
  setFilterPassed(numLeptons >= m_NLeptons, ctx);
  return StatusCode::SUCCESS;
}
