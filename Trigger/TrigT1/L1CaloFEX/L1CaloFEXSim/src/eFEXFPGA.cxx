/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFEXFPGA  -  description
//                              -------------------
//     begin                : 15 10 2019
//     email                : jacob.julian.kempster@cern.ch
//  ***************************************************************************/
#include "eFEXFPGA.h"
#include "L1CaloFEXSim/eTowerContainer.h"
#include "L1CaloFEXSim/eFEXegAlgo.h"
#include "L1CaloFEXSim/eFEXegTOB.h"
#include "L1CaloFEXSim/eFEXOutputCollection.h"
#include "eFEXtauAlgo.h"
#include "L1CaloFEXSim/eFEXtauTOB.h"
#include <vector>
#include "StoreGate/ReadHandle.h"
#include "TrigConfData/L1Menu.h"
#include <unordered_map>

#include <iostream>
#include <fstream>

namespace LVL1 {

  // default constructor for persistency

eFEXFPGA::eFEXFPGA(const std::string& type,const std::string& name,const IInterface* parent):
  AthAlgTool(type,name,parent)
{
  declareInterface<eFEXFPGA>(this);
}
 
    
  /** Destructor */
  eFEXFPGA::~eFEXFPGA()
  {
  }

//---------------- Initialisation -------------------------------------------------
  
StatusCode eFEXFPGA::initialize()
{

  ATH_CHECK(m_eTowerContainerKey.initialize());
  ATH_CHECK( m_eFEXegAlgoTool.retrieve() );
  ATH_CHECK( m_eFEXtauAlgoTool.retrieve() );
  ATH_CHECK( m_eFEXtauBDTAlgoTool.retrieve() );
  
  
  ATH_CHECK(m_l1MenuKey.initialize());

  return StatusCode::SUCCESS;
}
  

StatusCode eFEXFPGA::init(int id, int efexid)
{
  m_id = id;
  m_efexid = efexid;

  return StatusCode::SUCCESS;
}

void eFEXFPGA::reset(){

  m_id = -1;
  m_efexid = -1;
}

StatusCode eFEXFPGA::execute(eFEXOutputCollection* inputOutputCollection){
  m_emTobObjects.clear();
  m_tauHeuristicTobObjects.clear();
  m_tauBDTTobObjects.clear();

  SG::ReadHandle<eTowerContainer> eTowerContainer(m_eTowerContainerKey/*,ctx*/);
  if(!eTowerContainer.isValid()){
    ATH_MSG_FATAL("Could not retrieve container " << m_eTowerContainerKey.key() );
    return StatusCode::FAILURE;
  }

  // Retrieve the L1 menu configuration
  SG::ReadHandle<TrigConf::L1Menu> l1Menu (m_l1MenuKey/*, ctx*/);
  ATH_CHECK(l1Menu.isValid());

  auto & thr_eEM = l1Menu->thrExtraInfo().eEM();
  auto & thr_eTAU = l1Menu->thrExtraInfo().eTAU();

  auto emAlgoVersion = thr_eEM.algoVersion();
  auto tauAlgoVersion = thr_eTAU.algoVersion();


  // Define eta range to consider extra towers in edge cases
  int min_eta;
  int overflow_eta;
  if ((m_efexid%3 == 0) && (m_id == 0)) {
    min_eta = 0;
  } else {
    min_eta = 1;
  }
  if ((m_efexid%3 == 2) && (m_id == 3)) {
    overflow_eta = 6;
  } else {
    overflow_eta = 5;
  }
  
  for(int ieta = min_eta; ieta < overflow_eta; ieta++) {
    for(int iphi = 1; iphi < 9; iphi++) {

      int tobtable[3][3]={
        {ieta > 0 ? m_eTowersIDs[iphi-1][ieta-1] : 0,
         m_eTowersIDs[iphi-1][ieta],
         ieta < 5 ? m_eTowersIDs[iphi-1][ieta+1] : 0},

        {ieta > 0 ? m_eTowersIDs[iphi][ieta-1] : 0,
         m_eTowersIDs[iphi][ieta],
         ieta < 5 ? m_eTowersIDs[iphi][ieta+1] : 0},

        {ieta > 0 ? m_eTowersIDs[iphi+1][ieta-1] : 0,
         m_eTowersIDs[iphi+1][ieta],
         ieta < 5 ? m_eTowersIDs[iphi+1][ieta+1] : 0},
      };


      ATH_CHECK( m_eFEXegAlgoTool->safetyTest() );
      m_eFEXegAlgoTool->setup(tobtable, m_efexid, m_id, ieta);

      // ignore any tobs without a seed, move on to the next window
      if (m_eFEXegAlgoTool->hasSeed() == false) continue;
      unsigned int seed = m_eFEXegAlgoTool->getSeed();
      unsigned int und = (m_eFEXegAlgoTool->getUnD() ? 1 : 0);

      // the minimum energy to send to topo (not eta dependent yet, but keep inside loop as it will be eventually?)
      unsigned int ptMinToTopoCounts = thr_eEM.ptMinToTopoCounts();

      //returns a unsigned integer et value corresponding to the... eFEX EM cluster in 25 MeV internal calculation scale
      unsigned int eEMTobEt = m_eFEXegAlgoTool->getET();
            
      // thresholds from Trigger menu
      // the menu eta runs from -25 to 24
      int menuEta = m_id*4 + (m_efexid%3)*16 + ieta - 25;
      auto iso_loose  = thr_eEM.isolation(TrigConf::Selection::WP::LOOSE, menuEta);
      auto iso_medium = thr_eEM.isolation(TrigConf::Selection::WP::MEDIUM, menuEta);
      auto iso_tight  = thr_eEM.isolation(TrigConf::Selection::WP::TIGHT, menuEta);  

      std::vector<unsigned int> threshReta;
      threshReta.push_back(iso_loose.reta_fw());
      threshReta.push_back(iso_medium.reta_fw());
      threshReta.push_back(iso_tight.reta_fw());

      std::vector<unsigned int> threshRhad;
      threshRhad.push_back(iso_loose.rhad_fw());
      threshRhad.push_back(iso_medium.rhad_fw());
      threshRhad.push_back(iso_tight.rhad_fw());

      std::vector<unsigned int> threshWstot;
      threshWstot.push_back(iso_loose.wstot_fw());
      threshWstot.push_back(iso_medium.wstot_fw());
      threshWstot.push_back(iso_tight.wstot_fw());

      ATH_MSG_DEBUG("ieta=" << ieta << "  loose => reta_fw=" << threshReta[0] << ", rhad_fw=" << threshRhad[0] << ", wstot_fw=" << threshWstot[0]);
      ATH_MSG_DEBUG("ieta=" << ieta << "  medium => reta_fw=" << threshReta[1] << ", rhad_fw=" << threshRhad[1] << ", wstot_fw=" << threshWstot[1]);
      ATH_MSG_DEBUG("ieta=" << ieta << "  tight => reta_fw=" << threshReta[2] << ", rhad_fw=" << threshRhad[2] << ", wstot_fw=" << threshWstot[2]);

      // Get Reta and Rhad outputs
      std::vector<unsigned int> RetaCoreEnv; 
      m_eFEXegAlgoTool->getReta(RetaCoreEnv);
      std::vector<unsigned int> RhadEMHad; 
      m_eFEXegAlgoTool->getRhad(RhadEMHad);
      std::vector<unsigned int> WstotDenNum;
      m_eFEXegAlgoTool->getWstot(WstotDenNum);

      // Set Reta, Rhad and Wstot WP
      unsigned int RetaWP = 0;
      unsigned int RhadWP = 0;
      unsigned int WstotWP = 0;
      
      // bitshifts for the different iso vars
      unsigned int RetaBitS = 3;
      unsigned int RhadBitS = 3;
      unsigned int WstotBitS = 5;

      unsigned int maxEtCountsEm = thr_eEM.maxEtCounts(m_eFexStep);
      if (eEMTobEt >= maxEtCountsEm){
	       RetaWP = 3;
	       RhadWP = 3;
	       WstotWP = 3;
      }
      else{
	       SetIsoWP(RetaCoreEnv,threshReta,RetaWP,RetaBitS);
	       SetIsoWP(RhadEMHad,threshRhad,RhadWP,RhadBitS);
	       SetIsoWP(WstotDenNum,threshWstot,WstotWP,WstotBitS);
      }
      int eta_ind = ieta; // No need to offset eta index with new 0-5 convention
      int phi_ind = iphi - 1;

      //form the egamma tob word and xTOB words
      uint32_t tobword = m_eFEXFormTOBsTool->formEmTOBWord(m_id,eta_ind,phi_ind,RhadWP,WstotWP,RetaWP,seed,und,eEMTobEt,ptMinToTopoCounts, emAlgoVersion);
      std::vector<uint32_t> xtobwords = m_eFEXFormTOBsTool->formEmxTOBWords(m_efexid,m_id,eta_ind,phi_ind,RhadWP,WstotWP,RetaWP,seed,und,eEMTobEt,ptMinToTopoCounts, emAlgoVersion);

      std::unique_ptr<eFEXegTOB> tmp_tob = m_eFEXegAlgoTool->geteFEXegTOB();
      
      tmp_tob->setFPGAID(m_id);
      tmp_tob->seteFEXID(m_efexid);
      tmp_tob->setEta(ieta);
      tmp_tob->setPhi(iphi);
      tmp_tob->setTobword(tobword);
      tmp_tob->setxTobword0(xtobwords[0]);
      tmp_tob->setxTobword1(xtobwords[1]);

      // for plotting
      if (inputOutputCollection->getdooutput() && (tobword != 0) && (eEMTobEt != 0)) {
        inputOutputCollection->addeFexNumber(m_efexid);
        inputOutputCollection->addEMtob(tobword);
        inputOutputCollection->addValue_eg("WstotNum", tmp_tob->getWstotNum());
        inputOutputCollection->addValue_eg("WstotDen", tmp_tob->getWstotDen());
        inputOutputCollection->addValue_eg("RetaNum", tmp_tob->getRetaCore());
        inputOutputCollection->addValue_eg("RetaDen", tmp_tob->getRetaEnv());
        inputOutputCollection->addValue_eg("RhadNum", tmp_tob->getRhadEM());
        inputOutputCollection->addValue_eg("RhadDen", tmp_tob->getRhadHad());
        inputOutputCollection->addValue_eg("haveSeed", m_eFEXegAlgoTool->hasSeed());
        inputOutputCollection->addValue_eg("ET", m_eFEXegAlgoTool->getET());
        float eta = 9999;
        m_eFEXegAlgoTool->getRealEta(eta);
        inputOutputCollection->addValue_eg("eta", eta);
        float phi = 9999;
        m_eFEXegAlgoTool->getRealPhi(phi);
        inputOutputCollection->addValue_eg("phi", phi);
        unsigned int em_et = 9999; 
        m_eFEXegAlgoTool->getCoreEMTowerET(em_et);
        inputOutputCollection->addValue_eg("em", em_et);
        unsigned int had_et = 9999;
        m_eFEXegAlgoTool->getCoreHADTowerET(had_et);
        inputOutputCollection->addValue_eg("had", had_et);
        inputOutputCollection->fill_eg();
      }

      // Now we've finished with that object we can move it into the class results store
      if ( (tobword != 0) && (eEMTobEt != 0) ) m_emTobObjects.push_back(std::move(tmp_tob));

    }
  }

  // --------------- TAU -------------
  for(int ieta = min_eta; ieta < overflow_eta; ieta++)
  {
    for(int iphi = 1; iphi < 9; iphi++)
    {
      int tobtable[3][3]={
        {ieta > 0 ? m_eTowersIDs[iphi-1][ieta-1] : 0,
         m_eTowersIDs[iphi-1][ieta],
         ieta < 5 ? m_eTowersIDs[iphi-1][ieta+1] : 0},

        {ieta > 0 ? m_eTowersIDs[iphi][ieta-1] : 0,
         m_eTowersIDs[iphi][ieta],
         ieta < 5 ? m_eTowersIDs[iphi][ieta+1] : 0},

        {ieta > 0 ? m_eTowersIDs[iphi+1][ieta-1] : 0,
         m_eTowersIDs[iphi+1][ieta],
         ieta < 5 ? m_eTowersIDs[iphi+1][ieta+1] : 0},
      };
      
      ATH_CHECK( m_eFEXtauAlgoTool->safetyTest() );
      ATH_CHECK( m_eFEXtauBDTAlgoTool->safetyTest() );
      m_eFEXtauAlgoTool->setup(tobtable, m_efexid, m_id, ieta);
      m_eFEXtauAlgoTool->setAlgoVersion(tauAlgoVersion);
      m_eFEXtauBDTAlgoTool->setup(tobtable, m_efexid, m_id, ieta);

      if ( m_eFEXtauAlgoTool->isCentralTowerSeed() != m_eFEXtauBDTAlgoTool->isCentralTowerSeed() )
      {
      	ATH_MSG_FATAL("BDT tau algo and heuristic tau algo should agree on seeding for all TOBs");
        return StatusCode::FAILURE;
      }

      if (!m_eFEXtauAlgoTool->isCentralTowerSeed()){ continue; }

      // the minimum energy to send to topo (not eta dependent yet, but keep inside loop as it will be eventually?)
      unsigned int ptTauMinToTopoCounts = thr_eTAU.ptMinToTopoCounts();

      unsigned int ptTauMinToTopoInEfexCounts = TrigConf::energyInCounts(thr_eTAU.ptMinToTopoMeV(), m_eFexStep);

      // Get Et of eFEX tau object in internal units (25 MeV)
      unsigned int eTauTobEt = m_eFEXtauAlgoTool->getEt();
      unsigned int eTauBDTTobEt = m_eFEXtauBDTAlgoTool->getEt();

      // thresholds from Trigger menu
      // the menu eta runs from -25 to 24
      int menuEta = m_id*4 + (m_efexid%3)*16 + ieta - 25;
      auto iso_loose  = thr_eTAU.isolation(TrigConf::Selection::WP::LOOSE, menuEta);
      auto iso_medium = thr_eTAU.isolation(TrigConf::Selection::WP::MEDIUM, menuEta);
      auto iso_tight  = thr_eTAU.isolation(TrigConf::Selection::WP::TIGHT, menuEta);  

      std::vector<unsigned int> threshRCore;
      threshRCore.push_back(iso_loose.rCore_fw());
      threshRCore.push_back(iso_medium.rCore_fw());
      threshRCore.push_back(iso_tight.rCore_fw());

      std::vector<unsigned int> threshRHad;
      threshRHad.push_back(iso_loose.rHad_fw());
      threshRHad.push_back(iso_medium.rHad_fw());
      threshRHad.push_back(iso_tight.rHad_fw());

      // Get isolation values
      std::vector<unsigned int> rCoreVec; 
      m_eFEXtauAlgoTool->getRCore(rCoreVec);

      std::vector<unsigned int> rHadVec;
      m_eFEXtauAlgoTool->getRHad(rHadVec);

      // BDT-based tau algorithm outputs (both 0 for heuristic algorithm)
      unsigned int bdtScore = 0;
      unsigned int bdtCondition = 0;
      unsigned int bdtRHadWP = 0;

      // Set isolation WP
      unsigned int rCoreWP = 0;
      unsigned int rHadWP = 0;

      // Isolation bitshift value
      unsigned int RcoreBitS = 3;
      unsigned int RhadBitS = 3;

      unsigned int maxEtCountsTau = thr_eTAU.maxEtCounts(m_eFexStep);
      unsigned int bdtMinEtCounts = thr_eTAU.minIsoEtCounts(m_eFexStep);
      if (eTauTobEt >= maxEtCountsTau) {
        rCoreWP = 3;
        rHadWP = 3;
      } else {
        SetIsoWP(rCoreVec,threshRCore,rCoreWP,RcoreBitS);
        SetIsoWP(rHadVec,threshRHad,rHadWP,RhadBitS);
      }

      // Only one tau algorithm may be active and this is controlled by the L1 menu.
      // When the BDT algorithm is active, the rCore field contains the BDT L/M/T thresholds.
      std::vector<unsigned int> threshBDT;
      threshBDT.push_back(iso_loose.rCore_fw());
      threshBDT.push_back(iso_medium.rCore_fw());
      threshBDT.push_back(iso_tight.rCore_fw());
      // in tau algoVersion 2, the autopass threshold for rhad frac (last parameter) is hardcoded
      m_eFEXtauBDTAlgoTool->setThresholds(threshRHad, threshBDT, ptTauMinToTopoInEfexCounts, maxEtCountsTau, bdtMinEtCounts, (tauAlgoVersion==2) ? 0x0ff0 : maxEtCountsTau);
      // Re-compute after setting thresholds. 
      // Threshold bits in the BDT algorithm's implementation are computed inside the algorithm class
      m_eFEXtauBDTAlgoTool->compute();
      eTauBDTTobEt = m_eFEXtauBDTAlgoTool->getEt();
      bdtScore = m_eFEXtauBDTAlgoTool->getBDTScore();
      bdtCondition = m_eFEXtauBDTAlgoTool->getBDTCondition();
      bdtRHadWP = m_eFEXtauBDTAlgoTool->getBDTHadFracCondition();

      unsigned int seed = m_eFEXtauAlgoTool->getSeed();
      // Heuristic seed as returned is supercell value within 3x3 area, here want it within central cell
      seed = seed - 4;      

      unsigned int und = (m_eFEXtauAlgoTool->getUnD() ? 1 : 0);

      unsigned int bdtSeed = m_eFEXtauBDTAlgoTool->getSeed();

      int eta_ind = ieta; // No need to offset eta index with new 0-5 convention
      int phi_ind = iphi - 1;

      // Form the tau TOB word and xTOB words
      std::vector<uint32_t> xtobwords;
      std::vector<uint32_t> xtobwordsBDT;

      ATH_MSG_DEBUG("m_id: " << m_id << ", eta_ind: " << eta_ind << ", phi_ind: "
		      << phi_ind << ", eTauBDTTobEt: " << eTauBDTTobEt
		      << ", eTauTobEt: " << eTauTobEt << ", ptTauMinToTopoCounts: "
		      << ptTauMinToTopoCounts << ", maxEtCountsTau: " << maxEtCountsTau
		      << ", bdtScore: " << bdtScore << " bdtMinEtCounts: " << bdtMinEtCounts << " bdtRHadWP " << bdtRHadWP);

      uint32_t tobwordBDT = m_eFEXFormTOBsTool->formTauTOBWord(m_id, eta_ind, phi_ind, eTauBDTTobEt, bdtRHadWP, bdtCondition, bdtSeed,0, ptTauMinToTopoCounts, tauAlgoVersion);
      xtobwordsBDT = m_eFEXFormTOBsTool->formTauxTOBWords(m_efexid, m_id, eta_ind, phi_ind, eTauBDTTobEt, bdtRHadWP, bdtCondition, bdtSeed, 0, ptTauMinToTopoCounts, tauAlgoVersion, bdtScore);
      uint32_t tobword = m_eFEXFormTOBsTool->formTauTOBWord(m_id, eta_ind, phi_ind, eTauTobEt, rHadWP, rCoreWP, seed, und, ptTauMinToTopoCounts, 0);
      xtobwords = m_eFEXFormTOBsTool->formTauxTOBWords(m_efexid, m_id, eta_ind, phi_ind, eTauTobEt, rHadWP, rCoreWP, seed, und, ptTauMinToTopoCounts, 0, 0);

      std::unique_ptr<eFEXtauTOB> tmp_tau_tob = m_eFEXtauAlgoTool->getTauTOB();
      tmp_tau_tob->setFPGAID(m_id);
      tmp_tau_tob->seteFEXID(m_efexid);
      tmp_tau_tob->setEta(ieta);
      tmp_tau_tob->setPhi(iphi);
      tmp_tau_tob->setTobword(tobword);
      tmp_tau_tob->setxTobword0(xtobwords[0]);
      tmp_tau_tob->setxTobword1(xtobwords[1]);

      std::unique_ptr<eFEXtauTOB> tmp_tau_tob_bdt = m_eFEXtauBDTAlgoTool->getTauTOB();
      tmp_tau_tob_bdt->setFPGAID(m_id);
      tmp_tau_tob_bdt->seteFEXID(m_efexid);
      tmp_tau_tob_bdt->setEta(ieta);
      tmp_tau_tob_bdt->setPhi(iphi);
      tmp_tau_tob_bdt->setTobword(tobwordBDT);
      tmp_tau_tob_bdt->setxTobword0(xtobwordsBDT[0]);
      tmp_tau_tob_bdt->setxTobword1(xtobwordsBDT[1]);

      // for plotting
      if ((inputOutputCollection->getdooutput()) && ( tobword != 0 )) {
        inputOutputCollection->addValue_tau("isCentralTowerSeed", m_eFEXtauAlgoTool->isCentralTowerSeed());
        inputOutputCollection->addValue_tau("Et", m_eFEXtauAlgoTool->getEt());
        inputOutputCollection->addValue_tau("EtBDT", m_eFEXtauBDTAlgoTool->getEt());
        inputOutputCollection->addValue_tau("Eta", ieta);
        inputOutputCollection->addValue_tau("Phi", iphi);
        const LVL1::eTower * centerTower = eTowerContainer->findTower(m_eTowersIDs[iphi][ieta]);
        const LVL1::eTower * oneOffEtaTower = ieta < 5 ? eTowerContainer->findTower(m_eTowersIDs[iphi][ieta+1]) : nullptr;
        const LVL1::eTower * oneBelowEtaTower = ieta > 0 ? eTowerContainer->findTower(m_eTowersIDs[iphi][ieta-1]) : nullptr;
        inputOutputCollection->addValue_tau("CenterTowerEt", centerTower->getTotalET());
        inputOutputCollection->addValue_tau("OneOffEtaTowerEt", oneOffEtaTower ? oneOffEtaTower->getTotalET() : 0);
        inputOutputCollection->addValue_tau("OneBelowEtaTowerEt", oneBelowEtaTower ? oneBelowEtaTower->getTotalET() : 0);
        inputOutputCollection->addValue_tau("FloatEta", centerTower->eta() * centerTower->getPosNeg());
        inputOutputCollection->addValue_tau("FloatPhi", centerTower->phi());
        inputOutputCollection->addValue_tau("RCoreCore", rCoreVec[0]);
        inputOutputCollection->addValue_tau("RCoreEnv", rCoreVec[1]);
        inputOutputCollection->addValue_tau("RealRCore", m_eFEXtauAlgoTool->getRealRCore());
        inputOutputCollection->addValue_tau("RCoreWP", rCoreWP);
        inputOutputCollection->addValue_tau("RHadCore", rHadVec[0]);
        inputOutputCollection->addValue_tau("RHadEnv", rHadVec[1]);
        inputOutputCollection->addValue_tau("RealRHad", m_eFEXtauAlgoTool->getRealRHad());
        inputOutputCollection->addValue_tau("RealRHadBDT", m_eFEXtauBDTAlgoTool->getRealRHad());
        inputOutputCollection->addValue_tau("RHadWP", rHadWP);
        inputOutputCollection->addValue_tau("Seed", seed);
        inputOutputCollection->addValue_tau("UnD", und);
        inputOutputCollection->addValue_tau("BDTScore", bdtScore);
        inputOutputCollection->addValue_tau("BDTCondition", bdtCondition);
        inputOutputCollection->addValue_tau("eFEXID", m_efexid);
        inputOutputCollection->addValue_tau("FPGAID", m_id);

        
        inputOutputCollection->fill_tau();
      }
      // Now we've finished with that object we can move it into the class results store
      if ( tobword != 0 ) m_tauHeuristicTobObjects.push_back(std::move(tmp_tau_tob));
      if ( tobwordBDT != 0 ) m_tauBDTTobObjects.push_back(std::move(tmp_tau_tob_bdt));

    }
  }

  return StatusCode::SUCCESS;
}



std::vector<std::unique_ptr<eFEXegTOB>> eFEXFPGA::getEmTOBs()
{
  // TOB sorting moved to eFEXSysSim to simplify xTOB production
  // But leave this here in case more subtle requirement is uncovered in future
  /*
  auto tobsSort = m_emTobObjects;

  ATH_MSG_DEBUG("number of tobs: " <<tobsSort.size() << " in FPGA: " << m_id << " before truncation");

  // sort tobs by their et (last 12 bits of the 32 bit tob word)
  std::sort (tobsSort.begin(), tobsSort.end(), TOBetSort<eFEXegTOB>);

  // return the top 6 highest ET TOBs from the FPGA
  if (tobsSort.size() > 6) tobsSort.resize(6);
  return tobsSort;
  */

  /* Returning a vector of unique_pointers means this class will lose ownership.
     This shouldn't be an issue since all this class does is create and return the 
     objects, but you should bear it in mind if you make changes */

  // This copy seems to be needed - it won't let me pass m_emTobOjects directly (to do with being a class member?)
  std::vector<std::unique_ptr<eFEXegTOB>> tobsSort;
  for(auto &j : m_emTobObjects){
      tobsSort.push_back(std::move(j));
  }

  return tobsSort;
}

std::vector<std::unique_ptr<eFEXtauTOB>> eFEXFPGA::getTauTOBs(std::vector< std::unique_ptr<eFEXtauTOB> >& tauTobObjects)
{
  // TOB sorting moved to eFEXSysSim to simplify xTOB production
  // But leave this here in case more subtle requirement is uncovered in future
  /*
  auto tobsSort = tauTobObjects;

  ATH_MSG_DEBUG("number of tobs: " <<tobsSort.size() << " in FPGA: " << m_id << " before truncation");

  // sort tobs by their et (last 12 bits of the 32 bit tob word)
  std::sort (tobsSort.begin(), tobsSort.end(), TOBetSort<eFEXtauTOB>);

  // return the top 6 highest ET TOBs from the FPGA
  if (tobsSort.size() > 6) tobsSort.resize(6);
  return tobsSort;
  */

  /* Returning a vector of unique_pointers means this class will lose ownership.
     This shouldn't be an issue since all this class does is create and return the 
     objects, but you should bear it in mind if you make changes */

  // This copy seems to be needed - it won't let me pass m_tauTobOjects directly (to do with being a class member?)
  std::vector<std::unique_ptr<eFEXtauTOB>> tobsSort;
  for(auto &j : tauTobObjects){
      tobsSort.push_back(std::move(j));
  }

  return tobsSort;
}

std::vector<std::unique_ptr<eFEXtauTOB>> eFEXFPGA::getTauHeuristicTOBs()
{
  return getTauTOBs(m_tauHeuristicTobObjects);
}

std::vector<std::unique_ptr<eFEXtauTOB>> eFEXFPGA::getTauBDTTOBs()
{
  return getTauTOBs(m_tauBDTTobObjects);
}

void eFEXFPGA::SetTowersAndCells_SG(int tmp_eTowersIDs_subset[][6]){
    
  int rows = 10;
  int cols = sizeof tmp_eTowersIDs_subset[0] / sizeof tmp_eTowersIDs_subset[0][0];
  
  std::copy(&tmp_eTowersIDs_subset[0][0], &tmp_eTowersIDs_subset[0][0]+(10*6),&m_eTowersIDs[0][0]);
  
  if(false){ //this prints out the eTower IDs that each FPGA is responsible for
    ATH_MSG_DEBUG("\n---- eFEXFPGA --------- eFEX (" << m_efexid << " ----- FPGA (" << m_id << ") IS RESPONSIBLE FOR eTOWERS :");
    for (int thisRow=rows-1; thisRow>=0; thisRow--){
      for (int thisCol=0; thisCol<cols; thisCol++){
	if(thisCol != cols-1){ ATH_MSG_DEBUG("|  " << m_eTowersIDs[thisRow][thisCol] << "  "); }
	else { ATH_MSG_DEBUG("|  " << m_eTowersIDs[thisRow][thisCol] << "  |"); }
      }
    }
  }


  //-----------------------------------------------------------
  // Set up a the second CSV file if necessary (should only need to be done if the mapping changes, which should never happen unless major changes to the simulation are required)
  if(false){ // CSV CODE TO BE RE-INTRODUCED VERY SOON
    SG::ReadHandle<eTowerContainer> eTowerContainer(m_eTowerContainerKey);
    if(!eTowerContainer.isValid()){
      ATH_MSG_FATAL("Could not retrieve container " << m_eTowerContainerKey.key() );
    }
    
    std::ofstream tower_fpga_efex_map;
    tower_fpga_efex_map.open ("./tower_fpga_efex_map.csv", std::ios_base::app);
    
    for (int thisRow=rows-1; thisRow>=0; thisRow--){
      for (int thisCol=0; thisCol<cols; thisCol++){
	
	const LVL1::eTower * tmpTower = eTowerContainer->findTower(m_eTowersIDs[thisRow][thisCol]);
	
	tower_fpga_efex_map << m_efexid << "," << m_id << "," << m_eTowersIDs[thisRow][thisCol] << "," << tmpTower->eta() << "," << tmpTower->phi() << "\n";
	
      }
    }
  }
  //------------------------------------------------------------

  
}


void eFEXFPGA::SetIsoWP(const std::vector<unsigned int>& CoreEnv, const std::vector<unsigned int>& thresholds, unsigned int & workingPoint, unsigned int bitshift) const {
  // Working point evaluted by Core * 2^bitshift > Threshold * Environment conditions
  std::unordered_map<unsigned int, unsigned int> bsmap { {3, 8}, {5, 32}};

  // if core (denom) has overflowed, automatically pass all thresholds
  if (CoreEnv[0] > 0xffff) {
      workingPoint = 3;
      return;
  }

  unsigned int large = CoreEnv[0]*bsmap[bitshift]; // core
  unsigned int small = CoreEnv[1]; // env
  
  // Word length conditions
  if (large > 0xffff ) large = 0xffff;
  if (small > 0xffff ) small = 0xffff;

  // Fail all if env sum overflows
  if (small == 0xffff) {
    workingPoint = 0;
    return;
  }

  // Otherwise test thresholds in firmware order 
  // i.e. check lowest threshold failed rather than highest passed
  if ( large < small*thresholds[0] ) {
    workingPoint = 0;
    return;
  }
  if ( large < small*thresholds[1] ) {
    workingPoint = 1;
    return;
  }
  if ( large < small*thresholds[2] ) {
    workingPoint = 2;
    return;
  }
  workingPoint = 3;
  return;
}

} // end of namespace bracket
