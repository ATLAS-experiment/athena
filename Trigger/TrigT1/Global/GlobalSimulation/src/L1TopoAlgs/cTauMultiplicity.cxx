/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <cmath>

#include "cTauMultiplicity.h"
#include "isocut.h" 

#include "TrigConfData/L1Threshold.h"

#include "../IO/cTauTOBArray.h"
#include "../IO/Count.h"

#include <sstream>


namespace GlobalSim {

  cTauMultiplicity::cTauMultiplicity(const std::string& name,
				     unsigned int nbits,
				     const TrigConf::L1Threshold_cTAU& threshold,
				     const std::map<std::string, int>& isolationFW_CTAU,
				     const std::map<std::string, int>& isolationFW_CTAU_jTAUCoreScale
				     // const L1MenuResources& resources
				     ):
    m_name{name},
    m_nbits{nbits},
    m_threshold{threshold},
    m_isoFW_CTAU{isolationFW_CTAU},
    m_isoFW_CTAU_jTAUCoreScale{isolationFW_CTAU_jTAUCoreScale}
  {
    
    if (m_nbits == 0) {
      throw std::runtime_error("cTauMultiplicity m_nbits == 0 ");
    }
  }

   StatusCode
  cTauMultiplicity::run(const cTauTOBArray& cTaus,
			Count & count) {

    // candidate for being moved to the base class
    count.setNBits(m_nbits);

    int counting = 0;
    // Loop over eTau candidates
    for (const auto& etauCand : cTaus) {
      if(etauCand->tobType() != TCS::ETAU) continue;
      
      // accept = true if:
      // (isMatched==true && isIsolated==true) || (isMatched==false)
      bool accept = false;

      // Is the eTau matched to a jTau?
      bool isMatched  = false;
      
      // If matched: does the resulting cTau pass the isolation cut?
      bool isIsolated = false;

      // cTau Loose partial isolation (0 if no match is found)
      float isolation_partial_loose = 0;

      // cTau Medium partial isolation (0 if no match is found)
      float isolation_partial_medium = 0;

        // cTau Tight partial isolation (0 if no match is found)
      float isolation_partial_tight = 0;

      // cTau isolation score (0 if no match is found)
      unsigned int isoScore = 0;
    
      // Loop over jTau candidates
      for (const auto& jtauCand : cTaus) {
      
	if(jtauCand->tobType() != TCS::JTAU) continue;
	
	isMatched = cTauMatching(etauCand, jtauCand);
	
	if (isMatched) {
	  // Updated isolation condition, WP-dependent (ATR-28641)
	  // "Partial" isolation formula:
	  // I = (E_T^{jTAU Iso} + jTAUCoreScale * E_T^{jTAU Core}) / E_T^{eTAU}
	  // This formula is missing the eTAU Core substraction from
	  //the numerator, grouped with the isolation cut value

	  // variables to be  monitored
	  float denominator = static_cast<float>(etauCand->Et());

	
	  float numerator =  static_cast<float>(jtauCand->EtIso()) +
	    m_isoFW_CTAU_jTAUCoreScale.at("Loose")/1024.0 *
	    static_cast<float>(jtauCand->Et());
	  
	  m_TOB_isolation_partial_loose.push_back(numerator/denominator);
	  
	  
	  numerator = static_cast<float>(jtauCand->EtIso()) +
	    m_isoFW_CTAU_jTAUCoreScale.at("Medium")/1024.0 *
	    static_cast<float>(jtauCand->Et());
	  
	  m_TOB_isolation_partial_medium.push_back(numerator/denominator);

	  numerator = static_cast<float>(jtauCand->EtIso()) +
	    m_isoFW_CTAU_jTAUCoreScale.at("Tight")/1024.0 *
	    static_cast<float>(jtauCand->Et());
	  
	  m_TOB_isolation_partial_tight.push_back(numerator/denominator);

	  // Old isolation condition coded as in firmware:
	  // https://indico.cern.ch/event/1079697/contributions/4541419/
	  // attachments/2315137/3940824/cTAU_FirmwareAlgoProposal.pdf page 8
	
	  isoScore = convertIsoToBit(etauCand, jtauCand);
	  isIsolated =
	    isocut( TrigConf::Selection::wpToString(m_threshold.isolation()),
		    isoScore );
	  break; // Break loop when a match is found
	}
	
      } // End of jTau loop
      
      // Fill cTau TOB histograms before threshold cuts (matched cTaus only)
      if (isMatched) {
	m_TOB_et.push_back(etauCand->EtDouble());
	m_TOB_eta.push_back(etauCand->eta());
	m_TOB_phi.push_back(etauCand->phi());

	m_TOB_isolation_partial_loose.push_back(isolation_partial_loose);
	m_TOB_isolation_partial_medium.push_back(isolation_partial_medium);
	m_TOB_isolation_partial_tight.push_back(isolation_partial_tight);
	m_TOB_isoScore.push_back(isoScore);

      }
      
      // This is a good cTau
      if ( isMatched && isIsolated ) { accept = true; }
      
      // This is a non-matched eTau
      if ( not isMatched ) { accept = true; }

      // 6/2024 there have been updates to the L1TopoSimulation code
      // not included here.
      
      // Menu threshold uses 0.1 eta granularity but eFex objects
      // have 0.025 eta granularity
      // eFex eta is calculated as
      // 4*eta_tower (0.1 gran.) + seed (0.025 gran.), eta from -25 to 24
      int eta_thr;
      if ( etauCand->eta()%4 >= 0 ) {
	eta_thr = etauCand->eta() - etauCand->eta()%4;
      } else {
	eta_thr = etauCand->eta() - etauCand->eta()%4 - 4;
      }

       // Convert eta_thr to units of 0.1 to pass as an argument
      accept = accept && etauCand->Et() > m_threshold.thrValue100MeV(eta_thr/4);
      if (accept) {
	counting++;
	
	m_accept_eta.push_back(etauCand->eta());
	m_accept_et.push_back(etauCand->EtDouble());

      }
      
    } // End of eTau loop

    m_counts.push_back(counting);
    

    // Pass counting to TCS::Count object - output bits are composed there
    count.setSizeCount(counting);
    
    return StatusCode::SUCCESS;
  }


  unsigned int
  cTauMultiplicity::convertIsoToBit(const TCS::cTauTOB * etauCand,
				    const TCS::cTauTOB * jtauCand) const {
    unsigned int bit = 0;

    // Assign the tightest accept WP as default bit
    if( jtauCand->EtIso()*1024 +
	jtauCand->Et()*m_isoFW_CTAU_jTAUCoreScale.at("Loose") <
	etauCand->Et()*m_isoFW_CTAU.at("Loose") ) bit = 1;
    
    if( jtauCand->EtIso()*1024 +
	jtauCand->Et()*m_isoFW_CTAU_jTAUCoreScale.at("Medium") <
	etauCand->Et()*m_isoFW_CTAU.at("Medium") ) bit = 2;
    
    if( jtauCand->EtIso()*1024 +
	jtauCand->Et()*m_isoFW_CTAU_jTAUCoreScale.at("Tight") <
	etauCand->Et()*m_isoFW_CTAU.at("Tight") ) bit = 3;
    
    return bit;
  }


  bool
  cTauMultiplicity::cTauMatching(const TCS::cTauTOB * etauCand,
				 const TCS::cTauTOB * jtauCand) const {
    
    bool matching  = false; 
    
    // Matching is done comparing eta_tower and phi_tower (granularity = 0.1)
    // These coordinates represent the lower edge of the towers
    // (both for eFEX and jFEX)
    
    // eTau eta = 4*eta_tower + seed, eta from -25 to 24
    int eTauEtaTower;
    if (etauCand->eta()%4 >= 0 ) {
      eTauEtaTower = etauCand->eta() - etauCand->eta()%4;
    } else {
      eTauEtaTower = etauCand->eta() - etauCand->eta()%4 - 4;
    }

    int jTauEtaTower;
    if (jtauCand->eta()%4 >= 0 ) {
      jTauEtaTower = jtauCand->eta() - jtauCand->eta()%4;
    } else {
      jTauEtaTower = jtauCand->eta() - jtauCand->eta()%4 - 4;
    }
    
    //int jTauEtaTower = jtauCand->eta();
    // jTau eta = 4*eta_tower

    // eTau phi = 2*phi_tower 
    unsigned int eTauPhiTower = etauCand->phi() >> 1;     
    unsigned int jTauPhiTower = jtauCand->phi() >> 1;
    // jTau phi = 2*phi_tower + 1 (jTau coordinates are at center of tower)
    
    matching = (eTauEtaTower == jTauEtaTower) && (eTauPhiTower == jTauPhiTower);
    
    return matching;
    
  }

  unsigned int
  cTauMultiplicity::convertIsoToBit(const std::map<std::string, int>& isoFW_CTAU,
				    const std::map<std::string, int>& isoFW_CTAU_jTAUCoreScale,
				    const float jTauCore,
				    const float jTauIso,
				    const float eTauEt){ 
    unsigned int bit = 0;
    // Assign the tightest accept WP as default bit
    if( jTauIso*1024 + jTauCore*isoFW_CTAU_jTAUCoreScale.at("Loose") <
	eTauEt*isoFW_CTAU.at("Loose") ) bit = 1;
    
    if( jTauIso*1024 + jTauCore*isoFW_CTAU_jTAUCoreScale.at("Medium") <
	eTauEt*isoFW_CTAU.at("Medium") ) bit = 2;
    
    if( jTauIso*1024 + jTauCore*isoFW_CTAU_jTAUCoreScale.at("Tight") <
	eTauEt*isoFW_CTAU.at("Tight") ) bit = 3;
    
    return bit;
  }
  
  std::string cTauMultiplicity::toString() const {
    std::stringstream ss;
    ss <<  "cTauMultiplicity. name: " << m_name << '\n'
       <<  "numberOfBits: " << m_nbits;

    return ss.str();
    
  }

  const std::vector<double>&
  cTauMultiplicity::TOB_et() const {
    return m_TOB_et;
  }
     
  const std::vector<double>& cTauMultiplicity::TOB_eta() const {
    return m_TOB_eta;
  }
  
  const std::vector<double>&
  cTauMultiplicity::TOB_phi() const {
    return m_TOB_phi;
  }
     
  const std::vector<double>&
  cTauMultiplicity::TOB_isolation_partial_loose() const {
    return m_TOB_isolation_partial_loose;
  }
  
  const std::vector<double>&
  cTauMultiplicity::TOB_isolation_partial_medium() const {
    return m_TOB_isolation_partial_medium;
  }
     
  const std::vector<double>&
  cTauMultiplicity::TOB_isolation_partial_tight() const {
    return m_TOB_isolation_partial_tight;
  }
     
  const std::vector<double>&
  cTauMultiplicity::TOB_isoScore() const {
    return m_TOB_isoScore;
  }
  
  const std::vector<double>&
  cTauMultiplicity::accept_eta() const {
    return m_accept_eta;
  }
     
  const std::vector<double>&
  cTauMultiplicity::accept_et() const {
    return m_accept_et;
  }
     
  const std::vector<double>&
  cTauMultiplicity::counts() const {
    return m_counts;
  }
}

