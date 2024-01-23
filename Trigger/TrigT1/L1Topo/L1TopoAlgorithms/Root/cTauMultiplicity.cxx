/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
//  TopoCore
//

#include <cmath>

#include "L1TopoAlgorithms/cTauMultiplicity.h"
#include "L1TopoCommon/Exception.h"
#include "L1TopoInterfaces/Count.h"
#include "L1TopoSimulationUtils/Conversions.h"

#include "L1TopoEvent/TOBArray.h"
#include "L1TopoEvent/cTauTOBArray.h"

REGISTER_ALG_TCS(cTauMultiplicity)

// constructor
TCS::cTauMultiplicity::cTauMultiplicity(const std::string & name) : CountingAlg(name) {
  setNumberOutputBits(12); //To-Do: Make this flexible to adapt to the menu. Each counting requires more than one bit       
}


// destructor
TCS::cTauMultiplicity::~cTauMultiplicity() {}


TCS::StatusCode
TCS::cTauMultiplicity::initialize() {

  m_threshold = getThreshold(); 
  m_isoFW_CTAU = isolationFW_CTAU();
  m_isoFW_CTAU_jTAUCoreScale = isolationFW_CTAU_jTAUCoreScale();
  TRG_MSG_DEBUG("Initializing cTauMultiplicity L1Topo Algorithm");
  for(const auto& [wp, iso] : m_isoFW_CTAU) {
    TRG_MSG_DEBUG("WP \"" << wp << "\": R=" << iso << ", jTAUCoreScale=" << m_isoFW_CTAU_jTAUCoreScale[wp]);
  }

 
  // book histograms
  std::string hname_accept = "cTauMultiplicity_accept_EtaPt_"+m_threshold->name();
  bookHistMult(m_histAccept, hname_accept, "Mult_"+m_threshold->name(), "#eta#times40", "E_{t} [GeV]", 200, -200, 200, 100, 0, 100);

  hname_accept = "cTauMultiplicity_accept_counts_"+m_threshold->name();
  bookHistMult(m_histAccept, hname_accept, "Mult_"+m_threshold->name(), "counts", 15, 0, 15);

  // cTau TOB monitoring histograms
  bookHistMult(m_histcTauEt, "cTauTOBEt", "Matched cTau TOB Et", "E_{t} [GeV]", 200, 0, 400);
  bookHistMult(m_histcTauPhiEta, "cTauTOBPhiEta", "Matched cTau TOB location", "#eta#times40", "#phi#times20", 200, -200, 200, 128, 0, 128);
  bookHistMult(m_histcTauEtEta, "cTauTOBEtEta", "Matched cTau TOB Et vs eta", "#eta#times40", "E_{t} [GeV]", 200, -200, 200, 200, 0, 400);
  bookHistMult(m_histcTauPartialIsoLoose, "cTauTOBPartialIsoLoose", "Matched cTau Loose partial isolation", "Loose isolation", 200, 0, 10);
  bookHistMult(m_histcTauPartialIsoMedium, "cTauTOBPartialIsoMedium", "Matched cTau Medium partial isolation", "Medium isolation", 200, 0, 10);
  bookHistMult(m_histcTauPartialIsoTight, "cTauTOBPartialIsoTight", "Matched cTau Tight partial isolation", "Tight isolation", 200, 0, 10);
  bookHistMult(m_histcTauIsoScore, "cTauTOBIsoScore", "Matched cTau isolation score", "isolation score", 4, 0, 4);

  return StatusCode::SUCCESS;
}


// To be implemented 
TCS::StatusCode
TCS::cTauMultiplicity::processBitCorrect( const TCS::InputTOBArray & input, Count & count)
{
  return process(input, count);
}



TCS::StatusCode
TCS::cTauMultiplicity::process( const TCS::InputTOBArray & input, Count & count )
{

  // Grab the threshold and cast it into the right type                                                                                                                                                 
  const auto& cTauThr = dynamic_cast<const TrigConf::L1Threshold_cTAU &>(*m_threshold);

  const cTauTOBArray & cTaus = dynamic_cast<const cTauTOBArray&>(input);

  int counting = 0;
  // Loop over eTau candidates
  for(cTauTOBArray::const_iterator etauCand = cTaus.begin(); etauCand != cTaus.end(); ++etauCand ) {

    if((*etauCand)->tobType() != TCS::ETAU) continue;
    
    bool accept = false;     	// accept = true if (isMatched==true && isIsolated==true) || (isMatched==false)
    bool isMatched  = false;	// Is the eTau matched to a jTau?
    bool isIsolated = false;    // If matched: does the resulting cTau pass the isolation cut?
    float isolation_partial_loose = 0;  // cTau Loose partial isolation (0 if no match is found)
    float isolation_partial_medium = 0; // cTau Medium partial isolation (0 if no match is found)
    float isolation_partial_tight = 0;  // cTau Tight partial isolation (0 if no match is found)
    unsigned int isoScore = 0;  // cTau isolation score (0 if no match is found)

    // Loop over jTau candidates
    for(cTauTOBArray::const_iterator jtauCand = cTaus.begin(); jtauCand != cTaus.end(); ++jtauCand ) {
      
      if((*jtauCand)->tobType() != TCS::JTAU) continue;

      isMatched = cTauMatching( *etauCand, *jtauCand );

      if (isMatched) {
	// Updated isolation condition, WP-dependent (ATR-28641)
	// "Partial" isolation formula: I = (E_T^{jTAU Iso} + jTAUCoreScale * E_T^{jTAU Core}) / E_T^{eTAU}
	// This formula is missing the eTAU Core substraction from the numerator, grouped with the isolation cut value
	isolation_partial_loose = (static_cast<float>((*jtauCand)->EtIso()) + m_isoFW_CTAU_jTAUCoreScale.at("Loose")/1024.0 * static_cast<float>((*jtauCand)->Et())) / static_cast<float>((*etauCand)->Et()); // Internal variable for monitoring
	isolation_partial_medium = (static_cast<float>((*jtauCand)->EtIso()) + m_isoFW_CTAU_jTAUCoreScale.at("Medium")/1024.0 * static_cast<float>((*jtauCand)->Et())) / static_cast<float>((*etauCand)->Et()); // Internal variable for monitoring
	isolation_partial_tight = (static_cast<float>((*jtauCand)->EtIso()) + m_isoFW_CTAU_jTAUCoreScale.at("Tight")/1024.0 * static_cast<float>((*jtauCand)->Et())) / static_cast<float>((*etauCand)->Et()); // Internal variable for monitoring
        // Old isolation condition coded as in firmware: https://indico.cern.ch/event/1079697/contributions/4541419/attachments/2315137/3940824/cTAU_FirmwareAlgoProposal.pdf page 8

        isoScore = convertIsoToBit( *etauCand, *jtauCand );
        isIsolated = isocut( TrigConf::Selection::wpToString(cTauThr.isolation()), isoScore );
        break; // Break loop when a match is found
      }

    } // End of jTau loop

    // Fill cTau TOB histograms before threshold cuts (matched cTaus only)
    if (isMatched) {
      fillHist1D( m_histcTauEt[0], (*etauCand)->EtDouble() );
      fillHist2D( m_histcTauPhiEta[0], (*etauCand)->eta(), (*etauCand)->phi() );
      fillHist2D( m_histcTauEtEta[0], (*etauCand)->eta(), (*etauCand)->EtDouble() );
      fillHist1D( m_histcTauPartialIsoLoose[0], isolation_partial_loose );
      fillHist1D( m_histcTauPartialIsoMedium[0], isolation_partial_medium );
      fillHist1D( m_histcTauPartialIsoTight[0], isolation_partial_tight );
      fillHist1D( m_histcTauIsoScore[0], isoScore );
    }

    // This is a good cTau
    if ( isMatched && isIsolated ) { accept = true; }
    // This is a non-matched eTau
    if ( not isMatched ) { accept = true; }

    // Menu threshold uses 0.1 eta granularity but eFex objects have 0.025 eta granularity
    // eFex eta is calculated as 4*eta_tower (0.1 gran.) + seed (0.025 gran.), eta from -25 to 24
    int eta_thr;
    if ( (*etauCand)->eta()%4 >= 0 ) { eta_thr = (*etauCand)->eta() - (*etauCand)->eta()%4; }
    else                             { eta_thr = (*etauCand)->eta() - (*etauCand)->eta()%4 - 4; }

    accept = accept && (*etauCand)->Et() > cTauThr.thrValue100MeV(eta_thr/4); // Convert eta_thr to units of 0.1 to pass as an argument
    if (accept) {
      counting++;
      fillHist2D( m_histAccept[0], (*etauCand)->eta(), (*etauCand)->EtDouble() );
    }
    
  } // End of eTau loop

  fillHist1D( m_histAccept[1], counting);
  
  // Pass counting to TCS::Count object - output bits are composed there                                                                                                                               
  
  count.setSizeCount(counting);

  return TCS::StatusCode::SUCCESS;
  
}


unsigned int
TCS::cTauMultiplicity::convertIsoToBit(const TCS::cTauTOB * etauCand, const TCS::cTauTOB * jtauCand) const {
  unsigned int bit = 0;

  // Assign the tightest accept WP as default bit
  if( jtauCand->EtIso()*1024 + jtauCand->Et()*m_isoFW_CTAU_jTAUCoreScale.at("Loose") < etauCand->Et()*m_isoFW_CTAU.at("Loose") ) bit = 1;
  if( jtauCand->EtIso()*1024 + jtauCand->Et()*m_isoFW_CTAU_jTAUCoreScale.at("Medium") < etauCand->Et()*m_isoFW_CTAU.at("Medium") ) bit = 2;
  if( jtauCand->EtIso()*1024 + jtauCand->Et()*m_isoFW_CTAU_jTAUCoreScale.at("Tight") < etauCand->Et()*m_isoFW_CTAU.at("Tight") ) bit = 3;
  
  return bit;
}


bool
TCS::cTauMultiplicity::cTauMatching(const TCS::cTauTOB * etauCand, const TCS::cTauTOB * jtauCand) const {

  bool matching  = false; 

  // Matching is done comparing eta_tower and phi_tower (granularity = 0.1)
  // These coordinates represent the lower edge of the towers (both for eFEX and jFEX)

  // eTau eta = 4*eta_tower + seed, eta from -25 to 24
  int eTauEtaTower;
  if (etauCand->eta()%4 >= 0 ) { eTauEtaTower = etauCand->eta() - etauCand->eta()%4; }
  else                         { eTauEtaTower = etauCand->eta() - etauCand->eta()%4 - 4; }
  int jTauEtaTower;
  if (jtauCand->eta()%4 >= 0 ) { jTauEtaTower = jtauCand->eta() - jtauCand->eta()%4; }
  else                         { jTauEtaTower = jtauCand->eta() - jtauCand->eta()%4 - 4; }

  //int jTauEtaTower = jtauCand->eta();              // jTau eta = 4*eta_tower
  unsigned int eTauPhiTower = etauCand->phi() >> 1;     // eTau phi = 2*phi_tower 
  unsigned int jTauPhiTower = jtauCand->phi() >> 1;     // jTau phi = 2*phi_tower + 1 (jTau coordinates are at center of tower)

  matching = (eTauEtaTower == jTauEtaTower) && (eTauPhiTower == jTauPhiTower);

  return matching;

}

// Functions used by the HLT seeding 

#ifndef TRIGCONF_STANDALONE
size_t
TCS::cTauMultiplicity::cTauMatching(const xAOD::eFexTauRoI & eTau, const xAOD::jFexTauRoIContainer & jTauRoIs) {

  // Return the index of the matched jTau if existent (otherwise return std::numeric_limits<size_t>::max())
  size_t i_matched{std::numeric_limits<size_t>::max()};
  size_t i_jTau{0};
  
  int eTauEtaTower;
  if (eTau.iEtaTopo()%4 >= 0 ) { eTauEtaTower = eTau.iEtaTopo() - eTau.iEtaTopo()%4; }
  else                         { eTauEtaTower = eTau.iEtaTopo() - eTau.iEtaTopo()%4 - 4; }
  
  for (const xAOD::jFexTauRoI* jTau : jTauRoIs) {

    // eFEX: etaTower = iEta, phiTower = iPhi
    // jFEX: etaTower = globalEta, phiTower = globalPhi
    
    int jTauEtaTopo = TSU::toTopoEta(jTau->eta());
    int jTauEtaTower;
    if (jTauEtaTopo%4 >= 0 ) { jTauEtaTower = jTauEtaTopo - jTauEtaTopo%4; }
    else                     { jTauEtaTower = jTauEtaTopo - jTauEtaTopo%4 - 4; }
  
    unsigned int jTauPhiTower = TSU::toTopoPhi(jTau->phi()) >> 1; //ignore lowest bit as jTau coordinates are taken at tower center
    unsigned int eTauPhiTower = static_cast<unsigned int>(eTau.iPhiTopo()) >> 1; //shift eTau location in the same way to stay consistent
    bool matching = ( eTauEtaTower == jTauEtaTower ) && ( eTauPhiTower == jTauPhiTower );

    if ( matching ) {
      i_matched = i_jTau;
      break; // Break the loop when a match is found
    }
    ++i_jTau;
  }

  return i_matched;

}

bool 
TCS::cTauMultiplicity::cTauMatching(const xAOD::eFexTauRoI & eTau, const xAOD::jFexTauRoI & jTau) {

  // eFEX: etaTower = iEta, phiTower = iPhi
  // jFEX: etaTower = globalEta, phiTower = globalPhi
  
  int eTauEtaTower;
  if (eTau.iEtaTopo()%4 >= 0 ) { eTauEtaTower = eTau.iEtaTopo() - eTau.iEtaTopo()%4; }
  else                         { eTauEtaTower = eTau.iEtaTopo() - eTau.iEtaTopo()%4 - 4; }
  
  int jTauEtaTopo = TSU::toTopoEta(jTau.eta());
  int jTauEtaTower;
  if (jTauEtaTopo%4 >= 0 ) { jTauEtaTower = jTauEtaTopo - jTauEtaTopo%4; }
  else                     { jTauEtaTower = jTauEtaTopo - jTauEtaTopo%4 - 4; }

  unsigned int jTauPhiTower = TSU::toTopoPhi(jTau.phi()) >> 1; //ignore lowest bit as jTau coordinates are taken at tower center
  unsigned int eTauPhiTower = static_cast<unsigned int>(eTau.iPhiTopo()) >> 1; //shift eTau location in the same way to stay consistent
    
  bool matching = ( eTauEtaTower == jTauEtaTower ) && ( eTauPhiTower == jTauPhiTower );
  return matching;

}

unsigned int
TCS::cTauMultiplicity::convertIsoToBit(const std::map<std::string, int>& isoFW_CTAU, const std::map<std::string, int>& isoFW_CTAU_jTAUCoreScale, const float jTauCore, const float jTauIso, const float eTauEt){ 
  unsigned int bit = 0;
  // Assign the tightest accept WP as default bit
  if( jTauIso*1024 + jTauCore*isoFW_CTAU_jTAUCoreScale.at("Loose") < eTauEt*isoFW_CTAU.at("Loose") ) bit = 1;
  if( jTauIso*1024 + jTauCore*isoFW_CTAU_jTAUCoreScale.at("Medium") < eTauEt*isoFW_CTAU.at("Medium") ) bit = 2;
  if( jTauIso*1024 + jTauCore*isoFW_CTAU_jTAUCoreScale.at("Tight") < eTauEt*isoFW_CTAU.at("Tight") ) bit = 3;
  return bit;
}

#endif
