/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/ZtoLeptonFilter.h"
#include "TruthUtils/HepMCHelpers.h"

ZtoLeptonFilter::ZtoLeptonFilter(const std::string& name, ISvcLocator* pSvcLocator)
  : GenFilter(name, pSvcLocator)
{
  declareProperty("Ptcut",m_Ptmin = .0);
  declareProperty("Etacut",m_EtaRange = 10.0);
}


StatusCode ZtoLeptonFilter::filterEvent(const EventContext& ctx) {
  McEventCollection::const_iterator itr;
  for (itr = events()->begin(); itr!=events()->end(); ++itr) {
    const HepMC::GenEvent* genEvt = (*itr);
    for ( const auto& pitr: genEvt->particles()) {
      if (MC::isZ(pitr)) {
        if ( !pitr->end_vertex() && !MC::isPhysical(pitr)) continue; // Allow status 3 Zs with no end vertex
        else if (!pitr->end_vertex() ){
          // Found a Z boson with no end vertex and status!=3 .  Something is sick about this event
          break;
        }
        // Z children
        for (const auto& thisChild: pitr->end_vertex()->particles_out()) {
          if ( MC::isElectron(thisChild) || MC::isMuon(thisChild) || MC::isTau(thisChild) ) {
            return StatusCode::SUCCESS;
          }
        }
      }
    }
  }
  setFilterPassed(false, ctx);
  return StatusCode::SUCCESS;
}
