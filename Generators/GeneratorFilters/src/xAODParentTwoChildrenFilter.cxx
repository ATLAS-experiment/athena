/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/xAODParentTwoChildrenFilter.h"
#include "TruthUtils/HepMCHelpers.h"
#include "CxxUtils/BasicTypes.h"
#include "xAODTruth/TruthVertexContainer.h"

xAODParentTwoChildrenFilter::xAODParentTwoChildrenFilter(const std::string& name, ISvcLocator* pSvcLocator)
  : GenFilter(name,pSvcLocator)
{
  
}


StatusCode xAODParentTwoChildrenFilter::filterInitialize() {
 if (m_PDGParent.size() == 0) ATH_MSG_ERROR("PDGParent[] not set ");
 if (m_PDGChild.size() == 0) ATH_MSG_ERROR("PDGChild[] not set ");
 for (int i=0; i < int(m_PDGParent.size()); i++) ATH_MSG_DEBUG("PDGParent["<<i<<"] = " << m_PDGParent[i]);
 ATH_MSG_DEBUG("PtMinParent = " << m_PtMinParent);
 ATH_MSG_DEBUG("EtaRangeParent = " << m_EtaRangeParent);
 for (int i=0; i < int(m_PDGChild.size()); i++) ATH_MSG_DEBUG("PDGChild["<<i<<"] = " << m_PDGChild[i]);
 ATH_MSG_DEBUG("PtMinChild = " << m_PtMinChild);
 ATH_MSG_DEBUG("EtaRangeChild = " << m_EtaRangeChild);
 return StatusCode::SUCCESS;
}


StatusCode xAODParentTwoChildrenFilter::filterEvent() {
    ATH_MSG_DEBUG(" ParentTwoChildrenFilter filtering for: "
                << "Parent (" << m_PDGParent[0] << ") --> Child (" << m_PDGChild[0] << ") + antiparticle and "
                << "Parent (" << m_PDGParent[0] << ") --> Child (" << m_PDGChild[1] << ") + antiparticle." );
  int N_Child[2][2];
  for (int i = 0; i < 2; i++) {
    N_Child[i][0] = 0;
    N_Child[i][1] = 0;
  }

// Retrieve TruthGen container from xAOD Gen slimmer, contains all particles witout barcode_zero and
// duplicated barcode ones
  const xAOD::TruthParticleContainer* xTruthParticleContainer;
  if (evtStore()->retrieve(xTruthParticleContainer, "TruthGen").isFailure()) {
      ATH_MSG_ERROR("No TruthParticle collection with name " << "TruthGen" << " found in StoreGate!");
      return StatusCode::FAILURE;
  }

  // Loop over all particles in the event and build up the grid
  unsigned int nPart = xTruthParticleContainer->size();
  for (unsigned int iPart = 0; iPart < nPart; ++iPart) {
          const xAOD::TruthParticle* pitr =  (*xTruthParticleContainer)[iPart];

          int id = pitr->pdgId();
	  if (std::abs(id) != m_PDGParent[0]) continue;
    	  if (pitr->pt() < m_PtMinParent) continue;


     // Verify if we got a valid pointer and retrieve the number of daughters
      const  xAOD::TruthVertex* decayVtx = pitr->decayVtx();
      if (!decayVtx) continue; 
      int n_daughters = decayVtx->nOutgoingParticles();      
      if (n_daughters < 2) continue; 


      int neutralPar = 0;
      int num_outgoing_particles = decayVtx->nOutgoingParticles();          
      for (int j = 0; j< num_outgoing_particles; j++) {
                const  xAOD::TruthParticle* thisChild =  decayVtx->outgoingParticle(j);    
    
       ATH_MSG_DEBUG(" ParentTwoChildrenFilter: parent ==> " <<pitr->pdgId() << " child ===> "  <<thisChild->pdgId());
       for (int i = 0; i < 2; i++) {
	  if ( std::abs(thisChild->pdgId()) == m_PDGChild[i]) {
	    int antiparticle = ( MC::charge(m_PDGChild[i]) == 0 ? 1 : -1 ); // assume that zero charge particles are their own anti-particle
	    if ( thisChild->pdgId() == m_PDGChild[i] ) {
                 if( (thisChild->pt() >= m_PtMinChild) ) {
		if(antiparticle == 1) {
		  neutralPar++;
	  if(neutralPar == 1) N_Child[i][0]++; 
		}
		else N_Child[i][0]++;
	      }
	   }
            if ( thisChild->pdgId() == antiparticle * m_PDGChild[i] ) {
	      if( (thisChild->pt() >= m_PtMinChild) ) {
		if(antiparticle == 1){
		  if (neutralPar == 2) N_Child[i][1]++; 
		}
		else N_Child[i][1]++;
	      }
	    }
	  }
	}
      } 
    }  //lopp over TruthParticles
  
  setFilterPassed(N_Child[0][0] >= 1 && N_Child[0][1] >= 1 && N_Child[1][0] >= 1 && N_Child[1][1] >= 1);
  return StatusCode::SUCCESS;
}
