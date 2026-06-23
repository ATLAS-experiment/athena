/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration 
*/
#include "GeneratorFilters/SplitPhotonFilter.h"


SplitPhotonFilter::SplitPhotonFilter(const std::string& name, ISvcLocator* pSvcLocator)
  : GenFilter(name, pSvcLocator)
{
  declareProperty("Ptcut", m_Ptmin = 15000.);
  declareProperty("Etacut", m_EtaRange = 2.50);
  declareProperty("NPhotons", m_NPhotons = 1);
  declareProperty("AcceptedSplit", m_dauPdg);
}


StatusCode SplitPhotonFilter::filterEvent(const EventContext& ctx) {
  int NPhotons = 0;
  bool GoodFlav = m_dauPdg.size() == 0 ? true : false;
  McEventCollection::const_iterator itr;
  for (itr = events_const(ctx)->begin(); itr!=events_const(ctx)->end(); ++itr) {
    const HepMC::GenEvent* genEvt = (*itr);

// ** Loop on all particles **
    for(const auto& part: *genEvt) {

      // Check for a photon with desired kinematics
      if ( (MC::isPhoton(part)) ) {
        if ( (part->momentum().perp() >= m_Ptmin) &&
             fabs(part->momentum().pseudoRapidity()) <= m_EtaRange) {

	  // First find a direct photon (not from hadron decay)
          bool fromHadron(false);
          auto firstParent1 = part -> production_vertex() -> particles_in().begin();
          auto endParent1 = part -> production_vertex() -> particles_in().end();
        for (auto thisParent1 = firstParent1; thisParent1 != endParent1; ++thisParent1 ) {
            int pdgindex =  abs((*thisParent1)->pdg_id());
            if (pdgindex > 100) {
              fromHadron = true;
	      break;
            }
          }
          if (fromHadron)
	    continue;

	  if ( part->end_vertex() && part->end_vertex()->particles_out_size() > 1 ) {
	    ATH_MSG_DEBUG("A split photon");

// find daughters
            auto dauBegin = part->end_vertex()->particles_out().begin();
            auto dauEnd   = part->end_vertex()->particles_out().end();
	    for (auto dau = dauBegin; dau != dauEnd; dau++) {
	      int pdgid = (*dau)->pdg_id();
	      if (std::find(m_dauPdg.begin(),m_dauPdg.end(),abs(pdgid)) != m_dauPdg.end())
		//Argh I should break here... anyway should not waste too much time to continue the loop even when a good daughter is found
		GoodFlav = true; 
	      ATH_MSG_DEBUG("Daughter : pdg = " << pdgid);
	    }
	    NPhotons++;
	  }

        } // kinematic requirements
      }   // photon
    } // part
   }  // itr
  

  if (NPhotons >= m_NPhotons && GoodFlav) return StatusCode::SUCCESS;
  setFilterPassed(false, ctx);
  return StatusCode::SUCCESS;
}
