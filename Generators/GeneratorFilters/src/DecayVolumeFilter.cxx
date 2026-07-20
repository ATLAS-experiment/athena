/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Allows the user to search for particles with specified decay positions
// It will pass if there are at least MinPass many particles with LLP_PDGID
// that decay within the specified (cylindrical) volume 

#include <fstream>
#include <algorithm>
#include <cmath>

#include "GeneratorFilters/DecayVolumeFilter.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/DataSvc.h"

#include "StoreGate/StoreGateSvc.h"

#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Incident.h" 


DecayVolumeFilter::DecayVolumeFilter(const std::string& name, ISvcLocator* pSvcLocator)
  : GenFilter(name, pSvcLocator)
{}

// initialize
StatusCode DecayVolumeFilter::filterInitialize() {
  ATH_MSG_DEBUG("Configured cuts: "<< m_RCutMax<<", " << m_RCutMin<<", "<< m_zCutMax<<", "<< m_zCutMin);

  if (m_nBins>1){
    // initialize the histogram
    float lower_edge=std::min(m_RCutMin, m_zCutMin);
    float upper_edge=std::hypot(1.f*m_RCutMax, 1.f*m_zCutMax)-m_Rmargin;
    float bin_width=(upper_edge-lower_edge)/m_nBins;

    for (unsigned i=0;i<=m_nBins;++i){
      ATH_MSG_DEBUG("Bin " << i << " : ["<<lower_edge+i*bin_width<<" , "<<lower_edge+(i+1)*bin_width<<" )");
      m_bin_edges.push_back(lower_edge+i*bin_width);
      m_decay_radius[i]=0;
    }
    m_maxEntries=m_nEvents/m_nBins;
  }
  m_eventCounter=0;

  return StatusCode::SUCCESS;
}

// finalize
StatusCode DecayVolumeFilter::filterFinalize() {
  ATH_MSG_INFO("Filter statistics");
  for (const auto& pair : m_stats) {
    ATH_MSG_INFO("Events with " << pair.first << " decays within specs : " << pair.second);
  }

  if (m_nBins>1){
    reportBinStatistics(); 
  }

  return StatusCode::SUCCESS;
}

// flat binning acceptance logic
bool DecayVolumeFilter::accept_flatbinning(float r){
  auto it = std::upper_bound(m_bin_edges.begin(), m_bin_edges.end(), r);
  int bin = std::distance(m_bin_edges.begin(), it)-1;

  if ( bin<0 || bin>static_cast<int>(m_nBins) ) {
    ATH_MSG_VERBOSE("\t failed (bin index "<<bin<<"out of range [0,"<<m_nBins<<"]! ");
    return false;
  }

  if(m_decay_radius[bin]<m_maxEntries){
    ++m_decay_radius[bin];
    ATH_MSG_VERBOSE("\t passed! new bin content for bin "<<bin<<" : "<<m_decay_radius[bin]);
    return true;
  }
  ATH_MSG_VERBOSE("\t failed! bin content for bin "<<bin<<" : "<<m_decay_radius[bin]);
  return false;
}

// core filter logic
StatusCode DecayVolumeFilter::filterEvent(const EventContext& ctx) {
  int nPass = 0;
  // Loop over the events store
  for (const HepMC::GenEvent* genEvt : *events_const(ctx)) {
    for (const auto& pitr : *genEvt) {
      // Check decay position of LLP
      if ( std::abs(pitr->pdg_id()) == m_LLP_PDGID ) {
	      ATH_MSG_DEBUG("PDG: "<<pitr->pdg_id()<<" -> "<<std::fabs(pitr->momentum().pseudoRapidity()));
        
        // Check the eta range
        if ( std::abs(pitr->momentum().pseudoRapidity())>m_MaxAbsEta ) {
	        continue;
          }
        
        // Count only particles not decaying to themselves
	      bool selfDecay = false;
	      if (pitr->end_vertex()) {
	        for (const auto& child : pitr->end_vertex()->particles_out()) {
	          if ( child->pdg_id() == pitr->pdg_id() ) {
		        selfDecay = true;
		        break;}
	        }
        
        if (selfDecay){
          continue;
          }

	      HepMC::ConstGenVertexPtr vtx = pitr->end_vertex();
	      float x = vtx->position().x();
	      float y = vtx->position().y();
	      float z = vtx->position().z();
	      float Rdecay = std::sqrt(x*x + y*y);

  	    // Check if the point is within the outer cylinder
	      bool isInOuterCylinder = (Rdecay < m_RCutMax) && (std::abs(z) < m_zCutMax);
	      // Check if the point is outside the inner cylinder
	      bool isOutsideInnerCylinder = (Rdecay > m_RCutMin) || (std::abs(z) > m_zCutMin);

  	    if(isInOuterCylinder && isOutsideInnerCylinder) {
		      // only test for flat binning if event is not yet accepted
		      if (m_nBins<=1 || nPass>=1 || accept_flatbinning(std::sqrt(x*x+y*y+z*z))){
            nPass++;
            }
          }
	      }
	    }
    }
  }
  
  setFilterPassed(nPass >= m_MinPass, ctx);

  //fill statistics
  m_stats[nPass]++;
  m_eventCounter++;
  
  // Print stats every 10k events
  if (m_eventCounter%10000==0){
    reportBinStatistics(); 
    
    ATH_MSG_INFO("Success statistics: ");
    for (const auto& pair : m_stats) {
      ATH_MSG_INFO("Events with " << pair.first << " decays within specs: " << pair.second);
    }
  }
  
  return StatusCode::SUCCESS;
}


// stats printout
void DecayVolumeFilter::reportBinStatistics() const {
  if (m_nBins>1){
    ATH_MSG_INFO("Bin statistics: ");
    for (unsigned i=0;i<=m_nBins;i++){
	    ATH_MSG_INFO("Bin "<<i<<" : "<<m_decay_radius.at(i));
      }
    }
}
