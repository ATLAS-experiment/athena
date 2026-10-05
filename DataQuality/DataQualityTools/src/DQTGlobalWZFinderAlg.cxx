/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// ********************************************************************
//
// NAME:     DQTGlobalWZFinderAlg.cxx
// PACKAGE:  DataQualityTools  
// 
// AUTHORS:   Jahred Adelman (jahred.adelman@cern.ch)
//            Simon Viel (svielcern.ch)
//            Sam King (samking@physics.ubc.ca)
//            Koos van Nieuwkoop (jvannieu@cern.ch)
//	      Samuel Alibocus (salibocu@cern.ch)
//
// ********************************************************************

#include "DataQualityTools/DQTGlobalWZFinderAlg.h"
#include "AthenaBaseComps/AthAlgTool.h" 

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ITHistSvc.h"

#include "xAODTracking/Vertex.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODEgamma/EgammaTruthxAODHelpers.h"

#include "TruthUtils/TruthClasses.h"

#include <vector>

using Gaudi::Units::GeV;
using Gaudi::Units::mm;


//----------------------------------------------------------------------------------
DQTGlobalWZFinderAlg::DQTGlobalWZFinderAlg(const std::string & name,
		   ISvcLocator* pSvcLocator )
	: AthMonitorAlgorithm(name, pSvcLocator)
//----------------------------------------------------------------------------------
{
}

StatusCode DQTGlobalWZFinderAlg::initialize() {
  ATH_CHECK(AthMonitorAlgorithm::initialize());
  ATH_CHECK(m_muonSelectionTool.retrieve());
  ATH_CHECK(m_r3MatchingTool.retrieve());
  if (dataType() == DataType_t::monteCarlo) {
    ATH_CHECK(m_truthClassifier.retrieve());
  } else {
    m_truthClassifier.disable();
  }

  ATH_CHECK(m_ElectronContainerKey.initialize());
  ATH_CHECK(m_MuonContainerKey.initialize());
  ATH_CHECK(m_PhotonContainerKey.initialize());
  ATH_CHECK(m_VertexContainerKey.initialize());
  ATH_CHECK(m_TruthParticleContainerKey.initialize(dataType() == DataType_t::monteCarlo));
  ATH_CHECK(m_idTrackParticleContainerKey.initialize());
  ATH_CHECK(m_msTrackParticleContainerKey.initialize());
  ATH_CHECK(m_isoMuonContainerKey.initialize());
  ATH_CHECK(m_isoElectronContainerKey.initialize());
  return StatusCode::SUCCESS;
}

//----------------------------------------------------------------------------------
StatusCode DQTGlobalWZFinderAlg::fillHistograms( const EventContext& ctx ) const
//----------------------------------------------------------------------------------
{
  ATH_MSG_DEBUG("in DQTGlobalWZFinderAlg::fillHistograms()");

  using namespace Monitored;

  if (m_doRunBeam) {  
    
     auto group = getGroup("default");

     //Get LumiBlock and EventNumber
     SG::ReadHandle<xAOD::EventInfo> thisEventInfo { GetEventInfo(ctx) };
     if(! thisEventInfo.isValid()) {
       ATH_MSG_ERROR("Could not find EventInfo in evtStore()");
       return StatusCode::FAILURE;
     }

     bool isSimulation = thisEventInfo->eventType(xAOD::EventInfo::IS_SIMULATION);
     auto writeTTrees = Scalar("writeTTrees", isSimulation); 
 
     auto LB = Scalar<int>("LB", thisEventInfo->lumiBlock());
     auto eventNumber = Scalar<int>("eventNumber", thisEventInfo->eventNumber());
     auto runNumber = Scalar<int>("runNumber", thisEventInfo->runNumber());

     auto avgLiveFrac = Scalar("avgLiveFrac", lbAverageLivefraction(ctx));
     auto duration = Scalar("duration", lbDuration(ctx));
     auto avgIntPerXing = Scalar("avgIntPerXing", lbAverageInteractionsPerCrossing(ctx));
     
     fill(group, LB, avgLiveFrac, duration, avgIntPerXing);
     
     ATH_MSG_DEBUG("Filled LB hists");
     
      
     auto evtWeight = Scalar("evtWeight", 1.0);
     if (thisEventInfo->eventType(xAOD::EventInfo::IS_SIMULATION)) {
       evtWeight = thisEventInfo->mcEventWeight();
       ATH_MSG_DEBUG("Event Weight: " << evtWeight);
     }
     
     //Get Electrons
     SG::ReadHandle<xAOD::ElectronContainer> elecTES(m_ElectronContainerKey, ctx);
     if ( ! elecTES.isValid() ) {
       ATH_MSG_ERROR("No electron container" <<  m_ElectronContainerKey << " found in evtStore");
       return StatusCode::FAILURE;
     }
     
     ATH_MSG_DEBUG("ElectronContainer successfully retrieved");
     

     //Get Muons
        
     SG::ReadHandle<xAOD::MuonContainer> muons(m_MuonContainerKey, ctx);
     if (! muons.isValid() ) {
       ATH_MSG_ERROR("evtStore() does not contain muon Collection with name "<< m_MuonContainerKey);
       return StatusCode::FAILURE;
     }

     ATH_MSG_DEBUG("Got muon collection!");
     
     std::vector<const xAOD::Electron*> goodelectrons;
     std::vector<const xAOD::Muon*> goodmuonsZ;
     std::vector<const xAOD::Muon*> goodmuonsTP;
     
     //get primary vertex info
     const xAOD::Vertex* pVtx(0);
     SG::ReadHandle<xAOD::VertexContainer> vertices(m_VertexContainerKey, ctx);
     if (vertices.isValid()) {
       ATH_MSG_DEBUG("Collection with name " << m_VertexContainerKey << " with size " << vertices->size() << " found in evtStore()");
       for(const auto vtx : * vertices) {
         if(vtx->vertexType()==xAOD::VxType::PriVtx) {
           pVtx = vtx;
           break;
         }
       }
     } else {
       ATH_MSG_WARNING("No collection with name " << m_VertexContainerKey << " found in evtStore()");
     }
     
     const xAOD::Electron* leadingAllEle(0);
     const xAOD::Electron* subleadingAllEle(0);
     std::vector<const xAOD::Electron*> allElectrons;
     
     
     // Electron Cut Flow
     ATH_MSG_DEBUG("Start electron selection");
  
     auto elegroup = getGroup("electron");
     
     for(const auto electron : *elecTES) {
       allElectrons.push_back(electron);

       if(goodElectrons(electron, pVtx, ctx)){
         ATH_MSG_DEBUG("Good electron");

	       auto ele_Et = Scalar("ele_Et", electron->pt()/GeV);
         auto ele_Eta = Scalar("ele_Eta", electron->eta());
	       auto ele_Phi = Scalar("ele_Phi", electron->phi());
	       fill(elegroup, ele_Et, ele_Eta, ele_Phi, evtWeight);	  
	       goodelectrons.push_back(electron);
       }
     }     
     
     // Muon Cut Flow
     
     auto muongroup = getGroup("muon");
     ATH_MSG_DEBUG("Start muon selection");
     static const SG::Accessor<float> aptc20("ptcone20");

     for (const xAOD::Muon* muon : *muons){
       auto muTrk = (muon)->trackParticle(xAOD::Muon::TrackParticleType::Primary);
       float d0sig;
       if (!muTrk) {
         ATH_MSG_WARNING("No muon track! " << thisEventInfo->runNumber() << " " << thisEventInfo->eventNumber());
	       continue;
       }
       try {
	      d0sig = xAOD::TrackingHelpers::d0significance(muTrk, thisEventInfo->beamPosSigmaX(), thisEventInfo->beamPosSigmaY(), thisEventInfo->beamPosSigmaXY());
       } catch (...) {
         ATH_MSG_DEBUG("Invalid beamspot - muon");
	       try {
	         d0sig = xAOD::TrackingHelpers::d0significance(muTrk);
         } catch (...) {
           ATH_MSG_WARNING("Ridiculous exception thrown - muon");
	         continue;
	       }
       }

       float ptcone20 = 0;
       if (! aptc20.isAvailable(*muon)) {
        ATH_MSG_WARNING("aptc20 is not available -  muon");
       } else {
        ptcone20 = aptc20(*muon);
       }

       float muonIso = 0.0;
       if ((muon)->pt() != 0.0){
         muonIso = ptcone20/((muon)->pt());
       }

       ATH_MSG_DEBUG("Muon accept: " << static_cast<bool>(m_muonSelectionTool->accept(*muon)));
       ATH_MSG_DEBUG("Muon pt: " << (muon)->pt() << " " << m_muonPtCut*GeV);
       ATH_MSG_DEBUG("Muon iso: " << static_cast<bool>(muonIso < 0.1 ));
       ATH_MSG_DEBUG("Muon d0sig: " << d0sig);
       ATH_MSG_DEBUG("Muon Good vtx: " << pVtx);
       if (pVtx) ATH_MSG_DEBUG("Muon z0sinth: " << std::abs((muTrk->z0()+muTrk->vz()-pVtx->z())*std::sin(muTrk->theta())) << " " << 0.5*mm);
       
       if (m_muonSelectionTool->accept(*muon) &&
           ((muon)->pt() > 0.8*m_muonPtCut*GeV) &&
           muonIso < 0.1 &&
           std::abs(d0sig) < 3 &&
           pVtx &&
           std::abs((muTrk->z0()+muTrk->vz()-pVtx->z())*std::sin(muTrk->theta())) < 0.5*mm)
       {

         goodmuonsTP.push_back(muon);
         if (((muon)->pt() > m_muonPtCut*GeV))
         {
           auto muon_Pt = Scalar("muon_Pt", (muon)->pt()/GeV);
	         auto muon_Eta = Scalar("muon_Eta", (muon)->eta());
	         auto muon_Phi = Scalar("muon_Phi", (muon)->phi());
	         fill(muongroup, muon_Pt, muon_Eta, muon_Phi, evtWeight);
	         goodmuonsZ.push_back(muon);
	       }
       }
       
     }
     
     if (isSimulation) {
       doMuonTruthEff(goodmuonsZ, ctx);
     }

     for (const auto iEle : allElectrons) {
       Float_t pt = iEle->pt();
       ATH_MSG_DEBUG("Ele pt " << pt);
       if (!leadingAllEle || pt > leadingAllEle->pt()){
         subleadingAllEle = leadingAllEle;
         leadingAllEle = iEle;
       }
       else if (!subleadingAllEle || pt > subleadingAllEle->pt()){
         subleadingAllEle = iEle;
       }
     }

     // Perform all Tag and Probe Procedures
     
     doMuonLooseTP(goodmuonsTP, pVtx, ctx, isSimulation, writeTTrees, evtWeight);	
     doMuonInDetTP(goodmuonsTP, pVtx, ctx, isSimulation, writeTTrees, evtWeight);
     doEleTP(leadingAllEle, subleadingAllEle, pVtx, ctx, writeTTrees, isSimulation, evtWeight);
     doEleContainerTP(allElectrons, goodelectrons, ctx);

     // Sort Candidates by Pt
     const xAOD::Electron* leadingEle(0);
     const xAOD::Electron*  subleadingEle(0);
     const xAOD::Muon* leadingMuZ(0);
     const xAOD::Muon* subleadingMuZ(0);

     ATH_MSG_DEBUG("Beginning ele loop");
     for (const auto iEle : goodelectrons) {
        Float_t pt = iEle->pt();
	      ATH_MSG_DEBUG("Ele pt " << pt);
        if (! leadingEle || pt > leadingEle->pt()) {
           subleadingEle = leadingEle;
           leadingEle = iEle;
        }
        else if (! subleadingEle || pt > subleadingEle->pt()) {
           subleadingEle = iEle;
        }
     }
     ATH_MSG_DEBUG("Done ele loop");

     ATH_MSG_DEBUG("Start mu Z loop");
     for (const auto iMu : goodmuonsZ) {
        Float_t pt = iMu->pt();
        if (! leadingMuZ || pt > leadingMuZ->pt()) {
           subleadingMuZ = leadingMuZ;
           leadingMuZ = iMu;
        }
        else if (! subleadingMuZ || pt > subleadingMuZ->pt()) {
           subleadingMuZ = iMu;
        }
     }
     ATH_MSG_DEBUG("Done mu Z loop");

     // Z Mass
     bool isZee = (goodelectrons.size() > 1);
     bool isZmumu = (goodmuonsZ.size() > 1);
     ATH_MSG_DEBUG("Evaluated Event");
     auto ZeeGroup = getGroup("Zee");
     auto ZmumuGroup = getGroup("Zmumu");

     if(isZee){
       ATH_MSG_DEBUG("Zee found");
       TLorentzVector Zee = (leadingEle->p4() + subleadingEle->p4());
       auto mass = Scalar("mass", Zee.M());
       auto Zeecharge = Scalar("Zeecharge", leadingEle->charge() + subleadingEle->charge());
       bool passTrig = true;
       if (m_doTrigger){
	 passTrig = trigChainsArePassed(m_Z_ee_trigger);
       }
       bool inMassWindow = (mass > m_zCutLow*GeV && mass < m_zCutHigh*GeV);
       auto osel = Scalar<bool>("osel", false);
       auto ssel = Scalar<bool>("ssel", false);
       (Zeecharge == 0) ? (osel = true) : (ssel = true);
       if (inMassWindow){
         fill(ZeeGroup, Zeecharge, evtWeight);
         ATH_MSG_DEBUG("Found a Z to ee candidate! Mass = " << mass << ", and charge = " << Zeecharge );
         if(osel && passTrig){
           auto eta1 = Scalar("eta1", leadingEle->caloCluster()->etaBE(2));
	         auto eta2 = Scalar("eta2", subleadingEle->caloCluster()->etaBE(2));
	         auto phi1 = Scalar("phi1", leadingEle->phi());
           auto phi2 = Scalar("phi2", subleadingEle->phi());
           auto pT1 = Scalar("pT1", leadingEle->pt());
	         auto pT2 = Scalar("pT2", subleadingEle->pt());
           auto isTruth = Scalar("isTruth", false);  
	 
           if(writeTTrees){
             isTruth = checkTruthElectron(leadingEle) && checkTruthElectron(subleadingEle);
	         }
           fill(ZeeGroup, mass, eta1, eta2, phi1, phi2, pT1, pT2, evtWeight, LB, runNumber, eventNumber, isTruth, writeTTrees, osel);   
         }
         if(ssel && passTrig){
           if (!isSimulation){
             fill(ZeeGroup, mass, LB, evtWeight, ssel);
	         }
         }
         if(m_doTrigger){
           doEleTriggerTP(leadingEle, subleadingEle, ctx, writeTTrees, evtWeight, osel, ssel);
         }           
       }
     }
     if (isZmumu){
       ATH_MSG_DEBUG("Zmumu found");
       TLorentzVector Zmumu = leadingMuZ->p4() + subleadingMuZ->p4();
       auto mass = Scalar("mass", Zmumu.M());
       auto Zmumucharge = Scalar("Zmumucharge", leadingMuZ->charge() + subleadingMuZ->charge());
       // potentially ignore trigger...
       bool oktrig = true;
       if (m_doTrigger){
	 oktrig = trigChainsArePassed(m_Z_mm_trigger);
       }
       bool inMassWindow = (mass > m_zCutLow*GeV && mass < m_zCutHigh*GeV);
       auto osmu = Scalar<bool>("osmu", false);
       auto ssmu = Scalar<bool>("ssmu", false);
       (Zmumucharge == 0) ? (osmu = true) : (ssmu = true);
       if(inMassWindow){
         fill(ZmumuGroup, Zmumucharge, evtWeight); 
         ATH_MSG_DEBUG("Found a Z to mumu candidate! Mass = " << mass << ", and charge = " << Zmumucharge);
         if(osmu && oktrig){
           auto eta1 = Scalar("eta1", leadingMuZ->eta());
	         auto eta2 = Scalar("eta2", subleadingMuZ->eta());
	         auto phi1 = Scalar("phi1", leadingMuZ->phi());
	         auto phi2 = Scalar("phi2", subleadingMuZ->phi());
	         auto pT1 = Scalar("pT1", leadingMuZ->pt());
	         auto pT2 = Scalar("pT2", subleadingMuZ->pt());
           auto isTruth = Scalar("isTruth", false);
           if (writeTTrees){
	          isTruth = checkTruthMuon(leadingMuZ) && checkTruthMuon(subleadingMuZ); 
           }
           fill(ZmumuGroup, eta1, eta2, phi1, phi2, pT1, pT2, isTruth, evtWeight, LB, runNumber, eventNumber, mass, writeTTrees, osmu); 
         }
         if(osmu && !oktrig){
           ATH_MSG_DEBUG("Trigger failure!");
         }
         if(osmu && m_doTrigger){
           doMuonTriggerTP(leadingMuZ, subleadingMuZ, ctx, isSimulation, writeTTrees, evtWeight);
         }
         if(ssmu && oktrig){
           if (!isSimulation){
             fill(ZmumuGroup, mass, LB, evtWeight, ssmu);
           }
         }
       }
     }
  } 
   
  return StatusCode::SUCCESS;
}

void DQTGlobalWZFinderAlg::doEleTriggerTP(const xAOD::Electron* el1, const xAOD::Electron* el2, const EventContext& ctx, bool writeTTrees, const float evtWeight, bool osel, bool ssel) const{

  using namespace Monitored;

  SG::ReadHandle<xAOD::EventInfo> thisEventInfo { GetEventInfo(ctx) };

  auto group_EleTrigTP = getGroup("EleTrigTP");
  auto matched = Scalar("matched", 0);
  auto weight = Scalar("weight", evtWeight);
  auto os = Scalar<bool>("os", osel);
  auto ss = Scalar<bool>("ss", ssel);  

  std::vector<const xAOD::Electron*> electrons{el1, el2};

  for (const auto el: electrons) {
    for (const auto &chain: m_Z_ee_trigger) {
      if (m_r3MatchingTool->match(*el, chain, 0.1, false)) {
        matched++;
        break;
      }
    }
  }

  fill(group_EleTrigTP, matched, weight, os, ss);

  if (!writeTTrees){
    return;
  } 

  for (const auto& tagel : electrons) {
    bool matched_tag = false;
    for (const auto &chain: m_Z_ee_trigger) {
      if (m_r3MatchingTool->match(*tagel, chain, 0.1, false)) {
        matched_tag = true;
        break;
      }
    }
  

    auto tagelp4(tagel->p4());
    if (!matched_tag) continue;
    for (const auto& probeel : electrons) {
      if (tagel == probeel) {
        continue;
      }
      auto probeelp4(probeel->p4());
      auto mass = Scalar("mass", (tagelp4+probeelp4).M());
      bool matched_probe = false;
      if (mass < m_zCutLow*GeV || mass > m_zCutHigh*GeV) continue;

      auto pT = Scalar("pT", probeel->pt());
      auto phi = Scalar("phi", probeel->phi());
      auto eta = Scalar("eta", probeel->caloCluster()->etaBE(2));
      auto runNumber = Scalar("runNumber", thisEventInfo->runNumber());
      auto eventNumber = Scalar("eventNumber", thisEventInfo->eventNumber());
      auto LB = Scalar("LB", thisEventInfo->lumiBlock());
      auto mtype = Scalar("mtype", -1000);
 
      for (const auto &chain: m_Z_ee_trigger){
        if (m_r3MatchingTool->match(*probeel, chain, 0.1, false)) {
          matched_probe = true;
          break;
        }
      }

      if (matched_probe) {
        mtype = osel ? 0 : 1;
      }
      else if (!matched_probe) {
        mtype = osel ? 2 : 3;
      }

      if (writeTTrees){
        fill(group_EleTrigTP, pT, phi, eta, mass, runNumber, eventNumber, LB, mtype, weight);
      }
    }
  } 
}

void DQTGlobalWZFinderAlg::doEleTP(const xAOD::Electron* leadingAllEle, const xAOD::Electron* subleadingAllEle, const xAOD::Vertex* pVtx, const EventContext& ctx, bool writeTTrees, bool isSimulation, const float evtWeight) const{

  using namespace Monitored;

  SG::ReadHandle<xAOD::EventInfo> thisEventInfo { GetEventInfo(ctx) }; 
  
  auto group_EleTP = getGroup("EleTP");

  // first check we have both electrons
  if(leadingAllEle && subleadingAllEle){

    // Truth matching
    if (isSimulation) {
      if (!(checkTruthElectron(leadingAllEle) && checkTruthElectron(subleadingAllEle))) return;
    }

    // then get all the parameters we will need ready
    auto Zeecharge = Scalar("Zeecharge", (leadingAllEle->charge() + subleadingAllEle->charge()));
    auto p1(leadingAllEle->p4());
    auto p2(subleadingAllEle->p4());
    auto mass = Scalar("mass", (p1+p2).M());

    bool leadingPassKinematics = kinematicCuts(leadingAllEle);
    bool subleadPassKinematics = kinematicCuts(subleadingAllEle);

    if(!leadingPassKinematics || !subleadPassKinematics) return;

    bool leading_good	 = goodElectrons(leadingAllEle, pVtx, ctx);
    bool subleading_good = goodElectrons(subleadingAllEle, pVtx, ctx);

    bool leading_antigood    = antiGoodElectrons(leadingAllEle, pVtx, ctx);
    bool subleading_antigood = antiGoodElectrons(subleadingAllEle, pVtx, ctx);

    // do trigger matching
    bool leading_trig = false;
    for (const auto &chain: m_Z_ee_trigger) {
      if (m_r3MatchingTool->match(*leadingAllEle, chain, 0.1, false)){
        leading_trig = true;
        break;
      }
    }

    bool subleading_trig = false;
    for (const auto &chain: m_Z_ee_trigger) {
      if (m_r3MatchingTool->match(*subleadingAllEle, chain, 0.1, false)){
        subleading_trig = true;
        break;
      }
    }

    bool opp_sign = (Zeecharge==0);

    bool tag_good1 = (leadingAllEle->passSelection("LHTight") && leading_trig && leading_good);
    bool tag_good2 = (subleadingAllEle->passSelection("LHTight") && subleading_trig && subleading_good);

    fillEleEffHistos(tag_good1, subleading_good, subleading_antigood, opp_sign, mass);
    fillEleEffHistos(tag_good2, leading_good, leading_antigood, opp_sign, mass);

    if (!writeTTrees)
      return;
   
    auto pT = Scalar("pT", -1000.0);
    auto phi = Scalar("phi", -1000.0);
    auto eta = Scalar("eta", -1000.0);
    auto weight = Scalar("weight", -1000.0);
    auto runNumber = Scalar("runNumber", -1000);
    auto eventNumber = Scalar("eventNumber", -1000);
    auto LB = Scalar("LB", -1000);
    auto mtype = Scalar("mtype", -1000);
 
    // now fill the trees
    if(tag_good1){
      pT = subleadingAllEle->pt();
      phi = subleadingAllEle->phi();
      eta = subleadingAllEle->caloCluster()->etaBE(2);
      weight = evtWeight;
      runNumber = thisEventInfo->runNumber();
      eventNumber = thisEventInfo->eventNumber();
      LB = thisEventInfo->lumiBlock();

      if(opp_sign){
        mtype = subleading_good ? 0 : 2;
        if(subleading_antigood)
          mtype = 4;
      }else{
        mtype = subleading_good ? 1 : 3;
        if(subleading_antigood)
          mtype = 5;
      }
      
      fill(group_EleTP, pT, phi, eta, mass, runNumber, eventNumber, LB, mtype, weight);  
    }

    if(tag_good2){
      pT = leadingAllEle->pt();
      phi = leadingAllEle->phi();
      eta = leadingAllEle->caloCluster()->etaBE(2);
      weight = evtWeight;
      runNumber = thisEventInfo->runNumber();
      eventNumber = thisEventInfo->eventNumber();
      LB = thisEventInfo->lumiBlock();

      if(opp_sign){
        if(leading_good)
          mtype = 0;
        if(!leading_good)
          mtype = 2;
        if(leading_antigood)
          mtype = 4;
      }else{
        if(leading_good)
          mtype = 1;
        if(!leading_good)
          mtype = 3;
        if(leading_antigood)
          mtype = 5;
      }
    }
    fill(group_EleTP, pT, phi, eta, mass, runNumber, eventNumber, LB, mtype, weight);  
  }
}

void DQTGlobalWZFinderAlg::doEleContainerTP(std::vector<const xAOD::Electron*>& allElectrons, std::vector<const xAOD::Electron*>& goodelectrons, const EventContext& ctx) const{

  using namespace Monitored;
  
  auto group_EleContainerTP = getGroup("EleContainerTP");
  auto pass_kine = Scalar<bool>("pass_kine", false);
  auto container_nomatch = Scalar<bool>("container_nomatch", false);

  SG::ReadHandle<xAOD::PhotonContainer> photons(m_PhotonContainerKey, ctx);
  if ( ! photons.isValid() ) {
    ATH_MSG_ERROR("No photon container" << m_PhotonContainerKey << "found in evtStore");
    return;
  }

  ATH_MSG_DEBUG("PhotonContainer successfully retrieved");

  for (const auto& tagEl : goodelectrons) {
    bool matched = false;
    for (const auto &chain: m_Z_ee_trigger) {
      if (m_r3MatchingTool->match(*tagEl, chain, 0.1, false) || ! m_doTrigger) {
        matched=true;
        break;
      }
    }

    if (!matched) continue;
    auto tagElp4(tagEl->p4());

    if (tagEl->passSelection("LHTight")){
      for (const auto& el2 : allElectrons){
        if (el2 != tagEl && kinematicCuts(el2)){
          auto probeElp4(el2->p4());
          auto mass = Scalar("mass", (tagElp4+probeElp4).M());
          pass_kine = true;
          fill(group_EleContainerTP, mass, pass_kine);
          break;
        }
      }
    }

    for (const xAOD::Photon* photon : *photons) {
      auto photonp4(photon->p4());
      auto mass = Scalar("mass", (tagElp4+photonp4).M());

      if (!kinematicCuts(photon))
        continue;

      for (const auto& el2 : allElectrons){
        // slightly relax pT cut for probe electron
        bool passKinematics = true;
        if (el2->pt() < (m_electronEtCut-2)*GeV)
          passKinematics = false;
        if (std::abs(el2->caloCluster()->etaBE(2)) > 2.4)
          passKinematics = false;
        if (std::abs(el2->caloCluster()->etaBE(2)) > 1.37 && std::abs(el2->caloCluster()->etaBE(2)) < 1.52)
          passKinematics = false;

        double deltaR = (el2->p4()).DeltaR(photon->p4());
        if (!passKinematics || tagEl == el2 || deltaR < 0.1)
          continue;

        container_nomatch = true;
        fill(group_EleContainerTP, mass, container_nomatch);
        break;
      }
    }
  } 
}

bool DQTGlobalWZFinderAlg::kinematicCuts(const xAOD::Egamma* particle) const{

  bool isGood = true;
  if(particle->pt() < m_electronEtCut*GeV) isGood = false;

  if(std::abs(particle->caloCluster()->etaBE(2)) > 2.4) isGood = false;

  if(std::abs(particle->caloCluster()->etaBE(2)) > 1.37 &&
     std::abs(particle->caloCluster()->etaBE(2)) < 1.52) isGood = false;

  return isGood;
}


bool DQTGlobalWZFinderAlg::goodElectrons(const xAOD::Electron* electron_itr, const xAOD::Vertex* pVtx, const EventContext& ctx) const{

  using namespace Monitored;

  SG::ReadHandle<xAOD::EventInfo> thisEventInfo { GetEventInfo(ctx) };

  bool isGood = false;

  static const SG::Accessor<float> aptc20("ptcone20");
  float ptcone20 = 0;
  if (! aptc20.isAvailable(*electron_itr)) {
    ATH_MSG_WARNING("aptc20 is not available -  goodElectron");
  } else {
    ptcone20 = aptc20(*electron_itr);
  }

  float eleIso = 0.0;
  if ((electron_itr)->pt() != 0.0){
    eleIso = ptcone20/((electron_itr)->pt());
  }

  bool passSel = electron_itr->passSelection("LHMedium");
  auto elTrk = (electron_itr)->trackParticle();

  if (!elTrk) {
    return false;
  }
  float d0sig;
  try {
    d0sig = xAOD::TrackingHelpers::d0significance(elTrk, thisEventInfo->beamPosSigmaX(), thisEventInfo->beamPosSigmaY(), thisEventInfo->beamPosSigmaXY());
  } catch (...) {
      ATH_MSG_DEBUG("Invalid beamspot - electron");
    try {
      d0sig = xAOD::TrackingHelpers::d0significance(elTrk);
    } catch (...) {
        ATH_MSG_WARNING("Ridiculous exception thrown - electron");
      return false;
    }
  }

  if ( ((electron_itr)->pt() > m_electronEtCut*GeV) &&
       std::abs(electron_itr->caloCluster()->etaBE(2)) < 2.4 &&
       passSel &&
       (eleIso < 0.1) &&
       std::abs(d0sig) < 5 &&
       pVtx &&
       std::abs((elTrk->z0()+elTrk->vz()-pVtx->z())*std::sin(elTrk->theta())) < 0.5*mm)
      {	// electron dead zone
    if (std::abs((electron_itr)->caloCluster()->etaBE(2)) > 1.37 && std::abs((electron_itr)->caloCluster()->etaBE(2)) < 1.52 ){
      isGood = false;
    } else{
      isGood = true;
    }
  }
 
  return isGood;
}

bool DQTGlobalWZFinderAlg::antiGoodElectrons(const xAOD::Electron* electron_itr, const xAOD::Vertex* pVtx, const EventContext& ctx) const{

  using namespace Monitored;

  SG::ReadHandle<xAOD::EventInfo> thisEventInfo { GetEventInfo(ctx) };

  bool antiGood = false;

  static const SG::Accessor<float> aptc20("ptcone20");
  float ptcone20 = 0;
  if (! aptc20.isAvailable(*electron_itr)) {
    ATH_MSG_WARNING("aptc20 is not available -  antiGoodElectron");
  } else {
    ptcone20 = aptc20(*electron_itr);
  }

  float eleIso = 0.0;
  if ((electron_itr)->pt() != 0.0){
    eleIso = ptcone20/((electron_itr)->pt());
  }

  bool passID = electron_itr->passSelection("LHLoose");
  bool passIso = false;
  if(eleIso < 0.1) passIso = true;
  auto elTrk = (electron_itr)->trackParticle();

  if (!elTrk) {
    return false;
  }
  float d0sig;
  try {
    d0sig = xAOD::TrackingHelpers::d0significance(elTrk, thisEventInfo->beamPosSigmaX(), thisEventInfo->beamPosSigmaY(), thisEventInfo->beamPosSigmaXY());
  } catch (...) {
      ATH_MSG_DEBUG("Invalid beamspot - electron");
    try {
      d0sig = xAOD::TrackingHelpers::d0significance(elTrk);
    } catch (...) {
        ATH_MSG_WARNING("Ridiculous exception thrown - electron");
      return false;
    }
  }

  // pass basic selection, except ID+Isolation
  if (((electron_itr)->pt() > m_electronEtCut*GeV) &&
     std::abs(electron_itr->caloCluster()->etaBE(2)) < 2.4 &&
     std::abs(d0sig) < 5 &&
     pVtx &&
     std::abs((elTrk->z0()+elTrk->vz()-pVtx->z())*std::sin(elTrk->theta())) < 0.5*mm)
  {
    if(std::abs((electron_itr)->caloCluster()->etaBE(2)) > 1.37 && std::abs((electron_itr)->caloCluster()->etaBE(2)) < 1.52){
      antiGood = false;
      
    }else{
      if(!passID && !passIso) antiGood = true;
    }
  }
  
  return antiGood;
}


//compute trigger efficiencies
void DQTGlobalWZFinderAlg::doMuonTriggerTP(const xAOD::Muon* mu1, const xAOD::Muon* mu2, const EventContext& ctx, bool isSimulation, bool writeTTrees, const float evtWeight) const{
  //algorithm: plot # events with zero, one or two SL triggers
  //zero triggers for MC closure checks

  using namespace Monitored;

  SG::ReadHandle<xAOD::EventInfo> thisEventInfo { GetEventInfo(ctx) };
    
  auto group_MuonTriggerTP = getGroup("MuonTriggerTP"); 
  auto do_BCID = Scalar("do_BCID", false);
  auto isOS = Scalar("isOS", false); 
  auto matched = Scalar("matched", 0);
  auto weight = Scalar("weight", evtWeight);
  std::vector<const xAOD::Muon*> muons{mu1, mu2};

  //Truth matching
  if (isSimulation) {
    int truthMatching = 0;
    for (const auto mu: muons) {
      if (checkTruthMuon(mu)) {
        truthMatching++;
      }
    }
    if (truthMatching < 2) return;
  }

  for (const auto mu: muons) {
    for (const auto &chain: m_Z_mm_trigger) {
      if (m_r3MatchingTool->match(*mu, chain, 0.1, false)) {
        matched++;
        break;
      }
    }
  }
  fill(group_MuonTriggerTP, matched, weight);
  
  for (const auto& tagmu : muons) {
    bool matched_tag = false;
    for (const auto &chain: m_Z_mm_trigger) {
      if (m_r3MatchingTool->match(*tagmu, chain, 0.1, false)) {
        matched_tag=true;
        break;
      }
    }
    auto tagmup4(tagmu->p4());
    if (!matched_tag) continue;

    for (const auto& probemu : muons) {
      if (tagmu == probemu) {
        continue;
      }
      auto probemup4(probemu->p4());
      auto mass = Scalar("mass", (tagmup4+probemup4).M());
      bool matched_probe = false;
      if (mass < m_zCutLow*GeV || mass > m_zCutHigh*GeV) continue;

      auto pT = Scalar("pT", probemu->pt());
      auto eta = Scalar("eta", probemu->eta());
      auto phi = Scalar("phi", probemu->phi());
      auto isTruth = Scalar("isTruth", checkTruthMuon(probemu));
      auto mtype = Scalar("mtype", -1000);
      auto runNumber = Scalar("runNumber", thisEventInfo->runNumber());
      auto eventNumber = Scalar("eventNumber", thisEventInfo->eventNumber());
      auto LB = Scalar("LB", thisEventInfo->lumiBlock());

      if (!m_doTrigger){
        ATH_MSG_WARNING("Warning, the m_doTrigger activated");
      }

      for (const auto &chain: m_Z_mm_trigger) {
        if (m_r3MatchingTool->match(*probemu, chain, 0.1, false)) {
          matched_probe=true;
          break;
        }
      }

      if (matched_probe) {
        if (probemu->charge() != tagmu->charge()) {
          mtype = 0;
        }
        else {
          mtype = 1;
        }
        break;
      }


      else if (!matched_probe) {
        if (probemu->charge() != tagmu->charge()) {
          mtype = 2;
        }
        else {
          mtype = 3;
        }
      }
      if (writeTTrees){
        fill(group_MuonTriggerTP, pT, eta, phi, mass, isTruth, runNumber, LB, eventNumber, mtype, weight);
      }
    }
  }
}

void DQTGlobalWZFinderAlg::doMuonTruthEff(std::vector<const xAOD::Muon*>& goodmuonsZ, const EventContext& ctx) const{
   SG::ReadHandle<xAOD::TruthParticleContainer> vtruth(m_TruthParticleContainerKey, ctx);
   auto group_MuonTruthEff = getGroup("MuonTruthEff");
   if (! vtruth.isValid() ) {
     ATH_MSG_WARNING("No muon truth particles");
     return;
   }
   auto match = Monitored::Scalar("match", 0);
   for (const xAOD::TruthParticle* truthmu : *vtruth) {
     if (truthmu->abseta() > m_muonMaxEta || truthmu->pt() < m_muonPtCut*GeV) {
       continue;
     }
     TLorentzVector truthp4(truthmu->p4());
     match = 0;
     for (const auto& foundmu : goodmuonsZ) {
       if (foundmu->p4().DeltaR(truthp4) < 0.05) {
         match = 1;
         break;
       }
     }
     fill(group_MuonTruthEff, match);
   }
  
}



void DQTGlobalWZFinderAlg::doMuonLooseTP(std::vector<const xAOD::Muon*>& goodmuonsTP, const xAOD::Vertex* pVtx, const EventContext& ctx, bool isSimulation, bool writeTTrees, const float evtWeight) const{
  
  using namespace Monitored;

  auto group_MuonLooseTP = getGroup("MuonLooseTP");
  auto osmatch = Scalar<bool>("osmatch", false);
  auto ssmatch = Scalar<bool>("ssmatch", false);
  auto osnomatch = Scalar<bool>("osnomatch", false);
  auto ssnomatch = Scalar<bool>("ssnomatch", false);
  
  SG::ReadHandle<xAOD::EventInfo> thisEventInfo { GetEventInfo(ctx) }; 

  SG::ReadHandle<xAOD::TrackParticleContainer> idTracks_container_handle(m_idTrackParticleContainerKey, ctx);

  const xAOD::TrackParticleContainer* idTracks = idTracks_container_handle.cptr();  

  if (not idTracks) {
    ATH_MSG_FATAL("Unable to retrieve ID tacks to do muon T&P");
    return;
  }

  for (const auto& tagmu : goodmuonsTP) {
    
    // Truth matching
    if (isSimulation) {
      if (!checkTruthMuon(tagmu)) continue;
    }

    // only consider trigger-matched tags to avoid bias on probes
    bool matched = false;
    for (const auto &chain: m_Z_mm_trigger) {
      if (m_r3MatchingTool->match(*tagmu, chain, 0.1, false) || ! m_doTrigger) {
        matched=true;
        break;
      }
    }
    if (!matched) continue;
    auto tagmup4(tagmu->p4());
    for (const auto* trk : *idTracks) {
      
      // Truth matching
      if (isSimulation) {
        if (!checkTruthTrack(trk)) continue;
      }

      if (trk->pt() <  m_muonPtCut*GeV || std::abs(trk->eta()) > m_muonMaxEta)
        continue;
      if (std::abs((trk->z0()+trk->vz()-pVtx->z())*std::sin(trk->theta())) > 2*mm)  continue;

      auto trkp4(trk->p4());
      auto mass = Scalar("mass", (tagmup4+trkp4).M());
      if (mass < m_zCutLow*GeV || mass > m_zCutHigh*GeV) continue;
      auto pT = Scalar("pT", trk->pt());
      auto phi = Scalar("phi", trk->phi());
      auto eta = Scalar("eta", trk->eta());
      auto isTruth = Scalar("isTruth", checkTruthTrack(trk));
      auto runNumber = Scalar<int>("runNumber", thisEventInfo->runNumber());
      auto eventNumber = Scalar("eventNumber", thisEventInfo->eventNumber());
      auto mtype = Scalar("mtype", -1000);
      auto LB = Scalar("LB", thisEventInfo->lumiBlock());
      auto weight = Scalar("weight", evtWeight);

      bool opp_sign = (trk->charge() != tagmu->charge());
      bool matched = false;
      for (const auto& mu2: goodmuonsTP) {
        if (tagmu == mu2) continue;
        auto dR = Scalar("dR", trkp4.DeltaR(mu2->p4()));
	      auto dPT = Scalar("dPT", ((mu2->p4()).Pt() - trkp4.Pt()));

	      if (std::abs(dPT) < 10000 && dR < 0.05) {
	        matched = true;
	        break;
	      }
      }

      osmatch   = false;
      ssmatch   = false;
      osnomatch = false;
      ssnomatch = false;

      if (matched){
        mtype = (trk->charge() != tagmu->charge()) ? 0 : 1; 
        if (opp_sign) {
          osmatch = true;
        } else {
          ssmatch = true;
        }
      }
      else {
        mtype = (trk->charge() != tagmu->charge()) ? 2 : 3;
        if (opp_sign) {
          osnomatch = true;
        } else {
          ssnomatch = true;
        }
      }
      if (writeTTrees){
        fill(group_MuonLooseTP, pT, phi, eta, mass, isTruth, runNumber, LB, eventNumber, mtype, weight);
      }
      fill(group_MuonLooseTP, mass, osmatch, ssmatch, osnomatch, ssnomatch);
    }
  } 
}

void DQTGlobalWZFinderAlg::doMuonInDetTP(std::vector<const xAOD::Muon*>& goodmuonsZ, const xAOD::Vertex* pVtx, const EventContext& ctx, bool isSimulation, bool writeTTrees, const float evtWeight) const{

  using namespace Monitored;

  if (isSimulation) {
    int truthMatched = 0;
    for (const auto mu: goodmuonsZ) {
      if (checkTruthMuon(mu) == true) {
        truthMatched++;
      }
    }
    if (truthMatched < 2) return;
  }

  auto group_MuonInDetTP = getGroup("MuonInDetTP");
  auto osmatch = Scalar<bool>("osmatch", false);
  auto ssmatch = Scalar<bool>("ssmatch", false);
  auto osnomatch = Scalar<bool>("osnomatch", false);
  auto ssnomatch = Scalar<bool>("ssnomatch", false);

  SG::ReadHandle<xAOD::EventInfo> thisEventInfo { GetEventInfo(ctx) };

  SG::ReadHandle<xAOD::TrackParticleContainer> idTracks_container_handle(m_idTrackParticleContainerKey, ctx);
  SG::ReadHandle<xAOD::TrackParticleContainer> msTracks_container_handle(m_msTrackParticleContainerKey, ctx);

  const xAOD::TrackParticleContainer* idTracks = idTracks_container_handle.cptr();
  const xAOD::TrackParticleContainer* msTracks = msTracks_container_handle.cptr();

  if (not idTracks) {
    ATH_MSG_FATAL("Unable to retrieve ID tracks to do muon T&P");
  }
  if (not msTracks) {
    ATH_MSG_FATAL("Unable to retrieve MS tracks to do muon T&P");
  }

  for (const auto& tagmu : goodmuonsZ) {

    bool matched = false;
    for (const auto &chain: m_Z_mm_trigger) {
      if (m_r3MatchingTool->match(*tagmu, chain, 0.1, false) || ! m_doTrigger) {
        matched=true;
        break;
      }
    }
    if (!matched) continue;
    auto tagmup4(tagmu->p4());
    // For Every MS track....
    for (const xAOD::TrackParticle* trk : *msTracks) {
      if (trk->pt() < m_muonPtCut*GeV || std::abs(trk->eta()) > m_muonMaxEta)
        continue;
      if (std::abs((trk->z0()+trk->vz()-pVtx->z())*std::sin(trk->theta())) > 2*mm)
        continue;
      auto trkp4(trk->p4());
      auto mass = Scalar("mass", (tagmup4+trkp4).M());
      bool matched = false;

      if (mass < m_zCutLow*GeV || mass > m_zCutHigh*GeV) continue;

      auto pT = Scalar("pT", trk->pt());
      auto phi = Scalar("phi", trk->phi());
      auto eta = Scalar("eta", trk->eta());
      auto isTruth = Scalar("isTruth", checkTruthTrack(trk));
      auto mtype = Scalar("mtype", -1000);
      auto runNumber = Scalar("runNumber", thisEventInfo->runNumber());
      auto eventNumber = Scalar("eventNumber", thisEventInfo->eventNumber());
      auto weight = Scalar("weight", evtWeight);
      auto LB = Scalar("LB", thisEventInfo->lumiBlock());

      // for all ID tracks
      for (const xAOD::TrackParticle* mu2 : *idTracks) {
        auto idtrkp4(mu2->p4());
        auto mstrkp4(trk->p4());

        auto dR = Scalar("dR", idtrkp4.DeltaR(mstrkp4));
        auto dPT = Scalar("dPT", mstrkp4.Pt() - idtrkp4.Pt());

        //Currently using magic numbers tuned by eye, may want to fix in the future...
        if (std::abs(dPT) < 10000 && dR < 0.05){
          matched = true;
          break;
        }
      }

	    if (matched){
        (trk->charge() != tagmu->charge()) ? osmatch = true : ssmatch = true;
        mtype = (trk->charge() != tagmu->charge()) ? 0 : 1;
        if (writeTTrees) {
          fill(group_MuonInDetTP, pT, eta, phi, mass, mtype, isTruth, runNumber, LB, eventNumber, weight);
        }
        fill(group_MuonInDetTP, mass, osmatch, ssmatch);
      } else {
        (trk->charge() != tagmu->charge()) ? osnomatch = true : ssnomatch = true;
        mtype = (trk->charge() != tagmu->charge()) ? 2 : 3;
        if (writeTTrees) {
          fill(group_MuonInDetTP, pT, eta, phi, mass, mtype, isTruth, runNumber, LB, eventNumber, weight);
        }
	      fill(group_MuonInDetTP, mass, osnomatch, ssnomatch);  
      }
    }
  } 
}

bool DQTGlobalWZFinderAlg::checkTruthElectron(const xAOD::Electron* elec) const{

  using namespace MCTruthPartClassifier;

  // Check if input electron originates from a ZBoson, following EGamma recipe
  bool truthMatched = false;

  std::pair<unsigned int, unsigned int> res;

  const xAOD::TruthParticle* lastElTruth = xAOD::EgammaHelpers::getBkgElectronMother(elec);
  if( lastElTruth ){
    res=m_truthClassifier->particleTruthClassifier(lastElTruth);
  
    unsigned int iTypeOfPart = res.first;
    unsigned int iPartOrig   = res.second;

    if((iTypeOfPart == MCTruthPartClassifier::IsoElectron && iPartOrig == MCTruthPartClassifier::ZBoson) || (iPartOrig == MCTruthPartClassifier::FSRPhot)){
      truthMatched = true;
    }
  }

  return truthMatched;

}

bool DQTGlobalWZFinderAlg::checkTruthMuon(const xAOD::Muon* muon) const{

  using namespace MCTruthPartClassifier;

  // Check if input muon originates from a ZBoson
  bool truthMatched = false;

  std::pair<unsigned int, unsigned int> res;
  ParticleDef partDef;

  res=m_truthClassifier->particleTruthClassifier(muon);

  unsigned int iTypeOfPart = res.first;
  unsigned int iPartOrig   = res.second;

  auto muTrk = muon->trackParticle(xAOD::Muon::TrackParticleType::Primary);

  const auto* thePart = m_truthClassifier->getGenPart(muTrk);

  if(thePart){
    if(iTypeOfPart == MCTruthPartClassifier::IsoMuon && iPartOrig == MCTruthPartClassifier::ZBoson){
      truthMatched = true;
    }
  }

  return truthMatched;

}

bool DQTGlobalWZFinderAlg::checkTruthTrack(const xAOD::TrackParticle* trk) const{

  using namespace MCTruthPartClassifier;

  // Check if input track originates from a Z boson
  bool truthMatched = false;

  std::pair<unsigned int, unsigned int> res;
  ParticleDef partDef;

  res=m_truthClassifier->particleTruthClassifier(trk);

  unsigned int iTypeOfPart = res.first;
  unsigned int iPartOrig   = res.second;

  const auto* thePart = m_truthClassifier->getGenPart(trk);

  if(thePart){
    if(iTypeOfPart == MCTruthPartClassifier::IsoMuon && iPartOrig == MCTruthPartClassifier::ZBoson){
        truthMatched = true;
    }
  }

  return truthMatched;

}


void DQTGlobalWZFinderAlg::fillEleEffHistos(bool tag_good, bool probe_good, bool probe_anti_good, bool os, double el_mass) const{

  using namespace Monitored;

  if(!tag_good)
    return;

  auto group_EleTP = getGroup("EleTP");
  auto mass = Scalar("mass", el_mass);
  auto good_os = Scalar("good_os", false);
  auto good_ss = Scalar("good_ss", false);
  auto bad_os = Scalar("bad_os", false);
  auto bad_ss = Scalar("bad_ss", false);
  auto template_os = Scalar("template_os", false);
  auto template_ss = Scalar("template_ss", false);
 
  if(os){
    if(probe_good) good_os = true;
    else bad_os = true;
    if(probe_anti_good) template_os = true;
    fill(group_EleTP, mass, good_os, bad_os, template_os);
  }else{
    if(probe_good) good_ss = true;
    else bad_ss = true;
    if(probe_anti_good) template_ss = true;
    fill(group_EleTP, mass, good_ss, bad_ss, template_ss);
  }
}
