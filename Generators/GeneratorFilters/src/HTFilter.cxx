/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Header for this module
#include "GeneratorFilters/HTFilter.h"
#include "GeneratorFilters/Common.h"

// Framework Related Headers
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SystemOfUnits.h"

// Used for retrieving the collection
#include "xAODJet/JetContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthVertex.h"
#include "StoreGate/StoreGateSvc.h"

// Other classes used by this class
#include "TruthUtils/HepMCHelpers.h"
#include "AtlasHepMC/GenEvent.h"
// #include "GeneratorObjects/McEventCollection.h"
#include "TruthUtils/HepMCHelpers.h"

//--------------------------------------------------------------------------

HTFilter::HTFilter(const std::string& name, ISvcLocator* pSvcLocator)
   : GenFilter(name,pSvcLocator) 
   , m_total(0)
   , m_passed(0)
   , m_ptfailed(0)
{
  declareProperty("MinJetPt",m_MinJetPt = 0*Gaudi::Units::GeV);  
  declareProperty("MaxJetEta",m_MaxJetEta = 10.0);
  declareProperty("TruthJetContainer", m_TruthJetContainerName = "AntiKt4TruthWZJets");
  declareProperty("MinHT",m_MinHT = 20.*Gaudi::Units::GeV);
  declareProperty("MaxHT",m_MaxHT = 14000.*Gaudi::Units::GeV);
  declareProperty("UseNeutrinosFromWZTau",m_UseNu = false, "Include neutrinos from W/Z/tau decays in the calculation of HT");
  declareProperty("UseLeptonsFromWZTau",m_UseLep = false, "Include e/mu from W/Z/tau decays in the HT");
  declareProperty("MinLeptonPt",m_MinLepPt = 0*Gaudi::Units::GeV);
  declareProperty("MaxLeptonEta",m_MaxLepEta = 10.0);
}

//--------------------------------------------------------------------------

HTFilter::~HTFilter() {
}

//---------------------------------------------------------------------------

StatusCode HTFilter::filterInitialize() {
  m_MinJetPt /= Gaudi::Units::GeV;
  m_MinLepPt /= Gaudi::Units::GeV;
  m_MinHT /= Gaudi::Units::GeV;
  m_MaxHT /= Gaudi::Units::GeV;
  if (m_MaxHT<0) m_MaxHT=9e9;

  ATH_MSG_INFO( "Configured with " << m_MinJetPt << "<p_T GeV and abs(eta)<" << m_MaxJetEta << " for jets in " << m_TruthJetContainerName );
  ATH_MSG_INFO( "Will require H_T in range " << m_MinHT << " < H_T < " << m_MaxHT );
  if (m_UseNu)  ATH_MSG_INFO( " including neutrinos" );
  if (m_UseLep)  ATH_MSG_INFO( " including W/Z/tau leptons in range "  << m_MinLepPt << "<p_T GeV and abs(eta)<" << m_MaxLepEta );

  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode HTFilter::filterFinalize() {
  ATH_MSG_INFO( "Total efficiency: " << 100.*double(m_passed)/double(m_total) << "% (" 
                << 100.*double(m_ptfailed)/double(m_total) << "% failed p_T cuts)"  );
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode HTFilter::filterEvent() {
  m_total++; // Book keeping

#ifdef HEPMC3
    

ATH_MSG_ERROR(" For HEPMC3 releases xAOD filters should be used. Exiting with ERROR. ");
return StatusCode::FAILURE;

#endif

  // Get jet container out
  const xAOD::JetContainer* truthjetTES = 0;
  if ( !evtStore()->contains<xAOD::JetContainer>( m_TruthJetContainerName ) ||
        evtStore()->retrieve( truthjetTES, m_TruthJetContainerName).isFailure() || !truthjetTES ){
    ATH_MSG_INFO( "No xAOD::JetContainer found in StoreGate with key " << m_TruthJetContainerName ); 
    setFilterPassed(m_MinHT<1.);  
    return StatusCode::SUCCESS;
  }

  // Get HT
  double HT = -1;
  for (xAOD::JetContainer::const_iterator it_truth = (*truthjetTES).begin(); it_truth != (*truthjetTES).end() ; ++it_truth) {
    if (!(*it_truth)) continue;
    if ( (*it_truth)->pt()>m_MinJetPt*Gaudi::Units::GeV && std::abs((*it_truth)->eta())<m_MaxJetEta ) {
      ATH_MSG_VERBOSE("Adding truth jet with pt " << (*it_truth)->pt()
		      << ", eta " << (*it_truth)->eta()
		      << ", phi " << (*it_truth)->phi()
		      << ", nconst = " << (*it_truth)->numConstituents());
      HT += (*it_truth)->pt();
    }
  }

  // If we are asked to include neutrinos or leptons...
  if (m_UseLep || m_UseNu){  
    // Get MC event collection
    const McEventCollection* mecc = 0;
    if ( evtStore()->retrieve(mecc).isFailure() || !mecc || mecc->size()<1 || !((*mecc)[0]) ){
      ATH_MSG_WARNING( "Could not retrieve MC Event Collection - weight might not work" );
      return StatusCode::SUCCESS;
    }

    std::vector<HepMC::GenParticlePtr> WZleptons;
    WZleptons.reserve(10);

    for (const auto& iter: *((*mecc)[0])){
      if ( !iter ) continue;
      int pdgid = iter->pdg_id();
      if (m_UseNu && MC::isNeutrino(pdgid) && MC::isGenStable(iter)) {
	if( Common::fromWZ(iter) || Common::fromTau(iter) ) {
	  HT += iter->momentum().perp();
	}
      }
      // pick muons and electrons specifically -- isLepton selects both charged leptons and neutrinos
      if (m_UseLep && (std::abs(pdgid)==11 || std::abs(pdgid)==13) && MC::isGenStable(iter)
	  && (iter)->momentum().perp()>m_MinLepPt*Gaudi::Units::GeV && std::abs(iter->momentum().eta())<m_MaxLepEta) {
	bool isFromWZ = Common::fromWZ(iter);
	if(isFromWZ || Common::fromTau(iter) ) {
	  ATH_MSG_VERBOSE("Adding W/Z/tau lepton with pt " << iter->momentum().perp()
			  << ", eta " << iter->momentum().eta()
			  << ", phi " << iter->momentum().phi()
			  << ", status " << iter->status()
			  << ", pdgId " << pdgid);
	  HT += iter->momentum().perp();
	}
      }
    }
  } // End need to access MC Event

  HT /= Gaudi::Units::GeV; // Make sure we're in GeV
  ATH_MSG_DEBUG( "HT: " << HT );

  if (HT<m_MinHT || HT>=m_MaxHT){
    ATH_MSG_DEBUG( "Failed filter on HT: " << HT << " is not between " << m_MinHT << " and " << m_MaxHT );
    setFilterPassed(false);
  } else {
    // Made it to the end - success!
    m_passed++;
    setFilterPassed(true);
  }

  return StatusCode::SUCCESS;
}


