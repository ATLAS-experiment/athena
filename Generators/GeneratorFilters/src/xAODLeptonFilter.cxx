/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/xAODLeptonFilter.h"
#include <cmath>
#include "TruthUtils/HepMCHelpers.h"

StatusCode xAODLeptonFilter::filterInitialize()
{
  CHECK(m_truthElectronContKey.initialize());
  CHECK(m_truthMuonContKey.initialize());
  return StatusCode::SUCCESS;
}


StatusCode xAODLeptonFilter::filterEvent() {

  // Retrieve TruthElectrons  container
  SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainerElectron{m_truthElectronContKey};
  CHECK(xTruthParticleContainerElectron.isValid());
  // Retrieve TruthMuons container
  SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainerMuon{m_truthMuonContKey};
  CHECK(xTruthParticleContainerMuon.isValid());

    double leading_lepton_pt_e = 0;
    double leading_lepton_pt_mu = 0;
    double leading_lepton_pt = 0;
    

   // Loop over xTruthParticleContainerElectron 
  for (const xAOD::TruthParticle* part : *xTruthParticleContainerElectron) {
    if (MC::isStable(part) && MC::isElectron(part)){ //electron
          const double pT = part->pt();
          const double eta = part->abseta();
          if (pT > leading_lepton_pt_e && std::abs(eta) <= m_EtaRange) {
	  leading_lepton_pt_e = pT;
        }       
     }
  }

  // Loop over xTruthParticleContainerMuon
  for (const xAOD::TruthParticle* part : *xTruthParticleContainerMuon) {
    if (MC::isStable(part) && MC::isMuon(part)){ //Muon
        const double pT = part->pt();
          const double eta = part->abseta();
          if (pT > leading_lepton_pt_mu && std::abs(eta) <= m_EtaRange) {
          leading_lepton_pt_mu = pT;
        }
     }
   }
  
   if (leading_lepton_pt_e > leading_lepton_pt_mu){
     leading_lepton_pt = leading_lepton_pt_e;
   } else  {
     leading_lepton_pt = leading_lepton_pt_mu;
   }

     
  ATH_MSG_DEBUG ( "Leading lepton pt = " << leading_lepton_pt << "within |eta| <= " << m_EtaRange);

  if (leading_lepton_pt < m_Ptmin) {
    setFilterPassed(false);
    ATH_MSG_DEBUG( "Fail: no e or mu found "
		   << " with pT >= " << m_Ptmin);
  } else if (leading_lepton_pt >= m_Ptmax) {
    setFilterPassed(false);
    ATH_MSG_DEBUG ( "Fail: high pt lepton veto "
		    << " pT < " << m_Ptmax );
  } else {
    setFilterPassed(true);
    ATH_MSG_DEBUG ( "Within min and max pt cuts " << m_Ptmin << ", " 
		    << m_Ptmax );
  }
  return StatusCode::SUCCESS;
}
