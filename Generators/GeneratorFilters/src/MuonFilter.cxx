/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/MuonFilter.h"
#include "TruthUtils/HepMCHelpers.h"
MuonFilter::MuonFilter(const std::string& name, ISvcLocator* pSvcLocator)
  : GenFilter(name,pSvcLocator)
{
  declareProperty("Ptcut",m_Ptmin = 10000.);
  declareProperty("Etacut",m_EtaRange = 10.0);
}


StatusCode MuonFilter::filterEvent(const EventContext& ctx) {
  McEventCollection::const_iterator itr;
  for (itr = events()->begin(); itr!=events()->end(); ++itr) {
    const HepMC::GenEvent* genEvt = (*itr);
    for (const auto& pitr: *genEvt){
    if (!MC::isStable(pitr) || !MC::isMuon(pitr))  continue;
    if (pitr->momentum().perp() < m_Ptmin || std::abs(pitr->momentum().pseudoRapidity()) > m_EtaRange) continue;
    return StatusCode::SUCCESS;
    }
  }
  setFilterPassed(false, ctx);
  return StatusCode::SUCCESS;
}
