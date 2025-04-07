/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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

#include "TrigConfData/L1ThrExtraInfo.h"

REGISTER_ALG_TCS(cTauMultiplicity)

TCS::cTauMultiplicity::cTauMultiplicity(const std::string & name) : CountingAlg(name) {
  setNumberOutputBits(12); //To-Do: Make this flexible to adapt to the menu. Each counting requires more than one bit       
}


TCS::StatusCode TCS::cTauMultiplicity::initialize() {
  m_threshold = dynamic_cast<const TrigConf::L1Threshold_cTAU*>(getThreshold());
  m_extraInfo = m_threshold->getExtraInfo();
 
  // Book monitoring histograms
  bookHistMult(m_histAccept, "cTauMultiplicity_accept_EtaPt_"+m_threshold->name(), "Mult_"+m_threshold->name(), "#eta#times40", "E_{t} [GeV]", 200, -200, 200, 100, 0, 100);
  bookHistMult(m_histAccept, "cTauMultiplicity_accept_counts_"+m_threshold->name(), "Mult_"+m_threshold->name(), "counts", 15, 0, 15);

  // cTau TOB monitoring histograms
  bookHistMult(m_histcTauEt, "cTauTOBEt", "Matched cTau TOB Et", "E_{t} [GeV]", 200, 0, 400);
  bookHistMult(m_histcTauPhiEta, "cTauTOBPhiEta", "Matched cTau TOB location", "#eta#times40", "#phi#times20", 200, -200, 200, 128, 0, 128);
  bookHistMult(m_histcTauEtEta, "cTauTOBEtEta", "Matched cTau TOB Et vs eta", "#eta#times40", "E_{t} [GeV]", 200, -200, 200, 200, 0, 400);
  
  const std::string wp_name = m_threshold->isolation() != WP::NONE ? TrigConf::Selection::wpToString(m_threshold->isolation()) : "NoIso";
  bookHistMult(m_histcTauIsoFraction, "cTauTOBIsoFraction", std::string("Matched cTAU ") + wp_name + " isolation fraction", wp_name + " isolation fraction", 200, 0, 10);

  bookHistMult(m_histcTauIsoMatchedPass, "cTauTOBIsoMatchedPass", "Matched cTau isolation pass", "isolation pass", 2, 0, 2);

  return StatusCode::SUCCESS;
}


// To be implemented 
TCS::StatusCode TCS::cTauMultiplicity::processBitCorrect(const TCS::InputTOBArray& input, Count& count)
{
  return process(input, count);
}


TCS::StatusCode TCS::cTauMultiplicity::process(const TCS::InputTOBArray& input, Count& count)
{
  const cTauTOBArray& cTaus = dynamic_cast<const cTauTOBArray&>(input);

  int counting = 0;
  // Loop over eTau candidates
  for(cTauTOBArray::const_iterator etauCand = cTaus.begin(); etauCand != cTaus.end(); ++etauCand) {
    if((*etauCand)->tobType() != TCS::ETAU) continue;
    
    bool accept = false; // accept = (isMatched==true && isIsolated==true) || (isMatched==false)
    bool isMatched  = false; // Is the eTau matched to a jTau?
    bool isIsolated = false; // If matched: does the resulting cTau pass the isolation cut?
    float isolation_fraction = 0; // cTAU isolation fraction

    // Loop over jTau candidates
    for(cTauTOBArray::const_iterator jtauCand = cTaus.begin(); jtauCand != cTaus.end(); ++jtauCand) {
      if((*jtauCand)->tobType() != TCS::JTAU) continue;

      isMatched = cTauMatching(*etauCand, *jtauCand);

      if(isMatched) {
        float etauCand_et = static_cast<float>((*etauCand)->Et());
        float etauCand_eta = static_cast<float>((*etauCand)->etaDouble());
        float jtauCand_et = static_cast<float>((*jtauCand)->Et());
        float jtauCand_etIso = static_cast<float>((*jtauCand)->EtIso());

        // Updated isolation condition, WP-dependent (ATR-28641)
        // I = (E_T^{jTAU Iso} + jTAUCoreScale * (E_T^{jTAU Core} - E_T^{eTAU})) / E_T^{eTAU}
        if(m_threshold->isolation() != WP::NONE) {
          isolation_fraction = (jtauCand_etIso + m_extraInfo->isolation(m_threshold->isolation(), etauCand_eta).isolation_jTAUCoreScale_d() * (jtauCand_et - etauCand_et)) / etauCand_et;
        }
        // Old isolation condition coded as in firmware: https://indico.cern.ch/event/1079697/contributions/4541419/attachments/2315137/3940824/cTAU_FirmwareAlgoProposal.pdf page 8

        // Check the isolation WP
        isIsolated = checkIsolationWP(*etauCand, *jtauCand);

        break; // Break loop when a match is found
      }

    } // End of jTau loop

    // Fill cTau TOB histograms before threshold cuts (matched cTaus only)
    if(isMatched) {
      fillHist1D(m_histcTauEt[0], (*etauCand)->EtDouble());
      fillHist2D(m_histcTauPhiEta[0], (*etauCand)->eta(), (*etauCand)->phi());
      fillHist2D(m_histcTauEtEta[0], (*etauCand)->eta(), (*etauCand)->EtDouble());
      fillHist1D(m_histcTauIsoFraction[0], isolation_fraction);
      fillHist1D(m_histcTauIsoMatchedPass[0], isMatched && isIsolated);
    }

    if(isMatched && isIsolated) accept = true; // This is a good matched cTau
    if(!isMatched) accept = true; // This is a non-matched eTau

    if(!checkeTAUWP(*etauCand)) accept = false; // Check eTAU rCore/BDT and rHad WP

    // Menu threshold uses 0.1 eta granularity but eFex objects have 0.025 eta granularity
    // eFex eta is calculated as 4*eta_tower (0.1 gran.) + seed (0.025 gran.), eta from -25 to 24
    int eta_thr;
    if((*etauCand)->eta()%4 >= 0) eta_thr = (*etauCand)->eta() - (*etauCand)->eta()%4;
    else                          eta_thr = (*etauCand)->eta() - (*etauCand)->eta()%4 - 4;

    accept = accept && (*etauCand)->Et() > m_threshold->thrValue100MeV(eta_thr/4); // Convert eta_thr to units of 0.1 to pass as an argument

    if(accept) {
      counting++;
      fillHist2D(m_histAccept[0], (*etauCand)->eta(), (*etauCand)->EtDouble());
    }
    
  } // End of eTau loop

  fillHist1D(m_histAccept[1], counting);
  
  // Pass counting to TCS::Count object - output bits are composed there
  count.setSizeCount(counting);

  return TCS::StatusCode::SUCCESS;
}


bool TCS::cTauMultiplicity::checkIsolationWP(const TCS::cTauTOB* etauCand, const TCS::cTauTOB* jtauCand) const {
  if(m_threshold->isolation() == WP::NONE) return true;
  auto iso_wp = m_extraInfo->isolation(m_threshold->isolation(), etauCand->etaDouble());
  return jtauCand->EtIso()*1024 + jtauCand->Et()*iso_wp.isolation_jTAUCoreScale_fw() < etauCand->Et()*iso_wp.isolation_fw();
}


bool TCS::cTauMultiplicity::checkeTAUWP(const TCS::cTauTOB* etauCand) const {
  if(m_threshold->isolation() == WP::NONE) return true;
  auto iso_wp = m_extraInfo->isolation(m_threshold->isolation(), etauCand->etaDouble());
  return etauCand->RCore() >= iso_wp.eTAU_rCoreMin_WP_fw() && etauCand->RHad() >= iso_wp.eTAU_rHadMin_WP_fw();
}


bool TCS::cTauMultiplicity::cTauMatching(const TCS::cTauTOB* etauCand, const TCS::cTauTOB* jtauCand) const {

  bool matching  = false; 

  // Matching is done comparing eta_tower and phi_tower (granularity = 0.1)
  // These coordinates represent the lower edge of the towers (both for eFEX and jFEX)

  // eTau eta = 4*eta_tower + seed, eta from -25 to 24
  int eTauEtaTower;
  if(etauCand->eta()%4 >= 0 ) eTauEtaTower = etauCand->eta() - etauCand->eta()%4;
  else                        eTauEtaTower = etauCand->eta() - etauCand->eta()%4 - 4;
  int jTauEtaTower;
  if (jtauCand->eta()%4 >= 0 ) jTauEtaTower = jtauCand->eta() - jtauCand->eta()%4;
  else                         jTauEtaTower = jtauCand->eta() - jtauCand->eta()%4 - 4;

  //int jTauEtaTower = jtauCand->eta();              // jTau eta = 4*eta_tower
  unsigned int eTauPhiTower = etauCand->phi() >> 1;     // eTau phi = 2*phi_tower 
  unsigned int jTauPhiTower = jtauCand->phi() >> 1;     // jTau phi = 2*phi_tower + 1 (jTau coordinates are at center of tower)

  matching = (eTauEtaTower == jTauEtaTower) && (eTauPhiTower == jTauPhiTower);

  return matching;

}

// Functions used by the HLT seeding 

#ifndef TRIGCONF_STANDALONE
size_t TCS::cTauMultiplicity::cTauMatching(const xAOD::eFexTauRoI& eTau, const xAOD::jFexTauRoIContainer& jTauRoIs) {

  // Return the index of the matched jTau if existent (otherwise return std::numeric_limits<size_t>::max())
  size_t i_matched{std::numeric_limits<size_t>::max()};
  size_t i_jTau{0};
  
  int eTauEtaTower;
  if(eTau.iEtaTopo()%4 >= 0 ) eTauEtaTower = eTau.iEtaTopo() - eTau.iEtaTopo()%4;
  else                        eTauEtaTower = eTau.iEtaTopo() - eTau.iEtaTopo()%4 - 4;
  
  for(const xAOD::jFexTauRoI* jTau : jTauRoIs) {

    // eFEX: etaTower = iEta, phiTower = iPhi
    // jFEX: etaTower = globalEta, phiTower = globalPhi
    
    int jTauEtaTopo = TSU::toTopoEta(jTau->eta());
    int jTauEtaTower;
    if(jTauEtaTopo%4 >= 0 ) jTauEtaTower = jTauEtaTopo - jTauEtaTopo%4;
    else                    jTauEtaTower = jTauEtaTopo - jTauEtaTopo%4 - 4;
  
    unsigned int jTauPhiTower = TSU::toTopoPhi(jTau->phi()) >> 1; //ignore lowest bit as jTau coordinates are taken at tower center
    unsigned int eTauPhiTower = static_cast<unsigned int>(eTau.iPhiTopo()) >> 1; //shift eTau location in the same way to stay consistent
    bool matching = ( eTauEtaTower == jTauEtaTower ) && ( eTauPhiTower == jTauPhiTower );

    if(matching) {
      i_matched = i_jTau;
      break; // Break the loop when a match is found
    }
    ++i_jTau;
  }

  return i_matched;

}


bool TCS::cTauMultiplicity::cTauMatching(const xAOD::eFexTauRoI& eTau, const xAOD::jFexTauRoI& jTau) {

  // eFEX: etaTower = iEta, phiTower = iPhi
  // jFEX: etaTower = globalEta, phiTower = globalPhi
  
  int eTauEtaTower;
  if(eTau.iEtaTopo()%4 >= 0 ) eTauEtaTower = eTau.iEtaTopo() - eTau.iEtaTopo()%4;
  else                        eTauEtaTower = eTau.iEtaTopo() - eTau.iEtaTopo()%4 - 4;
  
  int jTauEtaTopo = TSU::toTopoEta(jTau.eta());
  int jTauEtaTower;
  if(jTauEtaTopo%4 >= 0 ) jTauEtaTower = jTauEtaTopo - jTauEtaTopo%4;
  else                    jTauEtaTower = jTauEtaTopo - jTauEtaTopo%4 - 4;

  unsigned int jTauPhiTower = TSU::toTopoPhi(jTau.phi()) >> 1; //ignore lowest bit as jTau coordinates are taken at tower center
  unsigned int eTauPhiTower = static_cast<unsigned int>(eTau.iPhiTopo()) >> 1; //shift eTau location in the same way to stay consistent
    
  bool matching = ( eTauEtaTower == jTauEtaTower ) && ( eTauPhiTower == jTauPhiTower );
  return matching;

}


bool TCS::cTauMultiplicity::checkIsolationWP(const xAOD::eFexTauRoI& eTau, const xAOD::jFexTauRoI& jTau, const TrigConf::L1Threshold_cTAU& thr) {
  if(thr.isolation() == WP::NONE) return true;
  auto iso_wp = thr.getExtraInfo()->isolation(thr.isolation(), eTau.eta());
  return jTau.tobIso()*2*1024 + jTau.tobEt()*2*iso_wp.isolation_jTAUCoreScale_fw() < eTau.etTOB()*iso_wp.isolation_fw();
}


bool TCS::cTauMultiplicity::checkeTAUWP(const xAOD::eFexTauRoI& eTau, const TrigConf::L1Threshold_cTAU& thr) {
  if(thr.isolation() == WP::NONE) return true;
  auto iso_wp = thr.getExtraInfo()->isolation(thr.isolation(), eTau.eta());
  return eTau.tauOneThresholds() >= iso_wp.eTAU_rCoreMin_WP_fw() && eTau.tauTwoThresholds() >= iso_wp.eTAU_rHadMin_WP_fw();
}

#endif
