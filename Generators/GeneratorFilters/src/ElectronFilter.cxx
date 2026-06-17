/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/ElectronFilter.h"
#include "TruthUtils/HepMCHelpers.h"

ElectronFilter::ElectronFilter(const std::string& name, ISvcLocator* pSvcLocator)
  : GenFilter(name, pSvcLocator)
{
  declareProperty("Ptcut", m_Ptmin = 10000.);
  declareProperty("Etacut", m_EtaRange = 10.0);
}


StatusCode ElectronFilter::filterEvent(const EventContext& ctx) {
  for (McEventCollection::const_iterator itr = events()->begin(); itr != events()->end(); ++itr) {
    const HepMC::GenEvent* genEvt = *itr;
    for (const auto& part: *genEvt) {
      if (MC::isStable(part) && MC::isElectron(part)) { // electron
        if (part->momentum().perp() >= m_Ptmin && std::abs(part->momentum().pseudoRapidity()) <= m_EtaRange) {
          return StatusCode::SUCCESS;
        }
      }
    }
  }
  setFilterPassed(false, ctx);
  return StatusCode::SUCCESS;
}
