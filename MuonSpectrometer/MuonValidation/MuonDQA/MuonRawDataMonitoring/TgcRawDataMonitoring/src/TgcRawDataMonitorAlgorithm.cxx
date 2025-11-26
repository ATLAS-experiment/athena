/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcRawDataMonitorAlgorithm.h"

#include "TObjArray.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "StoreGate/ReadDecorHandle.h"
#include "GoodRunsLists/TGRLCollection.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include <fstream>

namespace {
  // Cut values on pt bein exploited throughout the monitoring
  constexpr double pt_30_cut = 30. * Gaudi::Units::GeV;
  constexpr double pt_15_cut = 15. * Gaudi::Units::GeV;
  constexpr double pt_10_cut = 10. * Gaudi::Units::GeV;
  constexpr double pt_4_cut = 4. * Gaudi::Units::GeV;

  /// End of the barrel region
  constexpr double barrel_end = 1.05;
  constexpr double eifi_boundary = 1.3;
  constexpr double endcap_end = 1.9;
  constexpr double trigger_end = 2.4;

  /// L1Muon geometric parameters
  constexpr int number_of_trigger_sectors_endcap = 48;
  constexpr int number_of_trigger_sectors_forward = 24;

  // offset for better drawing
  constexpr double tgc_coin_phi_small_offset = 0.0001;

  // NSW-related parameters
  constexpr double nsw_rmax = 5000;
  constexpr double nsw_rmin = 900;
  constexpr double nsw_z = 7824.46;
  constexpr double nsw_rindex_div = 255;
  constexpr double nsw_rindex_step = ( nsw_rmax - nsw_rmin ) / nsw_rindex_div;
}
TgcRawDataMonitorAlgorithm::TgcRawDataMonitorAlgorithm(const std::string &name, ISvcLocator *pSvcLocator) :
  AthMonitorAlgorithm(name, pSvcLocator) {
}

StatusCode TgcRawDataMonitorAlgorithm::initialize() {
  ATH_MSG_DEBUG("initialize()");
  ATH_CHECK(AthMonitorAlgorithm::initialize());
  ATH_CHECK(m_DetectorManagerKey.initialize());
  ATH_CHECK(m_MuonContainerKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_MuonRoIContainerKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_MuonRoIContainerBCm2Key.initialize(SG::AllowEmpty));
  ATH_CHECK(m_MuonRoIContainerBCm1Key.initialize(SG::AllowEmpty));
  ATH_CHECK(m_MuonRoIContainerBCp1Key.initialize(SG::AllowEmpty));
  ATH_CHECK(m_MuonRoIContainerBCp2Key.initialize(SG::AllowEmpty));
  ATH_CHECK(m_TgcPrepDataContainerKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_TgcCoinDataContainerCurrBCKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_TgcCoinDataContainerNextBCKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_TgcCoinDataContainerNextNextBCKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_TgcCoinDataContainerPrevBCKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_PrimaryVertexContainerKey.initialize(SG::AllowEmpty));

  ATH_CHECK(m_extrapolator.retrieve(DisableTool{m_extrapolator.empty()}));
  ATH_CHECK(m_tgcMonTool.retrieve(DisableTool{m_tgcMonTool.empty()}));
  ATH_CHECK(m_muonSelectionTool.retrieve(DisableTool{m_muonSelectionTool.empty()}));
  ATH_CHECK(m_GoodRunsListSelectorTool.retrieve(DisableTool{m_GoodRunsListSelectorTool.empty()}));

  ATH_CHECK(m_L1MenuKey.initialize()); // ReadHandleKey, but DetStore (so renounce)
  renounce(m_L1MenuKey);
  ATH_CHECK(m_thresholdPatternsKey.initialize(m_monitorThresholdPatterns));

  m_extZposition.push_back(m_M1_Z);
  m_extZposition.push_back(m_M2_Z);
  m_extZposition.push_back(m_M3_Z);
  m_extZposition.push_back(m_EI_Z);
  m_extZposition.push_back(m_FI_Z);
  m_extZposition.push_back(-m_M1_Z);
  m_extZposition.push_back(-m_M2_Z);
  m_extZposition.push_back(-m_M3_Z);
  m_extZposition.push_back(-m_EI_Z);
  m_extZposition.push_back(-m_FI_Z);
  

  if(m_ctpDecMonList!=""){
    m_CtpDecMonObj.clear();
    TString Str = m_ctpDecMonList.value();// format="Tit:L1_MU20_Run3,Mul:1,HLT:HLT_mu26_ivarmedium_L1MU20,RPC:6,TGC:12FCH;"
    std::unique_ptr<TObjArray> monTrigs( Str.Tokenize(";") );
    for(int i = 0 ; i < monTrigs->GetEntries() ; i++){
      TString monTrig = monTrigs->At(i)->GetName();
      if(monTrig.IsNull())continue;
      CtpDecMonObj monObj;
      monObj.trigItem = monObj.title = "dummy";
      monObj.rpcThr=monObj.tgcThr=monObj.multiplicity=0;
      monObj.tgcF=monObj.tgcC=monObj.tgcH=monObj.rpcR=monObj.rpcM=false;
      std::unique_ptr<TObjArray> monElement( monTrig.Tokenize(",") );
      for(int j = 0 ; j < monElement->GetEntries() ; j++){
	std::string sysItem = monElement->At(j)->GetName();
	if(sysItem.empty())continue;
	std::string item = sysItem.substr(4,sysItem.size());// remove "Tit:", "CTP:", "HLT:", "RPC:", "TGC:"
	if(sysItem.starts_with( "Tit")){
	  monObj.title = item;
	}else if(sysItem.starts_with( "Mul")){
	  monObj.multiplicity = std::atoi(item.data());
	}else if(sysItem.starts_with( "CTP")|| sysItem.starts_with( "HLT")){
	  monObj.trigItem = item;
	}else if(sysItem.starts_with( "RPC")){
	  monObj.rpcThr = std::atoi(item.data());
	  monObj.rpcR = (item.find('R')!=std::string::npos);
	  monObj.rpcM = (item.find('M')!=std::string::npos);
	}else if(sysItem.starts_with( "TGC")){
	  monObj.tgcThr = std::atoi(item.data());
	  monObj.tgcF = (item.find('F')!=std::string::npos);
	  monObj.tgcC = (item.find('C')!=std::string::npos);
	  monObj.tgcH = (item.find('H')!=std::string::npos);
	}
      }
      m_CtpDecMonObj.push_back(monObj);
    }
  }

  if(m_thrPatternList!=""){
    m_thrMonList.clear();
    TString Str = m_thrPatternList.value();
    std::unique_ptr<TObjArray> arr( Str.Tokenize(",") );
    for(int i = 0 ; i < arr->GetEntries() ; i++){
      std::string name = arr->At(i)->GetName();
      if(!name.starts_with( "MU"))continue;
      m_thrMonList.insert(name);
    }
  }

  if(m_maskChannelFileName!=""){
    ATH_MSG_INFO("Opening mask channel file: " << m_maskChannelFileName);
    std::ifstream fi(m_maskChannelFileName);
    if(fi){
      std::string str;
      while(getline(fi,str)){
	m_maskChannelList.insert(str);
      }
    }
    ATH_MSG_INFO("Number of mask channels = " << m_maskChannelList.size());
    fi.close();
  }

  return StatusCode::SUCCESS;
}

StatusCode 
TgcRawDataMonitorAlgorithm::printOutAvailableMuonTriggers() const{
 // Print out all available muon triggers
  // This is to be used when making a list of triggers
  // to be monitored, and writted in .py config file
  // The defult should be FALSE
  if( m_printAvailableMuonTriggers ){
    ATH_MSG_DEBUG("printAvailableMuonTriggers");
    if( getTrigDecisionTool().empty() ){
      ATH_MSG_ERROR("TDT is not availeble");
      return StatusCode::FAILURE;
    }else{
      std::set<std::string> available_muon_triggers;
      auto chainGroup = getTrigDecisionTool()->getChainGroup(".*");
      if( chainGroup != nullptr ){
	      auto triggerList = chainGroup->getListOfTriggers();
	      if( !triggerList.empty() ){
	        for(const auto &trig : triggerList) {
            std::string thisTrig = trig;
            if( thisTrig.find("mu")==std::string::npos && thisTrig.find("MU")==std::string::npos)continue;
            if(getTrigDecisionTool()->getNavigationFormat() == "TriggerElement") { // run 2 access
              auto fc = getTrigDecisionTool()->features(thisTrig.data(),TrigDefs::alsoDeactivateTEs);
              for(const auto& comb : fc.getCombinations()){
                auto initRoIs = comb.get<TrigRoiDescriptor>("initialRoI",TrigDefs::alsoDeactivateTEs);
                for(const auto& roi : initRoIs){
                  if( roi.empty() )continue;
                  if( roi.cptr()==nullptr ) continue;
                  // found an available muon trigger here
                  available_muon_triggers.insert(thisTrig);
                }
	            }
	          }else{ // run 3 access
              auto initialRoIs = getTrigDecisionTool()->features<TrigRoiDescriptorCollection>(thisTrig.data(), TrigDefs::includeFailedDecisions, "", TrigDefs::lastFeatureOfType, "initialRoI");
              for(const auto& roiLinkInfo : initialRoIs) {
                if( !roiLinkInfo.isValid() )continue;
                auto roiEL = roiLinkInfo.link;
                if( !roiEL.isValid() )continue;
                auto roi = *roiEL;
                if( roi==nullptr ) continue;
                // found an available muon trigger here
                available_muon_triggers.insert(thisTrig);
              }
            }
          }
        }
      }
      for(const auto& trig : available_muon_triggers){
	      ATH_MSG_INFO("Available Muon Trigger: " << trig);
      }
    }
  } ///////////////End of printing out available muon triggers
  return StatusCode::SUCCESS;
}

std::set<std::string>
TgcRawDataMonitorAlgorithm::checkTriggerInfo() const{
///////////////// Preparation: check trigger information /////////////////////
  ATH_MSG_DEBUG("Preparing trigger information");
  std::set<std::string> list_of_single_muon_triggers;
  if ( !getTrigDecisionTool().empty() ){
    auto chainGroup = getTrigDecisionTool()->getChainGroup("HLT_.*");
    if( chainGroup != nullptr ){
      auto triggerList = chainGroup->getListOfTriggers();
      if( !triggerList.empty() ){
        for(const auto &trig : triggerList) {
          if( trig.find("HLT_mu") != 0 )continue; // muon trigger
          if( trig.find('-') != std::string::npos )continue; // vetoing topo item
          if( trig.find("L1MU") == std::string::npos )continue; // RoI-seedeed L1 muon trigger
          if( trig.find("mu") !=  trig.rfind("mu") )continue;  // mu occurrence only once -> single muon trigger
          if( trig.find("MU") !=  trig.rfind("MU") )continue;  // MU occurrence only once -> single muon trigger
          list_of_single_muon_triggers.insert( trig );
        }
      }
    }
  }
  return list_of_single_muon_triggers;
}

const xAOD::Vertex* 
TgcRawDataMonitorAlgorithm::getPrimaryVertex( const EventContext& ctx) const{
  const xAOD::Vertex* primVertex = nullptr;
  if(!m_PrimaryVertexContainerKey.empty()){
    SG::ReadHandle <xAOD::VertexContainer> primVtxContainer(m_PrimaryVertexContainerKey, ctx);
    if(primVtxContainer.isValid()){
      for(const auto vtx : *primVtxContainer){
        if(vtx->vertexType() == xAOD::VxType::VertexType::PriVtx){
          primVertex = vtx;
          break;
        }
      }
    }
  }
  return primVertex;
}

std::vector<TgcRawDataMonitorAlgorithm::TimedMuonRoI>
TgcRawDataMonitorAlgorithm::getRegionsOfInterest( const EventContext& ctx) const {
std::vector<TimedMuonRoI> AllBCMuonRoIs;
  if (m_anaMuonRoI) {
    ATH_MSG_DEBUG("Getting MuonRoI pointer");
    /* raw LVL1MuonRoIs distributions */
    bool isRun3 = false;
    if (!m_MuonRoIContainerKey.empty()){
      SG::ReadHandle<xAOD::MuonRoIContainer > handle( m_MuonRoIContainerKey, ctx);
      if(handle.isValid()) {
        for(const auto roi : *handle.cptr()){
          isRun3 = roi->isRun3();
          TimedMuonRoI myMuonRoI(roi);// current BC
          AllBCMuonRoIs.push_back(myMuonRoI);
        }
      }
    }
    if(isRun3){
      if(!m_MuonRoIContainerBCm2Key.empty()){
        SG::ReadHandle<xAOD::MuonRoIContainer > handle( m_MuonRoIContainerBCm2Key, ctx);
        if(handle.isValid()) {
          for(const auto roi : *handle.cptr()){
            TimedMuonRoI myMuonRoI(roi,-2);
            AllBCMuonRoIs.push_back(myMuonRoI);
          }
        }
      }
      if(!m_MuonRoIContainerBCm1Key.empty()){
        SG::ReadHandle<xAOD::MuonRoIContainer > handle( m_MuonRoIContainerBCm1Key, ctx);
        if(handle.isValid()) {
          for(const auto roi : *handle.cptr()){
            TimedMuonRoI myMuonRoI(roi,-1);
            AllBCMuonRoIs.push_back(myMuonRoI);
          }
        }
      }
      if(!m_MuonRoIContainerBCp1Key.empty()){
        SG::ReadHandle<xAOD::MuonRoIContainer > handle( m_MuonRoIContainerBCp1Key, ctx);
        if(handle.isValid()) {
          for(const auto roi : *handle.cptr()){
            TimedMuonRoI myMuonRoI(roi,+1);
            AllBCMuonRoIs.push_back(myMuonRoI);
          }
        }
      }
      if(!m_MuonRoIContainerBCp2Key.empty()){
        SG::ReadHandle<xAOD::MuonRoIContainer > handle( m_MuonRoIContainerBCp2Key, ctx);
        if(handle.isValid()) {
          for(const auto roi : *handle.cptr()){
            const TimedMuonRoI myMuonRoI(roi,+2);
            AllBCMuonRoIs.push_back(myMuonRoI);
          }
        }
      }
    }
  }
  return AllBCMuonRoIs;
}

void 
TgcRawDataMonitorAlgorithm::fillRoiHistograms(const std::vector<TgcRawDataMonitorAlgorithm::TimedMuonRoI> & roiVec, const EventContext& ctx) const{
 ///////////////// Filling MuonRoI-only histograms  /////////////////
  if( not roiVec.empty() ){
    ATH_MSG_DEBUG("Filling MuonRoI-only histograms");
    MonVariables  roi_variables;
    auto roi_bcid = Monitored::Scalar<int>("roi_bcid", GetEventInfo(ctx)->bcid());
    roi_variables.push_back(roi_bcid);
    auto roi_pileup = Monitored::Scalar<int>("roi_pileup", lbAverageInteractionsPerCrossing(ctx));
    roi_variables.push_back(roi_pileup);
    auto roi_lumiBlock = Monitored::Scalar<int>("roi_lumiBlock", GetEventInfo(ctx)->lumiBlock());
    roi_variables.push_back(roi_lumiBlock);
    auto roi_timing = Monitored::Collection("roi_timing", roiVec, [](const TimedMuonRoI& m) {
	return m.timing;
      });
    roi_variables.push_back(roi_timing);
    auto roi_currentBC = Monitored::Collection("roi_currentBC", roiVec, [](const TimedMuonRoI& m) {
	return m.timing==0;
      });
    roi_variables.push_back(roi_currentBC);
    auto roi_previousBC = Monitored::Collection("roi_previousBC", roiVec, [](const TimedMuonRoI& m) {
	return m.timing==-1;
      });
    roi_variables.push_back(roi_previousBC);
    auto roi_nextBC = Monitored::Collection("roi_nextBC", roiVec, [](const TimedMuonRoI& m) {
	return m.timing==+1;
      });
    roi_variables.push_back(roi_nextBC);
    auto roi_roiNumber = Monitored::Collection("roi_roiNumber", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getRoI();
      });
    roi_variables.push_back(roi_roiNumber);
    auto roi_sector = Monitored::Collection("roi_sector", roiVec, [](const TimedMuonRoI& m) {
	return (m.muonRoI->getHemisphere() == xAOD::MuonRoI::Positive)?(m.muonRoI->getSectorID()+1):(-1 * m.muonRoI->getSectorID()-1);
      });
    roi_variables.push_back(roi_sector);
    auto roi_sectorAbs = Monitored::Collection("roi_sectorAbs", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getSectorID()+1;
      });
    roi_variables.push_back(roi_sectorAbs);
    auto roi_sector_wBW3Coin = Monitored::Collection("roi_sector_wBW3Coin", roiVec, [](const TimedMuonRoI& m) {
	return (m.muonRoI->getBW3Coincidence()) ? ((m.muonRoI->getHemisphere() == xAOD::MuonRoI::Positive)?(m.muonRoI->getSectorID()+1):(-1 * m.muonRoI->getSectorID()-1)) : (-999);
      });
    roi_variables.push_back(roi_sector_wBW3Coin);
    auto roi_sector_wInnerCoin = Monitored::Collection("roi_sector_wInnerCoin", roiVec, [](const TimedMuonRoI& m) {
	return (m.muonRoI->getInnerCoincidence()) ? ((m.muonRoI->getHemisphere() == xAOD::MuonRoI::Positive)?(m.muonRoI->getSectorID()+1):(-1 * m.muonRoI->getSectorID()-1)) : (-999);
      });
    roi_variables.push_back(roi_sector_wInnerCoin);
    auto roi_eta = Monitored::Collection("roi_eta", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->eta();
      });
    roi_variables.push_back(roi_eta);
    auto roi_eta_rpc = Monitored::Collection("roi_eta_rpc", roiVec, [](const TimedMuonRoI& m) {
	return (m.muonRoI->getSource() == xAOD::MuonRoI::Barrel)?(m.muonRoI->eta()):(-10);
      });
    roi_variables.push_back(roi_eta_rpc);
    auto roi_eta_tgc = Monitored::Collection("roi_eta_tgc", roiVec, [](const TimedMuonRoI& m) {
	return (m.muonRoI->getSource() != xAOD::MuonRoI::Barrel)?(m.muonRoI->eta()):(-10);
      });
    roi_variables.push_back(roi_eta_tgc);
    auto roi_wInnerCoinEtaUpTo1p3 = Monitored::Collection("roi_wInnerCoinEtaUpTo1p3", roiVec, [](const TimedMuonRoI& m) {
	return (m.muonRoI->getSource() != xAOD::MuonRoI::Barrel && std::abs(m.muonRoI->eta()) < 1.3 && m.muonRoI->getInnerCoincidence());
      });
    roi_variables.push_back(roi_wInnerCoinEtaUpTo1p3);
    auto roi_wInnerCoinEtaBeyond1p3 = Monitored::Collection("roi_wInnerCoinEtaBeyond1p3", roiVec, [](const TimedMuonRoI& m) {
	return (m.muonRoI->getSource() != xAOD::MuonRoI::Barrel && std::abs(m.muonRoI->eta()) > 1.3 && m.muonRoI->getInnerCoincidence());
      });
    roi_variables.push_back(roi_wInnerCoinEtaBeyond1p3);
    auto roi_eta_wInnerCoin = Monitored::Collection("roi_eta_wInnerCoin", roiVec, [](const TimedMuonRoI& m) {
	return (m.muonRoI->getInnerCoincidence() && m.muonRoI->getSource() != xAOD::MuonRoI::Barrel)?(m.muonRoI->eta()):(-10);
      });
    roi_variables.push_back(roi_eta_wInnerCoin);
    auto roi_eta_wBW3Coin = Monitored::Collection("roi_eta_wBW3Coin", roiVec, [](const TimedMuonRoI& m) {
	return (m.muonRoI->getBW3Coincidence() && m.muonRoI->getSource() != xAOD::MuonRoI::Barrel)?(m.muonRoI->eta()):(-10);
      });
    roi_variables.push_back(roi_eta_wBW3Coin);
    auto roi_eta_wInnerCoinVeto = Monitored::Collection("roi_eta_wInnerCoinVeto", roiVec, [](const TimedMuonRoI& m) {
	return (!m.muonRoI->getInnerCoincidence() && m.muonRoI->getSource() != xAOD::MuonRoI::Barrel)?(m.muonRoI->eta()):(-10);
      });
    roi_variables.push_back(roi_eta_wInnerCoinVeto);
    auto roi_eta_wBW3CoinVeto = Monitored::Collection("roi_eta_wBW3CoinVeto", roiVec, [](const TimedMuonRoI& m) {
	return (!m.muonRoI->getBW3Coincidence() && m.muonRoI->getSource() != xAOD::MuonRoI::Barrel)?(m.muonRoI->eta()):(-10);
      });
    roi_variables.push_back(roi_eta_wBW3CoinVeto);
    auto roi_phi = Monitored::Collection("roi_phi", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->phi();
      });
    roi_variables.push_back(roi_phi);
    auto roi_phi_sideA = Monitored::Collection("roi_phi_sideA", roiVec, [](const TimedMuonRoI& m) {
	return ( (m.muonRoI->getHemisphere() == xAOD::MuonRoI::Positive)?(m.muonRoI->phi()):(-10) );
      });
    roi_variables.push_back(roi_phi_sideA);
    auto roi_phi_sideC = Monitored::Collection("roi_phi_sideC", roiVec, [](const TimedMuonRoI& m) {
	return ( (m.muonRoI->getHemisphere() == xAOD::MuonRoI::Negative)?(m.muonRoI->phi()):(-10) );
      });
    roi_variables.push_back(roi_phi_sideC);
    auto roi_phi_rpc = Monitored::Collection("roi_phi_rpc", roiVec, [](const TimedMuonRoI& m) {
	return (m.muonRoI->getSource() == xAOD::MuonRoI::Barrel) ? m.muonRoI->phi() : -10;
      });
    roi_variables.push_back(roi_phi_rpc);
    auto roi_phi_tgc = Monitored::Collection("roi_phi_tgc", roiVec, [](const TimedMuonRoI& m) {
	return (m.muonRoI->getSource() != xAOD::MuonRoI::Barrel) ? m.muonRoI->phi() : -10;
      });
    roi_variables.push_back(roi_phi_tgc);
    auto roi_phi_wInnerCoin = Monitored::Collection("roi_phi_wInnerCoin", roiVec, [](const TimedMuonRoI& m) {
	return (m.muonRoI->getInnerCoincidence() && m.muonRoI->getSource() != xAOD::MuonRoI::Barrel)?(m.muonRoI->phi()):(-10);;
      });
    roi_variables.push_back(roi_phi_wInnerCoin);
    auto roi_phi_wBW3Coin = Monitored::Collection("roi_phi_wBW3Coin", roiVec, [](const TimedMuonRoI& m) {
	return (m.muonRoI->getBW3Coincidence() && m.muonRoI->getSource() != xAOD::MuonRoI::Barrel)?(m.muonRoI->phi()):(-10);;
      });
    roi_variables.push_back(roi_phi_wBW3Coin);
    auto roi_phi_wInnerCoinVeto = Monitored::Collection("roi_phi_wInnerCoinVeto", roiVec, [](const TimedMuonRoI& m) {
	return (!m.muonRoI->getInnerCoincidence() && m.muonRoI->getSource() != xAOD::MuonRoI::Barrel)?(m.muonRoI->phi()):(-10);;
      });
    roi_variables.push_back(roi_phi_wInnerCoinVeto);
    auto roi_phi_wBW3CoinVeto = Monitored::Collection("roi_phi_wBW3CoinVeto", roiVec, [](const TimedMuonRoI& m) {
	return (!m.muonRoI->getBW3Coincidence() && m.muonRoI->getSource() != xAOD::MuonRoI::Barrel)?(m.muonRoI->phi()):(-10);;
      });
    roi_variables.push_back(roi_phi_wBW3CoinVeto);
    auto roi_phi_wBW3Coin_sideA = Monitored::Collection("roi_phi_wBW3Coin_sideA", roiVec, [](const TimedMuonRoI& m) {
	return (m.muonRoI->getBW3Coincidence() && m.muonRoI->getHemisphere() == xAOD::MuonRoI::Positive)?(m.muonRoI->phi()):(-10);;
      });
    roi_variables.push_back(roi_phi_wBW3Coin_sideA);
    auto roi_phi_wBW3Coin_sideC = Monitored::Collection("roi_phi_wBW3Coin_sideC", roiVec, [](const TimedMuonRoI& m) {
	return (m.muonRoI->getBW3Coincidence() && m.muonRoI->getHemisphere() == xAOD::MuonRoI::Negative)?(m.muonRoI->phi()):(-10);;
      });
    roi_variables.push_back(roi_phi_wBW3Coin_sideC);
    auto roi_phi_wBW3CoinVeto_sideA = Monitored::Collection("roi_phi_wBW3CoinVeto_sideA", roiVec, [](const TimedMuonRoI& m) {
	return (!m.muonRoI->getBW3Coincidence() && m.muonRoI->getHemisphere() == xAOD::MuonRoI::Positive)?(m.muonRoI->phi()):(-10);;
      });
    roi_variables.push_back(roi_phi_wBW3CoinVeto_sideA);
    auto roi_phi_wBW3CoinVeto_sideC = Monitored::Collection("roi_phi_wBW3CoinVeto_sideC", roiVec, [](const TimedMuonRoI& m) {
	return (!m.muonRoI->getBW3Coincidence() && m.muonRoI->getHemisphere() == xAOD::MuonRoI::Negative)?(m.muonRoI->phi()):(-10);;
      });
    roi_variables.push_back(roi_phi_wBW3CoinVeto_sideC);
    auto roi_thr = Monitored::Collection("roi_thr", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getThrNumber();
      });
    roi_variables.push_back(roi_thr);
    auto roi_rpc = Monitored::Collection("roi_rpc", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getSource() == xAOD::MuonRoI::Barrel;
      });
    roi_variables.push_back(roi_rpc);
    auto roi_tgc = Monitored::Collection("roi_tgc", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getSource() != xAOD::MuonRoI::Barrel;
      });
    roi_variables.push_back(roi_tgc);
    auto roi_barrel = Monitored::Collection("roi_barrel", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getSource() == xAOD::MuonRoI::Barrel;
      });
    roi_variables.push_back(roi_barrel);
    auto roi_endcap = Monitored::Collection("roi_endcap", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getSource() == xAOD::MuonRoI::Endcap;
      });
    roi_variables.push_back(roi_endcap);
    auto roi_forward = Monitored::Collection("roi_forward", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getSource() == xAOD::MuonRoI::Forward;
      });
    roi_variables.push_back(roi_forward);
    auto roi_phi_barrel = Monitored::Collection("roi_phi_barrel", roiVec, [](const TimedMuonRoI& m) {
	return (m.muonRoI->getSource() == xAOD::MuonRoI::Barrel) ? m.muonRoI->phi() : -10;
      });
    roi_variables.push_back(roi_phi_barrel);
    auto roi_phi_endcap = Monitored::Collection("roi_phi_endcap", roiVec, [](const TimedMuonRoI& m) {
	return (m.muonRoI->getSource() == xAOD::MuonRoI::Endcap) ? m.muonRoI->phi() : -10;
      });
    roi_variables.push_back(roi_phi_endcap);
    auto roi_phi_forward = Monitored::Collection("roi_phi_forward", roiVec, [](const TimedMuonRoI& m) {
	return (m.muonRoI->getSource() == xAOD::MuonRoI::Forward) ? m.muonRoI->phi() : -10;
      });
    roi_variables.push_back(roi_phi_forward);
    auto roi_sideA = Monitored::Collection("roi_sideA", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getHemisphere() == xAOD::MuonRoI::Positive;
      });
    roi_variables.push_back(roi_sideA);
    auto roi_sideC = Monitored::Collection("roi_sideC", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getHemisphere() == xAOD::MuonRoI::Negative;
      });
    roi_variables.push_back(roi_sideC);
    auto thrmask1 = Monitored::Collection("thrmask1", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getThrNumber() == 1;
      });
    roi_variables.push_back(thrmask1);
    auto thrmask2 = Monitored::Collection("thrmask2", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getThrNumber() == 2;
      });
    roi_variables.push_back(thrmask2);
    auto thrmask3 = Monitored::Collection("thrmask3", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getThrNumber() == 3;
      });
    roi_variables.push_back(thrmask3);
    auto thrmask4 = Monitored::Collection("thrmask4", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getThrNumber() == 4;
      });
    roi_variables.push_back(thrmask4);
    auto thrmask5 = Monitored::Collection("thrmask5", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getThrNumber() == 5;
      });
    roi_variables.push_back(thrmask5);
    auto thrmask6 = Monitored::Collection("thrmask6", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getThrNumber() == 6;
      });
    roi_variables.push_back(thrmask6);
    auto thrmask7 = Monitored::Collection("thrmask7", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getThrNumber() == 7;
      });
    roi_variables.push_back(thrmask7);
    auto thrmask8 = Monitored::Collection("thrmask8", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getThrNumber() == 8;
      });
    roi_variables.push_back(thrmask8);
    auto thrmask9 = Monitored::Collection("thrmask9", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getThrNumber() == 9;
      });
    roi_variables.push_back(thrmask9);
    auto thrmask10 = Monitored::Collection("thrmask10", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getThrNumber() == 10;
      });
    roi_variables.push_back(thrmask10);
    auto thrmask11 = Monitored::Collection("thrmask11", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getThrNumber() == 11;
      });
    roi_variables.push_back(thrmask11);
    auto thrmask12 = Monitored::Collection("thrmask12", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getThrNumber() == 12;
      });
    roi_variables.push_back(thrmask12);
    auto thrmask13 = Monitored::Collection("thrmask13", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getThrNumber() == 13;
      });
    roi_variables.push_back(thrmask13);
    auto thrmask14 = Monitored::Collection("thrmask14", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getThrNumber() == 14;
      });
    roi_variables.push_back(thrmask14);
    auto thrmask15 = Monitored::Collection("thrmask15", roiVec, [](const TimedMuonRoI& m) {
	return m.muonRoI->getThrNumber() == 15;
      });
    roi_variables.push_back(thrmask15);
    auto roi_charge = Monitored::Collection("roi_charge", roiVec, [](const TimedMuonRoI& m) {
	return (m.muonRoI->getCharge()==xAOD::MuonRoI::Neg)?(-1):((m.muonRoI->getCharge()==xAOD::MuonRoI::Pos)?(+1):(0));
      });
    roi_variables.push_back(roi_charge);
    auto roi_bw3coin = Monitored::Collection("roi_bw3coin",roiVec,[](const TimedMuonRoI& m) {
	return m.muonRoI->getBW3Coincidence();
      });
    roi_variables.push_back(roi_bw3coin);
    auto roi_bw3coinveto = Monitored::Collection("roi_bw3coinveto",roiVec,[](const TimedMuonRoI& m) {
	return !m.muonRoI->getBW3Coincidence() && m.muonRoI->getSource()!=xAOD::MuonRoI::Barrel;
      });
    roi_variables.push_back(roi_bw3coinveto);
    auto roi_innercoin = Monitored::Collection("roi_innercoin",roiVec,[](const TimedMuonRoI& m) {
	return m.muonRoI->getInnerCoincidence();
      });
    roi_variables.push_back(roi_innercoin);
    auto roi_innveto = Monitored::Collection("roi_innveto",roiVec,[](const TimedMuonRoI& m) {
	return !m.muonRoI->getInnerCoincidence() && m.muonRoI->getSource()!=xAOD::MuonRoI::Barrel;
      });
    roi_variables.push_back(roi_innveto);
    auto roi_goodmf = Monitored::Collection("roi_goodmf",roiVec,[](const TimedMuonRoI& m) {
	return m.muonRoI->getGoodMF();
      });
    roi_variables.push_back(roi_goodmf);
    auto roi_badmf = Monitored::Collection("roi_badmf",roiVec,[](const TimedMuonRoI& m){
	return !m.muonRoI->getGoodMF() && m.muonRoI->getSource()!=xAOD::MuonRoI::Barrel;
      });
    roi_variables.push_back(roi_badmf);
    auto roi_ismorecand = Monitored::Collection("roi_ismorecand",roiVec,[](const TimedMuonRoI& m){
	return m.muonRoI->isMoreCandInRoI();
      });
    roi_variables.push_back(roi_ismorecand);
    auto roi_posCharge = Monitored::Collection("roi_posCharge",roiVec,[](const TimedMuonRoI& m){
	return m.muonRoI->getCharge()==xAOD::MuonRoI::Pos;
      });
    roi_variables.push_back(roi_posCharge);
    auto roi_negCharge = Monitored::Collection("roi_negCharge",roiVec,[](const TimedMuonRoI& m){
	return m.muonRoI->getCharge()==xAOD::MuonRoI::Neg;
      });
    roi_variables.push_back(roi_negCharge);
    fill(m_packageName, roi_variables);
    ATH_MSG_DEBUG("End filling MuonRoI-only histograms");
  }
}

void
TgcRawDataMonitorAlgorithm::fillHistogramsAfterTriggerDecision(std::vector<TgcRawDataMonitorAlgorithm::TimedMuonRoI> & roiVec) const {
///////////////// Filling histograms for MuonRoIs after trigger decision /////////////////
  if ( !getTrigDecisionTool().empty() && roiVec.size()>0 && m_monitorTriggerMultiplicity ) {
    ATH_MSG_DEBUG("Filling histograms for MuonRoIs after trigger decision");
    for(const auto& monObj : m_CtpDecMonObj){
      std::set<unsigned int> allCands;
      std::set<unsigned int> ctpMuonCands;
      std::set<unsigned int> inputMuonCands;
      // collecting roiWords out of the CTP decision
      bool isRun2Legacy = false;
      if(getTrigDecisionTool()->getNavigationFormat() == "TriggerElement") { // run 2 access
	isRun2Legacy = true;
	if(monObj.title.find("Run2Legacy")==std::string::npos) continue;
	auto fc = getTrigDecisionTool()->features(monObj.trigItem.data(),TrigDefs::alsoDeactivateTEs);
	for(const auto& comb : fc.getCombinations()){
	  auto initRoIs = comb.get<TrigRoiDescriptor>("initialRoI",TrigDefs::alsoDeactivateTEs);
	  for(const auto& roi : initRoIs){
	    if( roi.empty() )continue;
	    if( roi.cptr()==nullptr ) continue;
	    ctpMuonCands.insert(roi.cptr()->roiWord());
	    allCands.insert(roi.cptr()->roiWord());
	  }
	}
      }else{ // run 3 access
	auto initialRoIs = getTrigDecisionTool()->features<TrigRoiDescriptorCollection>(monObj.trigItem.data(), TrigDefs::includeFailedDecisions, "", TrigDefs::lastFeatureOfType, "initialRoI");
	for(const auto& roiLinkInfo : initialRoIs) {
	  if( !roiLinkInfo.isValid() )continue;
	  auto roiEL = roiLinkInfo.link;
	  if( !roiEL.isValid() )continue;
	  auto roi = *roiEL;
	  if( roi==nullptr ) continue;
	  ctpMuonCands.insert(roi->roiWord());
	  allCands.insert(roi->roiWord());
	}
      }
      // collecting roiWords out of RPC/TGC
      bool isRun3 = false;
      for(const auto& allBcMuonRoI : roiVec){
	const xAOD::MuonRoI* roi = allBcMuonRoI.muonRoI;
	isRun3 = roi->isRun3();
	if(roi->getSource()==xAOD::MuonRoI::Barrel){
	  if(roi->getThrNumber()<monObj.rpcThr)continue;
	  if(monObj.rpcM && !roi->isMoreCandInRoI())continue;
	}else{
	  if(roi->getThrNumber()<monObj.tgcThr)continue;
	  if(monObj.tgcF && !roi->getBW3Coincidence())continue;
	  if(monObj.tgcC && !roi->getInnerCoincidence())continue;
	  if(monObj.tgcH && !roi->getGoodMF())continue;
	}
	inputMuonCands.insert(roi->roiWord());
	allCands.insert(roi->roiWord());
      }
      if(!isRun3 && isRun2Legacy && monObj.title.find("Run2Legacy")==std::string::npos)continue; // Run2Legacy
      if(!isRun3 && !isRun2Legacy && (monObj.title.find("Run2Legacy")!=std::string::npos||monObj.title.find("Run3")!=std::string::npos))continue; // Run2
      if(isRun3 && monObj.title.find("Run3")==std::string::npos)continue; // Run3

      if(ctpMuonCands.size()==0 && inputMuonCands.size()<monObj.multiplicity)continue;

      std::vector<int> roiMatching_CTPin;
      std::vector<int> roiMatching_CTPout;

      std::vector<double> roi_Eta;
      std::vector<double> roi_Phi;
      std::vector<double> roi_dRmin;
      std::vector<double> roi_pTdiff;
      std::vector<int> roi_ThrNum;
      std::vector<int> roi_Charge;
      std::vector<int> roi_BW3Coin;
      std::vector<int> roi_InnerCoin;
      std::vector<int> roi_GoodMF;
      std::vector<int> roi_IsMoreCandInRoI;
      std::vector<int> roi_PhiOverlap;
      std::vector<int> roi_EtaOverlap;
      std::vector<int> roi_isVetoed;
      std::vector<bool> roi_inOk_outOk;
      std::vector<bool> roi_inOk_outNg;
      std::vector<bool> roi_inNg_outOk;

      for(const auto& allBcMuonRoI : roiVec){ // scan all MuonRoIs
	const xAOD::MuonRoI* roi = allBcMuonRoI.muonRoI;
	bool ctp_in  = inputMuonCands.find(roi->roiWord())!=inputMuonCands.end();
	bool ctp_out = ctpMuonCands.find(roi->roiWord())!=ctpMuonCands.end();
	if(!ctp_in && !ctp_out)continue;
	roiMatching_CTPin.push_back(ctp_in?1:0);
	roiMatching_CTPout.push_back(ctp_out?1:0);
	double dRmin = 1000;
	double pTdiff = -15;
	for(const auto& allBcMuonRoI2 : roiVec){ // scan all the other MuonRoIs to check the isolation
	  const xAOD::MuonRoI* roi2 = allBcMuonRoI2.muonRoI;
	  if(roi == roi2)continue;
	  double dr = xAOD::P4Helpers::deltaR(roi->eta(),roi->phi(),roi2->eta(),roi2->phi());
	  if(dr < dRmin){
	    dRmin = dr;
	    pTdiff = roi2->getThrNumber() - roi->getThrNumber();
	  }
	}
	// adjust the value so that the value can be in the histgram range
	if(dRmin>999) dRmin = -0.05;
	else if(dRmin>1.0) dRmin = 0.95;
	roi_Eta.push_back(roi->eta());
	roi_Phi.push_back(roi->phi());
	roi_dRmin.push_back(dRmin);
	roi_pTdiff.push_back(pTdiff);
	roi_ThrNum.push_back((roi->getSource()==xAOD::MuonRoI::Barrel)?(roi->getThrNumber()):(-roi->getThrNumber()));
	roi_Charge.push_back((roi->getCharge()==xAOD::MuonRoI::Neg)?(-1):((roi->getCharge()==xAOD::MuonRoI::Pos)?(+1):(0)));
	roi_BW3Coin.push_back((roi->getSource()!=xAOD::MuonRoI::Barrel)?roi->getBW3Coincidence():-1);
	roi_InnerCoin.push_back((roi->getSource()!=xAOD::MuonRoI::Barrel)?roi->getInnerCoincidence():-1);
	roi_GoodMF.push_back((roi->getSource()!=xAOD::MuonRoI::Barrel)?roi->getGoodMF():-1);
	roi_IsMoreCandInRoI.push_back((roi->getSource()==xAOD::MuonRoI::Barrel)?roi->isMoreCandInRoI():-1);
	roi_PhiOverlap.push_back((roi->getSource()==xAOD::MuonRoI::Barrel)?roi->getPhiOverlap():-1);
	roi_EtaOverlap.push_back(roi->getEtaOverlap());
	roi_isVetoed.push_back(roi->isVetoed());
	roi_inOk_outOk.push_back( ctp_in && ctp_out );
	roi_inOk_outNg.push_back( ctp_in && !ctp_out );
	roi_inNg_outOk.push_back( !ctp_in && ctp_out );
      }

      MonVariables  ctpMonVariables;
      auto val_roiMatching_CTPin = Monitored::Collection("roiMatching_CTPin", roiMatching_CTPin);
      auto val_roiMatching_CTPout = Monitored::Collection("roiMatching_CTPout", roiMatching_CTPout);

      auto val_ctpMultiplicity = Monitored::Scalar<int>("ctpMultiplicity",ctpMuonCands.size());
      auto val_rawMultiplicity = Monitored::Scalar<int>("rawMultiplicity",inputMuonCands.size());
      auto val_countDiff = Monitored::Scalar<int>("countDiff",ctpMuonCands.size()-inputMuonCands.size());

      auto val_roi_Eta = Monitored::Collection("Eta",roi_Eta);
      auto val_roi_Phi = Monitored::Collection("Phi",roi_Phi);
      auto val_roi_dRmin = Monitored::Collection("dRmin",roi_dRmin);
      auto val_roi_pTdiff = Monitored::Collection("pTdiff",roi_pTdiff);
      auto val_roi_ThrNum = Monitored::Collection("ThrNum",roi_ThrNum);
      auto val_roi_Charge = Monitored::Collection("Charge",roi_Charge);
      auto val_roi_BW3Coin = Monitored::Collection("BW3Coin",roi_BW3Coin);
      auto val_roi_InnerCoin = Monitored::Collection("InnerCoin",roi_InnerCoin);
      auto val_roi_GoodMF = Monitored::Collection("GoodMF",roi_GoodMF);
      auto val_roi_IsMoreCandInRoI = Monitored::Collection("IsMoreCandInRoI",roi_IsMoreCandInRoI);
      auto val_roi_PhiOverlap = Monitored::Collection("PhiOverlap",roi_PhiOverlap);
      auto val_roi_EtaOverlap = Monitored::Collection("EtaOverlap",roi_EtaOverlap);
      auto val_roi_isVetoed = Monitored::Collection("isVetoed",roi_isVetoed);
      auto val_roi_inOk_outOk = Monitored::Collection("inOk_outOk",roi_inOk_outOk);
      auto val_roi_inOk_outNg = Monitored::Collection("inOk_outNg",roi_inOk_outNg);
      auto val_roi_inNg_outOk = Monitored::Collection("inNg_outOk",roi_inNg_outOk);

      ctpMonVariables.push_back(val_roiMatching_CTPin);
      ctpMonVariables.push_back(val_roiMatching_CTPout);
      ctpMonVariables.push_back(val_ctpMultiplicity);
      ctpMonVariables.push_back(val_rawMultiplicity);
      ctpMonVariables.push_back(val_countDiff);
      ctpMonVariables.push_back(val_roi_Eta);
      ctpMonVariables.push_back(val_roi_Phi);
      ctpMonVariables.push_back(val_roi_dRmin);
      ctpMonVariables.push_back(val_roi_pTdiff);
      ctpMonVariables.push_back(val_roi_ThrNum);
      ctpMonVariables.push_back(val_roi_Charge);
      ctpMonVariables.push_back(val_roi_BW3Coin);
      ctpMonVariables.push_back(val_roi_InnerCoin);
      ctpMonVariables.push_back(val_roi_GoodMF);
      ctpMonVariables.push_back(val_roi_IsMoreCandInRoI);
      ctpMonVariables.push_back(val_roi_PhiOverlap);
      ctpMonVariables.push_back(val_roi_EtaOverlap);
      ctpMonVariables.push_back(val_roi_isVetoed);
      ctpMonVariables.push_back(val_roi_inOk_outOk);
      ctpMonVariables.push_back(val_roi_inOk_outNg);
      ctpMonVariables.push_back(val_roi_inNg_outOk);
      fill(m_packageName + monObj.title, ctpMonVariables);
    }
    ATH_MSG_DEBUG("End filling histograms for MuonRoIs after trigger decision");
  }
  ///////////////// End filling histograms for MuonRoIs after trigger decision /////////////////
}


StatusCode TgcRawDataMonitorAlgorithm::fillHistograms(const EventContext &ctx) const {
  ATH_MSG_DEBUG("fillHistograms()");

  if( !m_GoodRunsListSelectorTool.empty() ){
    int runNumber   = GetEventInfo(ctx)->runNumber();
    int lumiBlockNr = GetEventInfo(ctx)->lumiBlock();
    if(m_GoodRunsListSelectorTool->getGRLCollection()->IsEmpty()){
      ATH_MSG_ERROR("Empty GRL");
      return StatusCode::FAILURE;
    }
    bool pass = m_GoodRunsListSelectorTool->getGRLCollection()->HasRunLumiBlock(runNumber,lumiBlockNr);
    if(pass){
      ATH_MSG_DEBUG("passing GRL: run=" << runNumber << " lb=" << lumiBlockNr);
    }else{
      ATH_MSG_DEBUG("failed GRL: run=" << runNumber << " lb=" << lumiBlockNr);
      return StatusCode::SUCCESS;
    }
  }

  if( !m_streamerFilter.empty() ) {
    if(m_doExpressProcessing){
      const unsigned int passBits = getTrigDecisionTool()->isPassedBits(m_streamerFilter);
      const bool expressPass = passBits & TrigDefs::Express_passed;
      if(!expressPass){
        ATH_MSG_DEBUG("failed expressPass");
        return StatusCode::SUCCESS;
      }
    }
    bool pass = getTrigDecisionTool()->isPassed(m_streamerFilter,TrigDefs::Physics);
    if(pass){
      ATH_MSG_DEBUG("passing StreamerFilter: " << m_streamerFilter );
    }else{
      ATH_MSG_DEBUG("failed StreamerFilter: " << m_streamerFilter );
      return StatusCode::SUCCESS;
    }
  }
  if (auto sc = printOutAvailableMuonTriggers();not sc.isSuccess()) return sc;
  auto list_of_single_muon_triggers = checkTriggerInfo();
  
  ///////////////// End preparation: check trigger information /////////////////////

  const xAOD::Vertex* primVertex = getPrimaryVertex(ctx);
 
  double primaryVertexZ = (primVertex!=nullptr)?(primVertex->z()):(-999);
  // define common monitoring variables //
  auto mon_bcid = Monitored::Scalar<int>("mon_bcid", GetEventInfo(ctx)->bcid());
  auto mon_pileup = Monitored::Scalar<int>("mon_pileup", lbAverageInteractionsPerCrossing(ctx));
  auto mon_lb = Monitored::Scalar<int>("mon_lb", GetEventInfo(ctx)->lumiBlock());
  auto mon_primvtx_z=Monitored::Scalar<double>("mon_primvtx_z",primaryVertexZ);
  
  fill(m_packageName+"_Common", mon_bcid, mon_pileup, mon_lb, mon_primvtx_z);
  ///////////////// Extract MuonRoI /////////////////
  std::vector<TimedMuonRoI> AllBCMuonRoIs = getRegionsOfInterest(ctx);
  fillRoiHistograms(AllBCMuonRoIs, ctx);
  ///////////////// End filling MuonRoI-only histograms  /////////////////
  fillHistogramsAfterTriggerDecision(AllBCMuonRoIs);


  ///////////////// Filling histograms for MuonRoIs in thresholdPattern /////////////////
  std::map<const xAOD::MuonRoI*,std::set<std::string>> roiAndMenu;
  std::map<std::string,std::vector<const xAOD::MuonRoI*>> menuAndRoIs;
  fillMuonRoisInThresholdPattern(roiAndMenu, menuAndRoIs, AllBCMuonRoIs, ctx);
  ///////////////// End filling histograms for MuonRoIs in thresholdPattern /////////////////

  ///////////////// Filling offline muon-related histograms /////////////////
  std::vector < const xAOD::Muon* > oflmuons;
  std::set < const xAOD::Muon* > biasedMuons;
  std::vector < TimedMuon > mymuons;
  std::map < std::string, std::vector< ExtPos > > extpositions;
  std::vector< ExtPos > extpositions_pivot;
  std::vector<double> deltaR_muons;
  std::vector<double> deltaR_muons_roi;
  std::vector<double> deltaR_muons_hlt;
  std::vector<double> muon2pv_dz;
  std::vector<double> muon2pv_dca;
  std::vector<double> mymuon2pv_dz;
  std::vector<double> mymuon2pv_dca;
  if (m_anaOfflMuon) {
    SG::ReadHandle < xAOD::MuonContainer > muons(m_MuonContainerKey, ctx);
    if (!muons.isValid()) {
      ATH_MSG_ERROR("Failed to get xAOD::MuonContainer");
      return StatusCode::SUCCESS;
    }
    ATH_MSG_DEBUG("Filling offline muon-related histograms");
    for (const auto muon : *muons) {
      // skip if muon is empty
      if (muon == nullptr) continue;
      // standard quality cuts for muons
      if (muon->pt() < 1000.) continue;
      // minimum requirements
      if ( muon->author() > xAOD::Muon::Author::MuidSA )continue;
      if ( muon->muonType() > xAOD::Muon::MuonType::MuonStandAlone )continue;
      // very loose-quality muons
      oflmuons.push_back(muon);
      // selectable requirements
      double dz=-999,dca=-999;
      if( dataType() != DataType_t::cosmics ){
        if(m_useMuonSelectorTool && !m_muonSelectionTool->accept(*muon)) continue;
        if(m_useOnlyCombinedMuons && muon->muonType()!=xAOD::Muon::MuonType::Combined) continue;
        if(m_useOnlyMuidCoStacoMuons && (muon->author()!=xAOD::Muon::Author::MuidCo && muon->author()!=xAOD::Muon::Author::STACO)) continue;
        if(!m_PrimaryVertexContainerKey.empty()){
          if(primVertex==nullptr)continue;
          auto trackParticle = muon->primaryTrackParticle();
          if(trackParticle!=nullptr){
            dz = trackParticle->z0() - primVertex->z();
            dca = trackParticle->d0();
          }
          muon2pv_dz.push_back(dz);
          muon2pv_dca.push_back(dca);
          if( std::abs(dz-m_muonToPVdzOffset) > m_muonToPVdz )continue;
          if( std::abs(dca) > m_muonToPVdca )continue;
        }
      }
      // initialize for muon-isolation check
      bool isolated = true;
      // initialize for tag-and-probe check
      bool probeOK = true;
      if( m_TagAndProbe ) probeOK = false; // t&p should be performed
      if( dataType() == DataType_t::cosmics ) probeOK = true; // won't performa t&p for cosmics because no enough muons
      // OK, let's start looking at the second muons
      for(const auto muon2 : *muons){
        // skip if muon is empty
        if (muon2 == nullptr) continue;
      
        // skip the same muon candidate
        if( muon == muon2 )continue;
      
        // skip possible mismeasured muons
        if( muon2->pt() < 1000. ) continue;
      
        // minimum requirements on muon quality
        if ( muon2->author() > xAOD::Muon::Author::MuidSA )continue;
        if ( muon2->muonType() > xAOD::Muon::MuonType::MuonStandAlone )continue;
      
        // tag muon to be only in the other region, barrel or endcap, to remove possible bias from the same region
        if( m_tagMuonInDifferentSystem &&
            ( (std::abs(muon->eta()) < barrel_end && std::abs(muon2->eta()) < barrel_end) ||
              (std::abs(muon->eta()) > barrel_end && std::abs(muon2->eta()) > barrel_end) ) )continue;
      
        // isolation calculation
        double dr_muons = xAOD::P4Helpers::deltaR(muon,muon2,false);
        deltaR_muons.push_back(dr_muons);
        if( dr_muons < m_isolationWindow ) isolated = false;
      
        // no need to check further if probeOK is already True
        // 0) if muon-orthogonal triggers are avaialble/fired
        // 1) if we don't use tag-and-probe
        // 2) if TrigDecTool is not available
        // 3) if the second muon matches the trigger requirement
        if(probeOK)continue;
      
        // loop over the single muon triggers if at least one of them matches this second muon
        for (const auto &trigName : list_of_single_muon_triggers) {
          if(m_doExpressProcessing){ // check the express_express bit
            const unsigned int passBits = getTrigDecisionTool()->isPassedBits(trigName.data());
            const bool expressPass = passBits & TrigDefs::Express_passed;
            if(!expressPass)continue;
          }
          // check if this particular tirgger has fired in this event
          if (!getTrigDecisionTool()->isPassed(trigName.data(),TrigDefs::Physics)) continue;
          ATH_MSG_DEBUG("This muon trigger, " << trigName << ", is fired in this event!!");
          // check if this second muon matches the HLT muon trigger
          if(getTrigDecisionTool()->getNavigationFormat() == "TriggerElement") { // run 2 access
            ATH_MSG_DEBUG("Trying Run2-style feature access");
            auto fc = getTrigDecisionTool()->features(trigName.data(),TrigDefs::Physics);
            for(const auto& comb : fc.getCombinations()){
              if(!comb.active())continue;
              auto MuFeatureContainers = comb.get<xAOD::MuonContainer>("MuonEFInfo",TrigDefs::Physics);
              for(const auto& mucont : MuFeatureContainers){
                if(mucont.empty())continue;
                if(mucont.te()==nullptr)continue;
                if(!mucont.te()->getActiveState())continue;
                for(const auto hltmu : *mucont.cptr()){
                  if (hltmu == nullptr) continue; // skip if hltmu is empty
                  if (hltmu->pt() < 1000.)continue; // skip if pT is very small
                  double dr = xAOD::P4Helpers::deltaR(muon2,hltmu,false);
                  deltaR_muons_hlt.push_back(dr);
                  if( dr < m_trigMatchWindow ){
                    probeOK = true;
                    ATH_MSG_DEBUG("Trigger matched: "<<trigName<<" dR=" << dr );
                    biasedMuons.insert(muon2);
                  }
                }// end loop of mucont.cptr()
              }// end loop of MuonFeatureContainers
            }//end loop of Combinations
          }else{ // run 3 access
            ATH_MSG_DEBUG("Trying Run3-style feature access");
            auto features = getTrigDecisionTool()->features < xAOD::MuonContainer > (trigName.data(), TrigDefs::Physics, "HLT_MuonsCB_RoI");
            for (const auto& aaa : features) {
              if (!aaa.isValid()) continue;
              auto hltmu_link = aaa.link;
              if (!hltmu_link.isValid()) continue;
              auto hltmu = *hltmu_link;
              if (hltmu == nullptr) continue; // skip if hltmu is empty
              if (hltmu->pt() < 1000.)continue; // skip if pT is very small
              double dr = xAOD::P4Helpers::deltaR(muon2,hltmu,false);
              deltaR_muons_hlt.push_back(dr);
              if( dr < m_trigMatchWindow ){
                probeOK = true;
                ATH_MSG_DEBUG("Trigger matched: "<<trigName<<" dR=" << dr );
                biasedMuons.insert(muon2);
              }
            } // end loop of features
          } // end IF Run2 or Run3 feature access
        } // end loop of single muon triggers
        // check if the second muon matches the single muon trigger
        if(!probeOK) continue;
        ATH_MSG_DEBUG("Basic Tag-and-Probe is OK");
        // check further if this muon pair satisfies Z->mumu criteria
        if( m_TagAndProbeZmumu && muon->charge() != muon2->charge() ){
          double m2 = 2. * muon->pt() * muon2->pt() * ( std::cosh(muon->eta() - muon2->eta()) - std::cos(muon->phi() - muon2->phi()) );
          double m = (m2>0.) ? ( std::sqrt(m2) ) : (0.);
          double mdiff = std::abs( m - m_zMass );
          probeOK = mdiff < m_zMassWindow;
          ATH_MSG_DEBUG("Checking Zmumu cut: " << probeOK);
        }
	ATH_MSG_DEBUG("Final condition of probleOK for this muon is: " << probeOK);
	if(probeOK) break; // no need to check further if probeOK is already True
      } // end loop of the second muons
      // check if the isolation requirement is OK
      if(m_requireIsolated && !isolated)continue;
      // check if the tag-and-probe requirement is OK
      if(!probeOK)continue;

      TimedMuon mymuon;
      /* fill basic info */
      mymuon.muon = muon;
      /* fill extrapolation info (only to TGC) */
      if ( std::abs(muon->eta()) > 0.5 // only endcap region
	   && muon->pt() > m_pTCutOnExtrapolation ) { // only reasonably-high pT muons
        for (const auto &z : m_extZposition) {
          if( muon->eta()<0 && z>0 )continue;
          if( muon->eta()>0 && z<0 )continue;
          xAOD::Muon::TrackParticleType trkPtclType;
          if(m_useIDTrackForExtrapolation){ trkPtclType = xAOD::Muon::TrackParticleType::InnerDetectorTrackParticle;
          }else if(m_useMSTrackForExtrapolation){trkPtclType = xAOD::Muon::TrackParticleType::MuonSpectrometerTrackParticle;
          }else if(m_useCBTrackForExtrapolation){trkPtclType = xAOD::Muon::TrackParticleType::CombinedTrackParticle;
          }else if(m_useExtMSTrackForExtrapolation){trkPtclType = xAOD::Muon::TrackParticleType::ExtrapolatedMuonSpectrometerTrackParticle;
          }else if(m_useMSOnlyExtMSTrackForExtrapolation){trkPtclType = xAOD::Muon::TrackParticleType::MSOnlyExtrapolatedMuonSpectrometerTrackParticle;
          }else{ trkPtclType = xAOD::Muon::TrackParticleType::Primary; } // default is Primary (i.e. same as muonType )
          auto trackParticle = (m_useDirectPrimaryTrackForExtrapolation) ? muon->primaryTrackParticle() : muon->trackParticle( trkPtclType );
          if(trackParticle==nullptr)continue;
          auto matrix = std::make_unique<Amg::Transform3D>();
          matrix->setIdentity();
          matrix->translation().z() = z;
          auto disc = std::make_unique < Trk::DiscSurface > (*matrix,
                         m_endcapPivotPlaneMinimumRadius,
                         m_endcapPivotPlaneMaximumRadius);
          const Trk::BoundaryCheck boundaryCheck = true;
          const auto extTrkParams = m_extrapolator->extrapolate(ctx,
                      trackParticle->perigeeParameters(),
                      *disc,
                      Trk::alongMomentum,
                      boundaryCheck,
                      Trk::muon);
          if(extTrkParams != nullptr){
            if( std::abs(extTrkParams->position().z() - z) > 10. )continue; // wrong extrapolation
            ExtPos ext;
            ext.extPosZ = z;
            ext.extPos = extTrkParams->position();
            ext.extVec = extTrkParams->momentum();
            Amg::Vector3D extVec(extTrkParams->position().x(),extTrkParams->position().y(),z);
            ext.passedChambers = m_tgcMonTool->getPassedChambers(extVec);
            ext.muon = muon;
            if( std::abs( std::abs(z) - m_M3_Z ) < 10. &&  // trigger pivot plane (i.e. M3)
          std::abs( muon->eta() ) > 1.05 &&
          std::abs( muon->eta() ) < 2.40){ // only endcap
              extpositions_pivot.push_back(ext);
            }
            for(const auto& cham : ext.passedChambers){
              extpositions[cham].push_back(ext);
            }
          }
        }
      }
      /* L1Muon RoI matching */
      mymuon.matchedL1Charge=false;
      mymuon.passBW3Coin=false;
      mymuon.passInnerCoin=false;
      mymuon.passGoodMF=false;
      mymuon.passIsMoreCandInRoI=false;
      double max_dr = getMatchingWindow(mymuon.muon);
      if (AllBCMuonRoIs.size()==0) {
        ATH_MSG_DEBUG("No RoI matching possible as no container has been retrieved");
        mymuons.push_back(mymuon);
        continue;
      }
      for(const auto& allBcMuonRoI : AllBCMuonRoIs){
        const xAOD::MuonRoI* roi = allBcMuonRoI.muonRoI;
        double dr = xAOD::P4Helpers::deltaR(*muon,roi->eta(),roi->phi(),false);
        deltaR_muons_roi.push_back(dr);
        if( dr < max_dr ){
          if(roiAndMenu.count(roi)>0)mymuon.matchedL1Items.insert( roiAndMenu[roi].begin(), roiAndMenu[roi].end() );
          mymuon.matchedL1ThrExclusive.insert( roi->getThrNumber() );
          if(roi->getSource()!=xAOD::MuonRoI::Barrel)mymuon.matchedL1ThrExclusiveTGC.insert( roi->getThrNumber() );
          if(muon->charge()<0 && roi->getCharge()==xAOD::MuonRoI::Neg)mymuon.matchedL1Charge|=true;
          else if(muon->charge()>0 && roi->getCharge()==xAOD::MuonRoI::Pos)mymuon.matchedL1Charge|=true;
          mymuon.passBW3Coin|=roi->getBW3Coincidence();
          mymuon.passInnerCoin|=roi->getInnerCoincidence();
          mymuon.passGoodMF|=roi->getGoodMF();
          mymuon.passIsMoreCandInRoI|=roi->isMoreCandInRoI();
        }
      }
      for (int ithr = 1; ithr <= 15 ; ++ithr) { // TGC thresholds from 1 up to 15
        for (const auto &thr : mymuon.matchedL1ThrExclusive) {
          if (thr >= ithr) {
            mymuon.matchedL1ThrInclusive.insert(ithr);
            break;
          }
        }
        for (const auto &thr : mymuon.matchedL1ThrExclusiveTGC) {
          if (thr >= ithr) {
            mymuon.matchedL1ThrInclusiveTGC.insert(ithr);
            break;
          }
        }
      }
      /* store TimedMuon */
      mymuons.push_back(mymuon);
      mymuon2pv_dz.push_back(dz);
      mymuon2pv_dca.push_back(dca);
    }


    auto oflmuon_num=Monitored::Scalar<int>("oflmuon_num",(*muons).size());
    auto oflmuon_muonType=Monitored::Collection("oflmuon_muonType",*muons,[](const xAOD::Muon*m){return m->muonType();});
    auto oflmuon_author=Monitored::Collection("oflmuon_author",*muons,[](const xAOD::Muon*m){return m->author();});
    auto oflmuon_quality=Monitored::Collection("oflmuon_quality",*muons,[](const xAOD::Muon*m){return m->quality();});
    auto oflmuon_pt=Monitored::Collection("oflmuon_pt",*muons,[](const xAOD::Muon*m){return m->pt() / Gaudi::Units::GeV;});
    auto oflmuon_eta=Monitored::Collection("oflmuon_eta",*muons,[](const xAOD::Muon*m){return m->eta();});
    auto oflmuon_phi=Monitored::Collection("oflmuon_phi",*muons,[](const xAOD::Muon*m){return m->phi();});
    auto oflmuon_pvdz=Monitored::Collection("oflmuon_pvdz",muon2pv_dz);
    auto oflmuon_pvdca=Monitored::Collection("oflmuon_pvdca",muon2pv_dca);

    auto oflmuon_probe_num=Monitored::Scalar<int>("oflmuon_probe_num",mymuons.size());
    auto oflmuon_probe_muonType=Monitored::Collection("oflmuon_probe_muonType",mymuons,[](const TimedMuon&m){return m.muon->muonType();});
    auto oflmuon_probe_author=Monitored::Collection("oflmuon_probe_author",mymuons,[](const TimedMuon&m){return m.muon->author();});
    auto oflmuon_probe_quality=Monitored::Collection("oflmuon_probe_quality",mymuons,[](const TimedMuon&m){return m.muon->quality();});
    auto oflmuon_probe_pt=Monitored::Collection("oflmuon_probe_pt",mymuons,[](const TimedMuon&m){return m.muon->pt() / Gaudi::Units::GeV;});
    auto oflmuon_probe_eta=Monitored::Collection("oflmuon_probe_eta",mymuons,[](const TimedMuon&m){return m.muon->eta();});
    auto oflmuon_probe_phi=Monitored::Collection("oflmuon_probe_phi",mymuons,[](const TimedMuon&m){return m.muon->phi();});
    auto oflmuon_probe_pvdz=Monitored::Collection("oflmuon_probe_pvdz",mymuon2pv_dz);
    auto oflmuon_probe_pvdca=Monitored::Collection("oflmuon_probe_pvdca",mymuon2pv_dca);

    auto oflmuon_deltaR=Monitored::Collection("oflmuon_deltaR",deltaR_muons);
    auto oflmuon_deltaR_roi=Monitored::Collection("oflmuon_deltaR_roi",deltaR_muons_roi);
    auto oflmuon_deltaR_hlt=Monitored::Collection("oflmuon_deltaR_hlt",deltaR_muons_hlt);


    fill(m_packageName+"_Muon",
	 oflmuon_num,oflmuon_muonType,oflmuon_author,oflmuon_quality,oflmuon_pt,oflmuon_eta,oflmuon_phi,oflmuon_pvdz,oflmuon_pvdca,
	 oflmuon_probe_num,oflmuon_probe_muonType,oflmuon_probe_author,oflmuon_probe_quality,oflmuon_probe_pt,oflmuon_probe_eta,oflmuon_probe_phi,oflmuon_probe_pvdz,oflmuon_probe_pvdca,
	 oflmuon_deltaR, oflmuon_deltaR_roi, oflmuon_deltaR_hlt
	 );


    
    MonVariables  oflmuon_variables;
    oflmuon_variables.push_back(oflmuon_num);
    oflmuon_variables.push_back(oflmuon_muonType);
    oflmuon_variables.push_back(oflmuon_author);
    oflmuon_variables.push_back(oflmuon_quality);
    oflmuon_variables.push_back(oflmuon_pt);
    oflmuon_variables.push_back(oflmuon_eta);
    oflmuon_variables.push_back(oflmuon_phi);


    auto muon_charge = Monitored::Collection("muon_charge",mymuons,[](const TimedMuon& m){
	return m.muon->charge();
      });
    oflmuon_variables.push_back(muon_charge);
    auto muon_chargePos = Monitored::Collection("muon_chargePos",mymuons,[](const TimedMuon& m){
return (m.muon->charge()>0);
      });
    oflmuon_variables.push_back(muon_chargePos);
    auto muon_chargeNeg = Monitored::Collection("muon_chargeNeg",mymuons,[](const TimedMuon& m){
	return (m.muon->charge()<0);
      });
    oflmuon_variables.push_back(muon_chargeNeg);
    auto muon_eta4gev = Monitored::Collection("muon_eta4gev",mymuons,[](const TimedMuon& m){
    	return (m.muon->pt()>pt_4_cut)?m.muon->eta():-10;
      });
    oflmuon_variables.push_back(muon_eta4gev);
    auto muon_phi4gev = Monitored::Collection("muon_phi4gev",mymuons,[](const TimedMuon& m){
    	return (m.muon->pt()>pt_4_cut)?m.muon->phi():-10;
      });
    oflmuon_variables.push_back(muon_phi4gev);
    auto muon_phi4gev_1p05eta1p3 = Monitored::Collection("muon_phi4gev_1p05eta1p3",mymuons,[](const TimedMuon& m){
	return (m.muon->pt()>pt_4_cut && std::abs(m.muon->eta())>barrel_end && std::abs(m.muon->eta())<eifi_boundary)?m.muon->phi():-10;
      });
    oflmuon_variables.push_back(muon_phi4gev_1p05eta1p3);
    auto muon_phi4gev_1p05eta1p3A = Monitored::Collection("muon_phi4gev_1p05eta1p3A",mymuons,[](const TimedMuon& m){
	return (m.muon->pt()>pt_4_cut && std::abs(m.muon->eta())>barrel_end && std::abs(m.muon->eta())<eifi_boundary && m.muon->eta()>0)?m.muon->phi():-10;
      });
    oflmuon_variables.push_back(muon_phi4gev_1p05eta1p3A);
    auto muon_phi4gev_1p05eta1p3C = Monitored::Collection("muon_phi4gev_1p05eta1p3C",mymuons,[](const TimedMuon& m){
	return (m.muon->pt()>pt_4_cut && std::abs(m.muon->eta())>barrel_end && std::abs(m.muon->eta())<eifi_boundary && m.muon->eta()<0)?m.muon->phi():-10;
      });
    oflmuon_variables.push_back(muon_phi4gev_1p05eta1p3C);
    auto muon_phi4gev_1p3eta2p4 = Monitored::Collection("muon_phi4gev_1p3eta2p4",mymuons,[](const TimedMuon& m){
	return (m.muon->pt()>pt_4_cut && std::abs(m.muon->eta())>eifi_boundary && std::abs(m.muon->eta())<trigger_end)?m.muon->phi():-10;
      });
    oflmuon_variables.push_back(muon_phi4gev_1p3eta2p4);
    auto muon_phi4gev_1p3eta2p4A = Monitored::Collection("muon_phi4gev_1p3eta2p4A",mymuons,[](const TimedMuon& m){
	return (m.muon->pt()>pt_4_cut && std::abs(m.muon->eta())>eifi_boundary && std::abs(m.muon->eta())<trigger_end && m.muon->eta()>0)?m.muon->phi():-10;
      });
    oflmuon_variables.push_back(muon_phi4gev_1p3eta2p4A);
    auto muon_phi4gev_1p3eta2p4C = Monitored::Collection("muon_phi4gev_1p3eta2p4C",mymuons,[](const TimedMuon& m){
	return (m.muon->pt()>pt_4_cut && std::abs(m.muon->eta())>eifi_boundary && std::abs(m.muon->eta())<trigger_end && m.muon->eta()<0)?m.muon->phi():-10;
      });
    oflmuon_variables.push_back(muon_phi4gev_1p3eta2p4C);
    auto muon_phi4gev_rpc = Monitored::Collection("muon_phi4gev_rpc",mymuons,[](const TimedMuon& m){
	return (std::abs(m.muon->eta()) < barrel_end && m.muon->pt() > pt_4_cut) ? m.muon->phi() : -10;
      });
    oflmuon_variables.push_back(muon_phi4gev_rpc);
    auto muon_phi4gev_rpcA = Monitored::Collection("muon_phi4gev_rpcA",mymuons,[](const TimedMuon& m){
	return (std::abs(m.muon->eta()) < barrel_end && m.muon->pt() > pt_4_cut && m.muon->eta() > 0) ? m.muon->phi() : -10;
      });
    oflmuon_variables.push_back(muon_phi4gev_rpcA);
    auto muon_phi4gev_rpcC = Monitored::Collection("muon_phi4gev_rpcC",mymuons,[](const TimedMuon& m){
	return (std::abs(m.muon->eta()) < barrel_end && m.muon->pt() > pt_4_cut && m.muon->eta() < 0) ? m.muon->phi() : -10;
      });
    oflmuon_variables.push_back(muon_phi4gev_rpcC);
    auto muon_phi4gev_tgc = Monitored::Collection("muon_phi4gev_tgc",mymuons,[](const TimedMuon& m){
	return (std::abs(m.muon->eta()) > barrel_end && std::abs(m.muon->eta()) < trigger_end && m.muon->pt() > pt_4_cut) ? m.muon->phi() : -10;
      });
    oflmuon_variables.push_back(muon_phi4gev_tgc);
    auto muon_phi4gev_tgcA = Monitored::Collection("muon_phi4gev_tgcA",mymuons,[](const TimedMuon& m){
	return (std::abs(m.muon->eta()) > barrel_end && std::abs(m.muon->eta()) < trigger_end && m.muon->pt() > pt_4_cut && m.muon->eta() > 0) ? m.muon->phi() : -10;
      });
    oflmuon_variables.push_back(muon_phi4gev_tgcA);
    auto muon_phi4gev_tgcC = Monitored::Collection("muon_phi4gev_tgcC",mymuons,[](const TimedMuon& m){
	return (std::abs(m.muon->eta()) > barrel_end && std::abs(m.muon->eta()) < trigger_end && m.muon->pt() > pt_4_cut && m.muon->eta() < 0) ? m.muon->phi() : -10;
      });
    oflmuon_variables.push_back(muon_phi4gev_tgcC);
    auto muon_phi0gev_tgc = Monitored::Collection("muon_phi0gev_tgc",mymuons,[](const TimedMuon& m){
	return (std::abs(m.muon->eta()) > barrel_end && std::abs(m.muon->eta()) < trigger_end) ? m.muon->phi() : -10;
      });
    oflmuon_variables.push_back(muon_phi0gev_tgc);
    auto muon_phi0gev_tgcA = Monitored::Collection("muon_phi0gev_tgcA",mymuons,[](const TimedMuon& m){
	return (std::abs(m.muon->eta()) > barrel_end && std::abs(m.muon->eta()) < trigger_end && m.muon->eta() > 0) ? m.muon->phi() : -10;
      });
    oflmuon_variables.push_back(muon_phi0gev_tgcA);
    auto muon_phi0gev_tgcC = Monitored::Collection("muon_phi0gev_tgcC",mymuons,[](const TimedMuon& m){
	return (std::abs(m.muon->eta()) > barrel_end && std::abs(m.muon->eta()) < trigger_end && m.muon->eta() < 0) ? m.muon->phi() : -10;
      });
    oflmuon_variables.push_back(muon_phi0gev_tgcC);
    auto muon_eta = Monitored::Collection("muon_eta", mymuons, [](const TimedMuon &m) {
    	return (m.muon->pt() > pt_30_cut) ? m.muon->eta() : -10;
      });
    oflmuon_variables.push_back(muon_eta);
    auto muon_phi = Monitored::Collection("muon_phi", mymuons, [](const TimedMuon &m) {
    	return (m.muon->pt() > pt_30_cut) ? m.muon->phi() : -10;
      });
    oflmuon_variables.push_back(muon_phi);
    auto muon_phi_rpc = Monitored::Collection("muon_phi_rpc", mymuons, [](const TimedMuon &m) {
    	return (std::abs(m.muon->eta()) < barrel_end && m.muon->pt() > pt_30_cut) ? m.muon->phi() : -10;
      });
    oflmuon_variables.push_back(muon_phi_rpc);
    auto muon_phi_rpcA = Monitored::Collection("muon_phi_rpcA", mymuons, [](const TimedMuon &m) {
	return (std::abs(m.muon->eta()) < barrel_end && m.muon->pt() > pt_30_cut && m.muon->eta()>0) ? m.muon->phi() : -10;
      });
    oflmuon_variables.push_back(muon_phi_rpcA);
    auto muon_phi_rpcC = Monitored::Collection("muon_phi_rpcC", mymuons, [](const TimedMuon &m) {
	return (std::abs(m.muon->eta()) < barrel_end && m.muon->pt() > pt_30_cut && m.muon->eta()<0) ? m.muon->phi() : -10;
      });
    oflmuon_variables.push_back(muon_phi_rpcC);
    auto muon_phi_tgc = Monitored::Collection("muon_phi_tgc", mymuons, [](const TimedMuon &m) {
	return (std::abs(m.muon->eta()) > barrel_end && std::abs(m.muon->eta()) < trigger_end && m.muon->pt() > pt_30_cut) ? m.muon->phi() : -10;
      });
    oflmuon_variables.push_back(muon_phi_tgc);
    auto muon_phi_tgcA = Monitored::Collection("muon_phi_tgcA", mymuons, [](const TimedMuon &m) {
	return (std::abs(m.muon->eta()) > barrel_end && std::abs(m.muon->eta()) < trigger_end && m.muon->pt() > pt_30_cut && m.muon->eta()>0) ? m.muon->phi() : -10;
      });
    oflmuon_variables.push_back(muon_phi_tgcA);
    auto muon_phi_tgcC = Monitored::Collection("muon_phi_tgcC", mymuons, [](const TimedMuon &m) {
	return (std::abs(m.muon->eta()) > barrel_end && std::abs(m.muon->eta()) < trigger_end && m.muon->pt() > pt_30_cut && m.muon->eta()<0) ? m.muon->phi() : -10;
      });
    oflmuon_variables.push_back(muon_phi_tgcC);
    auto muon_pt_rpc = Monitored::Collection("muon_pt_rpc", mymuons, [](const TimedMuon &m) {
	return (std::abs(m.muon->eta()) < barrel_end) ? m.muon->pt() / Gaudi::Units::GeV : -10;
      });
    oflmuon_variables.push_back(muon_pt_rpc);
    auto muon_pt_tgc = Monitored::Collection("muon_pt_tgc", mymuons, [](const TimedMuon &m) {
    	return (std::abs(m.muon->eta()) > barrel_end && std::abs(m.muon->eta()) < trigger_end) ? m.muon->pt() / Gaudi::Units::GeV : -10;
      });
    oflmuon_variables.push_back(muon_pt_tgc);
    auto muon_barrel = Monitored::Collection("muon_barrel", mymuons, [](const TimedMuon &m) {
	return (std::abs(m.muon->eta()) < barrel_end);
      });
    oflmuon_variables.push_back(muon_barrel);
    auto muon_endcap = Monitored::Collection("muon_endcap", mymuons, [](const TimedMuon &m) {
	return (std::abs(m.muon->eta()) > barrel_end && std::abs(m.muon->eta()) < endcap_end);
      });
    oflmuon_variables.push_back(muon_endcap);
    auto muon_forward = Monitored::Collection("muon_forward", mymuons, [](const TimedMuon &m) {
	return (std::abs(m.muon->eta()) > endcap_end && std::abs(m.muon->eta()) < trigger_end);
      });
    oflmuon_variables.push_back(muon_forward);
    auto muon_l1passThr1TGC = Monitored::Collection("muon_l1passThr1TGC", mymuons, [](const TimedMuon &m) {
    	return m.matchedL1ThrInclusiveTGC.find(1) != m.matchedL1ThrInclusiveTGC.end();
      });
    oflmuon_variables.push_back(muon_l1passThr1TGC);
    auto muon_l1passThr1 = Monitored::Collection("muon_l1passThr1", mymuons, [](const TimedMuon &m) {
    	return m.matchedL1ThrInclusive.find(1) != m.matchedL1ThrInclusive.end();
      });
    oflmuon_variables.push_back(muon_l1passThr1);
    auto muon_l1passThr2 = Monitored::Collection("muon_l1passThr2", mymuons, [](const TimedMuon &m) {
    	return m.matchedL1ThrInclusive.find(2) != m.matchedL1ThrInclusive.end();
      });
    oflmuon_variables.push_back(muon_l1passThr2);
    auto muon_l1passThr3 = Monitored::Collection("muon_l1passThr3", mymuons, [](const TimedMuon &m) {
    	return m.matchedL1ThrInclusive.find(3) != m.matchedL1ThrInclusive.end();
      });
    oflmuon_variables.push_back(muon_l1passThr3);
    auto muon_l1passThr4 = Monitored::Collection("muon_l1passThr4", mymuons, [](const TimedMuon &m) {
    	return m.matchedL1ThrInclusive.find(4) != m.matchedL1ThrInclusive.end();
      });
    oflmuon_variables.push_back(muon_l1passThr4);
    auto muon_l1passThr5 = Monitored::Collection("muon_l1passThr5", mymuons, [](const TimedMuon &m) {
    	return m.matchedL1ThrInclusive.find(5) != m.matchedL1ThrInclusive.end();
      });
    oflmuon_variables.push_back(muon_l1passThr5);
    auto muon_l1passThr6 = Monitored::Collection("muon_l1passThr6", mymuons, [](const TimedMuon &m) {
    	return m.matchedL1ThrInclusive.find(6) != m.matchedL1ThrInclusive.end();
      });
    oflmuon_variables.push_back(muon_l1passThr6);
    auto muon_l1passThr7 = Monitored::Collection("muon_l1passThr7", mymuons, [](const TimedMuon &m) {
    	return m.matchedL1ThrInclusive.find(7) != m.matchedL1ThrInclusive.end();
      });
    oflmuon_variables.push_back(muon_l1passThr7);
    auto muon_l1passThr8 = Monitored::Collection("muon_l1passThr8", mymuons, [](const TimedMuon &m) {
    	return m.matchedL1ThrInclusive.find(8) != m.matchedL1ThrInclusive.end();
      });
    oflmuon_variables.push_back(muon_l1passThr8);
    auto muon_l1passThr9 = Monitored::Collection("muon_l1passThr9", mymuons, [](const TimedMuon &m) {
    	return m.matchedL1ThrInclusive.find(9) != m.matchedL1ThrInclusive.end();
      });
    oflmuon_variables.push_back(muon_l1passThr9);
    auto muon_l1passThr10 = Monitored::Collection("muon_l1passThr10", mymuons, [](const TimedMuon &m) {
    	return m.matchedL1ThrInclusive.find(10) != m.matchedL1ThrInclusive.end();
      });
    oflmuon_variables.push_back(muon_l1passThr10);
    auto muon_l1passThr11 = Monitored::Collection("muon_l1passThr11", mymuons, [](const TimedMuon &m) {
    	return m.matchedL1ThrInclusive.find(11) != m.matchedL1ThrInclusive.end();
      });
    oflmuon_variables.push_back(muon_l1passThr11);
    auto muon_l1passThr12 = Monitored::Collection("muon_l1passThr12", mymuons, [](const TimedMuon &m) {
    	return m.matchedL1ThrInclusive.find(12) != m.matchedL1ThrInclusive.end();
      });
    oflmuon_variables.push_back(muon_l1passThr12);
    auto muon_l1passThr13 = Monitored::Collection("muon_l1passThr13", mymuons, [](const TimedMuon &m) {
    	return m.matchedL1ThrInclusive.find(13) != m.matchedL1ThrInclusive.end();
      });
    oflmuon_variables.push_back(muon_l1passThr13);
    auto muon_l1passThr14 = Monitored::Collection("muon_l1passThr14", mymuons, [](const TimedMuon &m) {
    	return m.matchedL1ThrInclusive.find(14) != m.matchedL1ThrInclusive.end();
      });
    oflmuon_variables.push_back(muon_l1passThr14);
    auto muon_l1passThr15 = Monitored::Collection("muon_l1passThr15", mymuons, [](const TimedMuon &m) {
    	return m.matchedL1ThrInclusive.find(15) != m.matchedL1ThrInclusive.end();
      });
    oflmuon_variables.push_back(muon_l1passThr15);
    auto muon_l1passCharge = Monitored::Collection("muon_l1passCharge",mymuons,[](const TimedMuon& m){
    	return m.matchedL1Charge;
      });
    oflmuon_variables.push_back(muon_l1passCharge);
    auto muon_l1passBW3Coin = Monitored::Collection("muon_l1passBW3Coin",mymuons,[](const TimedMuon& m){
    	return m.passBW3Coin;
      });
    oflmuon_variables.push_back(muon_l1passBW3Coin);
    auto muon_l1passBW3CoinVeto = Monitored::Collection("muon_l1passBW3CoinVeto",mymuons,[](const TimedMuon& m){
    	return !m.passBW3Coin;
      });
    oflmuon_variables.push_back(muon_l1passBW3CoinVeto);
    auto muon_l1passInnerCoin = Monitored::Collection("muon_l1passInnerCoin",mymuons,[](const TimedMuon& m){
    	return m.passInnerCoin;
      });
    oflmuon_variables.push_back(muon_l1passInnerCoin);
    auto muon_l1passInnerCoinVeto = Monitored::Collection("muon_l1passInnerCoinVeto",mymuons,[](const TimedMuon& m){
    	return !m.passInnerCoin;
      });
    oflmuon_variables.push_back(muon_l1passInnerCoinVeto);
    auto muon_l1passGoodMF = Monitored::Collection("muon_l1passGoodMF",mymuons,[](const TimedMuon& m){
    	return m.passGoodMF;
      });
    oflmuon_variables.push_back(muon_l1passGoodMF);
    auto muon_l1passBadMF = Monitored::Collection("muon_l1passBadMF",mymuons,[](const TimedMuon& m){
    	return !m.passGoodMF;
      });
    oflmuon_variables.push_back(muon_l1passBadMF);
    auto muon_l1passIsMoreCandInRoI = Monitored::Collection("muon_l1passIsMoreCandInRoI",mymuons,[](const TimedMuon& m){
    	return m.passIsMoreCandInRoI;
      });
    oflmuon_variables.push_back(muon_l1passIsMoreCandInRoI);
    fill(m_packageName, oflmuon_variables);

    ATH_MSG_DEBUG("End filling offline muon-related histograms");
  }
  ///////////////// End filling offline muon-related histograms /////////////////
  fillThresholdPatternHistograms(menuAndRoIs, mymuons, ctx);

  ///////////////// Filling TGC PRD histograms /////////////////
  fillTgcPrdHistograms(mon_bcid, mon_pileup, mon_lb, extpositions,ctx);
  ///////////////// End filling TGC PRD histograms /////////////////


  ///////////////// Filling TGC CoinData histograms /////////////////
  if(m_anaTgcCoin){
    SG::ReadHandle < Muon::TgcCoinDataContainer > tgcCoinPrev(m_TgcCoinDataContainerPrevBCKey, ctx);
    SG::ReadHandle < Muon::TgcCoinDataContainer > tgcCoinCurr(m_TgcCoinDataContainerCurrBCKey, ctx);
    SG::ReadHandle < Muon::TgcCoinDataContainer > tgcCoinNext(m_TgcCoinDataContainerNextBCKey, ctx);
    if (!tgcCoinCurr.isValid() || !tgcCoinNext.isValid() || !tgcCoinPrev.isValid()) {
      ATH_MSG_WARNING("Couldn't get TGC Coin Data");
    }else{
      ATH_MSG_DEBUG("Filling TGC CoinData histograms");
      std::map<int, SG::ReadHandle<Muon::TgcCoinDataContainer> > tgcCoin;
      tgcCoin[-1] = tgcCoinPrev;
      tgcCoin[0]  = tgcCoinCurr;
      tgcCoin[+1] = tgcCoinNext;
      if(!m_TgcCoinDataContainerNextNextBCKey.empty()){
	SG::ReadHandle < Muon::TgcCoinDataContainer > tgcCoinNextNext(m_TgcCoinDataContainerNextNextBCKey, ctx);
	if(tgcCoinNextNext.isValid())tgcCoin[+2] = tgcCoinNextNext;
      }
      std::vector< TgcTrig > tgcTrigMap_SL;
      std::vector< TgcTrig > tgcTrigMap_SL_Endcap;
      std::vector< TgcTrig > tgcTrigMap_SL_Forward;
      std::vector< TgcTrig > tgcTrigMap_HPT_Wire;
      std::vector< TgcTrig > tgcTrigMap_HPT_Endcap_Wire;
      std::vector< TgcTrig > tgcTrigMap_HPT_Forward_Wire;
      std::vector< TgcTrig > tgcTrigMap_HPT_Strip;
      std::vector< TgcTrig > tgcTrigMap_HPT_Endcap_Strip;
      std::vector< TgcTrig > tgcTrigMap_HPT_Forward_Strip;
      std::vector< TgcTrig > tgcTrigMap_LPT_Wire;
      std::vector< TgcTrig > tgcTrigMap_LPT_Endcap_Wire;
      std::vector< TgcTrig > tgcTrigMap_LPT_Forward_Wire;
      std::vector< TgcTrig > tgcTrigMap_LPT_Strip;
      std::vector< TgcTrig > tgcTrigMap_LPT_Endcap_Strip;
      std::vector< TgcTrig > tgcTrigMap_LPT_Forward_Strip;
      std::vector< TgcTrig > tgcTrigMap_EIFI_Wire;
      std::vector< TgcTrig > tgcTrigMap_EIFI_Endcap_Wire;
      std::vector< TgcTrig > tgcTrigMap_EIFI_Forward_Wire;
      std::vector< TgcTrig > tgcTrigMap_EIFI_Strip;
      std::vector< TgcTrig > tgcTrigMap_EIFI_Endcap_Strip;
      std::vector< TgcTrig > tgcTrigMap_EIFI_Forward_Strip;
      std::vector< TgcTrigTile > tgcTrigTileMap;
      std::vector< TgcTrigNsw > tgcTrigNswMap;
      std::vector< TgcTrigRpc > tgcTrigRpcMap;
      std::vector< TgcTrigEifi > tgcTrigEifiMap;
      std::map<TString, TgcTrigNsw > tgcTrigNswMapUnique;
      std::vector<int> multiplicity_endcap(number_of_trigger_sectors_endcap*2+1); // both A and C sides, plus one empty bin
      std::vector<int> multiplicity_forward(number_of_trigger_sectors_forward*2+1); // both A and C sides, plus one empty bin
      std::vector<int> sectors_endcap(number_of_trigger_sectors_endcap*2+1); // both A and C sides, plus one empty bin
      std::vector<int> sectors_forward(number_of_trigger_sectors_forward*2+1); // both A and C sides, plus one empty bin
      std::iota(sectors_endcap.begin(),sectors_endcap.end(),-number_of_trigger_sectors_endcap);
      std::iota(sectors_forward.begin(),sectors_forward.end(),-number_of_trigger_sectors_forward);
      int n_TgcCoin_detElementIsNull = 0;
      int n_TgcCoin_postOutPtrIsNull = 0;
      for (auto thisCoin : tgcCoin) {
	int bunch = thisCoin.first;
	for (const auto tgccnt : *(thisCoin.second)) {
	  for (const auto data : *tgccnt) {
	    if ( data->detectorElementOut() == nullptr ) n_TgcCoin_detElementIsNull++;
	    if ( data->posOutPtr() == nullptr ) n_TgcCoin_postOutPtrIsNull++;

	    int slsector = (data->isForward()) ? ( data->phi() % number_of_trigger_sectors_forward + 1) : ( (data->phi() + 1) % number_of_trigger_sectors_endcap + 1); // translation from the phi index to trigger sector
	    if(!data->isAside()) slsector *= -1;

	    if(data->type() == Muon::TgcCoinData::TYPE_UNKNOWN){ // inner muon detectors (EI/FI/Tile/NSW/RPCBIS78)
	      if (data->isInner() && data->isStrip()) {  // RPC-BIS78
		TgcTrigRpc rpcCoin;
		rpcCoin.slSector = slsector;
		rpcCoin.bcid = (data->inner() >> Muon::TgcCoinData::INNER_RPC_BCID_BITSHIFT) & Muon::TgcCoinData::INNER_RPC_BCID_BIT;
		rpcCoin.bunch = bunch;
		rpcCoin.currBc = (bunch==0);
		rpcCoin.rpcEta = (data->inner() >> Muon::TgcCoinData::INNER_RPC_ETA_BITSHIFT) & Muon::TgcCoinData::INNER_RPC_ETA_BIT;
		rpcCoin.rpcPhi = (data->inner() >> Muon::TgcCoinData::INNER_RPC_PHI_BITSHIFT) & Muon::TgcCoinData::INNER_RPC_PHI_BIT;
		rpcCoin.rpcDEta = (data->inner() >> Muon::TgcCoinData::INNER_RPC_DETA_BITSHIFT) & Muon::TgcCoinData::INNER_RPC_DETA_BIT;
		rpcCoin.rpcDPhi = (data->inner() >> Muon::TgcCoinData::INNER_RPC_DPHI_BITSHIFT) & Muon::TgcCoinData::INNER_RPC_DPHI_BIT;
		tgcTrigRpcMap.push_back(rpcCoin);
	      } else if (data->isInner() && !data->isStrip()) {  // NSW
		TgcTrigNsw nswCoin;
		nswCoin.slSector = slsector;
		nswCoin.slInput = (data->inner() >> Muon::TgcCoinData::INNER_NSW_INPUT_BITSHIFT) & Muon::TgcCoinData::INNER_NSW_INPUT_BIT;
		int boardID = (std::abs(nswCoin.slSector)-1) / 2 + 1; // 1..24 (1..12)
		nswCoin.slInputIndex = (boardID-1) * 6 + nswCoin.slInput;
		nswCoin.isAside = data->isAside();
		nswCoin.isForward = data->isForward();
		nswCoin.bcid = (data->inner() >> Muon::TgcCoinData::INNER_NSW_BCID_BITSHIFT) & Muon::TgcCoinData::INNER_NSW_BCID_BIT;
		nswCoin.bunch = bunch;
		nswCoin.currBc = (bunch==0);
		nswCoin.R = (data->inner() >> Muon::TgcCoinData::INNER_NSW_R_BITSHIFT) & Muon::TgcCoinData::INNER_NSW_R_BIT;
		nswCoin.Phi = (data->inner() >> Muon::TgcCoinData::INNER_NSW_PHI_BITSHIFT) & Muon::TgcCoinData::INNER_NSW_PHI_BIT;
		nswCoin.deltaTheta = (data->inner() >> Muon::TgcCoinData::INNER_NSW_DTHETA_BITSHIFT) & Muon::TgcCoinData::INNER_NSW_DTHETA_BIT;
		if(nswCoin.R!=0 && nswCoin.Phi!=0){
		  tgcTrigNswMap.push_back(nswCoin);
		  TString uniqueinfo = Form("%d:%d:%d:%d:%d:%d:%d:%d",nswCoin.slSector,nswCoin.slInput,nswCoin.slInputIndex,nswCoin.isAside,nswCoin.isForward,nswCoin.R,nswCoin.Phi,nswCoin.deltaTheta);
		  if(tgcTrigNswMapUnique.find(uniqueinfo)==tgcTrigNswMapUnique.end()){
		    nswCoin.bcmask = 0x1 << (1-bunch);
		    tgcTrigNswMapUnique[uniqueinfo] = nswCoin;
		  }else{
		    tgcTrigNswMapUnique[uniqueinfo].bcmask |= 0x1 << (1-bunch);
		  }
		}
	      } else if (!data->isInner() && data->isStrip()) {  // TMDB
		TgcTrigTile tileCoin;
		tileCoin.slSector = slsector;
		tileCoin.bcid = (data->inner() >> Muon::TgcCoinData::INNER_TILE_BCID_BITSHIFT) & Muon::TgcCoinData::INNER_TILE_BCID_BIT;
		tileCoin.bunch = bunch;
		tileCoin.currBc = (bunch==0);
		tileCoin.tmdbDecisions = (data->inner() >> Muon::TgcCoinData::INNER_TILE_MODULE_BITSHIFT) & Muon::TgcCoinData::INNER_TILE_MODULE_BIT;
		if(tileCoin.tmdbDecisions!=0)
		  tgcTrigTileMap.push_back(tileCoin);
	      } else  if (!data->isInner() && !data->isStrip()) {  // EI
		TgcTrigEifi eifiCoin;
		eifiCoin.slSector = slsector;
		eifiCoin.bunch = bunch;
		eifiCoin.currBc = (bunch==0);
		tgcTrigEifiMap.push_back(eifiCoin);
	      }
	    }

	    if ( data->detectorElementOut() == nullptr ||
		 data->posOutPtr() == nullptr )continue; // to avoid FPE
	    TgcTrig tgcTrig;
	    tgcTrig.lb = GetEventInfo(ctx)->lumiBlock();
	    const Amg::Vector3D &posIn = data->globalposIn();
	    tgcTrig.x_In = posIn[0];
	    tgcTrig.y_In = posIn[1];
	    tgcTrig.z_In = posIn[2];
	    const Amg::Vector3D &posOut = data->globalposOut();
	    tgcTrig.x_Out = posOut[0];
	    tgcTrig.y_Out = posOut[1];
	    tgcTrig.z_Out = posOut[2];
	    tgcTrig.eta = posOut.eta();
	    tgcTrig.phi = posOut.phi();
	    tgcTrig.width_In = data->widthIn();
	    tgcTrig.width_Out = data->widthOut();
	    if (data->type() == Muon::TgcCoinData::TYPE_SL) {
	      const Amg::MatrixX &matrix = data->errMat();
	      tgcTrig.width_R = matrix(0, 0);
	      tgcTrig.width_Phi = matrix(1, 1);

	      tgcTrig.muonMatched = 0;
	      for(const auto& ext : extpositions_pivot){
		if(ext.muon->pt() < m_pTCutOnTrigExtrapolation.value() )continue;
		if(data->isAside() && ext.extPos.z()<0)continue;
		if(!data->isAside()&& ext.extPos.z()>0)continue;
		if( Amg::deltaR(posOut,ext.extPos) > m_l1trigMatchWindowPt15 )continue;
		tgcTrig.muonMatched = 1;
		break;
	      }

	      tgcTrig.loosemuonMatched = 0;
	      for (const auto& muon : oflmuons) {
		double max_dr = getMatchingWindow(muon);
		double dr = xAOD::P4Helpers::deltaR(*muon,tgcTrig.eta,tgcTrig.phi,false);
		if( dr > max_dr )continue;
		tgcTrig.loosemuonMatched = 1;
		break;
	      }

	      tgcTrig.isBiased = 0;
	      for(const auto& muon : biasedMuons){
		double max_dr = getMatchingWindow(muon);
		double dr = xAOD::P4Helpers::deltaR(*muon,tgcTrig.eta,tgcTrig.phi,false);
		if( dr > max_dr )continue;
		tgcTrig.isBiased = 1;
		break;
	      }
	      // real1: if biasedMuons.size()==1, reject biased muon
	      // real2: if biasedMuons.size()>=2, use any muons.
	      // fake: just reject biased muons
	      if(biasedMuons.size()>=2) tgcTrig.isBiased = 0;

	    } else {
	      tgcTrig.width_R = 0.;
	      tgcTrig.width_Phi = 0.;
	    }
	    int etaout = 0;
	    int etain = 0;
	    const Identifier tcdidout = data->channelIdOut();
	    if (tcdidout.is_valid()) {
	      etaout = std::abs(int(m_idHelperSvc->tgcIdHelper().stationEta(tcdidout)));
	    }
	    const Identifier tcdidin  = data->channelIdIn();
	    if (tcdidin.is_valid()) {
	      etain  = std::abs(int(m_idHelperSvc->tgcIdHelper().stationEta(tcdidin)));
	    }
	    tgcTrig.etaout = etaout;
	    tgcTrig.etain = etain;
	    tgcTrig.isAside = data->isAside();
	    tgcTrig.isForward = data->isForward();
	    tgcTrig.isStrip = data->isStrip();
	    tgcTrig.isInner = data->isInner();
	    tgcTrig.isPositiveDeltaR = data->isPositiveDeltaR();
	    tgcTrig.type = data->type();
	    tgcTrig.trackletId = data->trackletId();
	    tgcTrig.trackletIdStrip = data->trackletIdStrip();
	    tgcTrig.sector = slsector;
	    tgcTrig.roi = data->roi();
	    tgcTrig.pt = data->pt();
	    tgcTrig.delta = data->delta();
	    tgcTrig.sub = data->sub();
	    tgcTrig.veto = data->veto();
	    tgcTrig.bunch = bunch;
	    tgcTrig.bcid = (GetEventInfo(ctx)->bcid() & 0xF);
	    tgcTrig.inner = data->inner();
	    if( !data->isInner() ){
	      if (data->type() == Muon::TgcCoinData::TYPE_SL && !data->isForward()) {
		tgcTrigMap_SL_Endcap.push_back(tgcTrig);
		tgcTrigMap_SL.push_back(tgcTrig);
		multiplicity_endcap[ slsector + number_of_trigger_sectors_endcap ]++;
	      }else if (data->type() == Muon::TgcCoinData::TYPE_SL && data->isForward()) {
		tgcTrigMap_SL_Forward.push_back(tgcTrig);
		tgcTrigMap_SL.push_back(tgcTrig);
		multiplicity_forward[ slsector + number_of_trigger_sectors_forward ]++;
	      }else if(data->type() == Muon::TgcCoinData::TYPE_HIPT && !data->isForward()){
		if(tgcTrig.isStrip){
		  tgcTrigMap_HPT_Endcap_Strip.push_back(tgcTrig);
		  tgcTrigMap_HPT_Strip.push_back(tgcTrig);
		}else{
		  tgcTrigMap_HPT_Endcap_Wire.push_back(tgcTrig);
		  tgcTrigMap_HPT_Wire.push_back(tgcTrig);
		}
	      }else if(data->type() == Muon::TgcCoinData::TYPE_HIPT && data->isForward()){
		if(tgcTrig.isStrip){
		  tgcTrigMap_HPT_Forward_Strip.push_back(tgcTrig);
		  tgcTrigMap_HPT_Strip.push_back(tgcTrig);
		}else{
		  tgcTrigMap_HPT_Forward_Wire.push_back(tgcTrig);
		  tgcTrigMap_HPT_Wire.push_back(tgcTrig);
		}
	      }else if(data->type() == Muon::TgcCoinData::TYPE_TRACKLET && !data->isForward()){
		if(tgcTrig.isStrip){
		  tgcTrigMap_LPT_Endcap_Strip.push_back(tgcTrig);
		  tgcTrigMap_LPT_Strip.push_back(tgcTrig);
		}else{
		  tgcTrigMap_LPT_Endcap_Wire.push_back(tgcTrig);
		  tgcTrigMap_LPT_Wire.push_back(tgcTrig);
		}
	      }else if(data->type() == Muon::TgcCoinData::TYPE_TRACKLET && data->isForward()){
		if(tgcTrig.isStrip){
		  tgcTrigMap_LPT_Forward_Strip.push_back(tgcTrig);
		  tgcTrigMap_LPT_Strip.push_back(tgcTrig);
		}else{
		  tgcTrigMap_LPT_Forward_Wire.push_back(tgcTrig);
		  tgcTrigMap_LPT_Wire.push_back(tgcTrig);
		}
	      }else if(data->type() == Muon::TgcCoinData::TYPE_TRACKLET_EIFI && !data->isForward()){
		if(tgcTrig.isStrip){
		  tgcTrigMap_EIFI_Endcap_Strip.push_back(tgcTrig);
		  tgcTrigMap_EIFI_Strip.push_back(tgcTrig);
		}else{
		  tgcTrigMap_EIFI_Endcap_Wire.push_back(tgcTrig);
		  tgcTrigMap_EIFI_Wire.push_back(tgcTrig);
		}
	      }else if(data->type() == Muon::TgcCoinData::TYPE_TRACKLET_EIFI && data->isForward()){
		if(tgcTrig.isStrip){
		  tgcTrigMap_EIFI_Forward_Strip.push_back(tgcTrig);
		  tgcTrigMap_EIFI_Strip.push_back(tgcTrig);
		}else{
		  tgcTrigMap_EIFI_Forward_Wire.push_back(tgcTrig);
		  tgcTrigMap_EIFI_Wire.push_back(tgcTrig);
		}
	      }
	    }else{ // inner coincidence, i.e. EI/FI/Tile/BIS78/NSW

	    }
	  }
	}
      }
      

      for(auto& sl : tgcTrigMap_SL){
	if( sl.bunch != 0 )continue;
	for(auto& inner : tgcTrigRpcMap){
	  if( sl.isForward == 1 )break;
	  if( sl.sector != inner.slSector )continue;
	  inner.roiEta = sl.eta;
	  inner.roiPhi = sl.phi;
	  inner.roiNum = sl.roi;
	  inner.deltaBcid = (sl.bunch==0 && sl.muonMatched==1 && sl.isBiased==0) ? (inner.bcid - sl.bcid) : -999;
	  inner.deltaTiming = (sl.bunch==0 && sl.muonMatched==1 && sl.isBiased==0) ? (inner.bunch - sl.bunch) : -999;
	  inner.goodBcid0  = inner.deltaBcid==0;
	  inner.goodBcid1 = (std::abs(inner.deltaBcid)<=1 || (16-std::abs(inner.deltaBcid))<=1);
	  inner.goodBcid2 = (std::abs(inner.deltaBcid)<=2 || (16-std::abs(inner.deltaBcid))<=2);
	  inner.goodTiming = (inner.bunch==sl.bunch && sl.bunch==0 && sl.muonMatched==1 && sl.isBiased==0);
	  sl.rpc.push_back(&inner);
	}
	for(auto& inner : tgcTrigNswMap){
	  if( sl.sector != inner.slSector )continue;
	  if( sl.isForward != inner.isForward )continue;
	  inner.deltaR = inner.R - std::abs(getNswRindexFromEta(sl.eta));
	  inner.roiEta = sl.eta;
	  inner.roiPhi = sl.phi;
	  inner.roiNum = sl.roi;
	  if( std::abs(inner.deltaR) < m_NswDeltaRCut ){
	    inner.deltaBcid = (sl.bunch==0 && sl.muonMatched==1 && sl.isBiased==0) ? (inner.bcid - sl.bcid) : -999;
	    inner.deltaTiming = (sl.bunch==0 && sl.muonMatched==1 && sl.isBiased==0) ? (inner.bunch - sl.bunch) : -999;
	    inner.goodBcid0  = inner.deltaBcid==0;
	    inner.goodBcid1 = (std::abs(inner.deltaBcid)<=1 || (16-std::abs(inner.deltaBcid))<=1);
	    inner.goodBcid2 = (std::abs(inner.deltaBcid)<=2 || (16-std::abs(inner.deltaBcid))<=2);
	    inner.goodTiming = (inner.bunch==sl.bunch && sl.bunch==0 && sl.muonMatched==1 && sl.isBiased==0);
	    sl.nsw.push_back(&inner);
	  }
	}
	for(auto& inner : tgcTrigNswMapUnique){
	  if( sl.sector != inner.second.slSector )continue;
	  if( sl.isForward != inner.second.isForward )continue;
	  inner.second.deltaR = inner.second.R - std::abs(getNswRindexFromEta(sl.eta));
	  inner.second.roiEta = sl.eta;
	  inner.second.roiPhi = sl.phi;
	  inner.second.roiNum = sl.roi;
	  if( std::abs(inner.second.deltaR) < m_NswDeltaRCut ){
	    inner.second.deltaBcid = (sl.bunch==0 && sl.muonMatched==1 && sl.isBiased==0) ? (inner.second.bcid - sl.bcid) : -999;
	    inner.second.deltaTiming = (sl.bunch==0 && sl.muonMatched==1 && sl.isBiased==0) ? (inner.second.bunch - sl.bunch) : -999;
	    inner.second.goodBcid0  = inner.second.deltaBcid==0;
	    inner.second.goodBcid1 = (std::abs(inner.second.deltaBcid)<=1 || (16-std::abs(inner.second.deltaBcid))<=1);
	    inner.second.goodBcid2 = (std::abs(inner.second.deltaBcid)<=2 || (16-std::abs(inner.second.deltaBcid))<=2);
	    inner.second.goodTiming = (inner.second.bunch==sl.bunch && sl.bunch==0 && sl.muonMatched==1 && sl.isBiased==0);
	    sl.nsw_unique.push_back(&inner.second);
	  }
	}
	for(auto& inner : tgcTrigTileMap){
	  if( sl.isForward == 1 )break;
	  if( sl.sector != inner.slSector )continue;
	  inner.roiEta = sl.eta;
	  inner.roiPhi = sl.phi;
	  inner.roiNum = sl.roi;
	  inner.deltaBcid = (sl.bunch==0 && sl.muonMatched==1 && sl.isBiased==0) ? (inner.bcid - sl.bcid) : -999;
	  inner.deltaTiming = (sl.bunch==0 && sl.muonMatched==1 && sl.isBiased==0) ? (inner.bunch - sl.bunch) : -999;
	  inner.goodBcid0  = inner.deltaBcid==0;
	  inner.goodBcid1 = (std::abs(inner.deltaBcid)<=1 || (16-std::abs(inner.deltaBcid))<=1);
	  inner.goodBcid2 = (std::abs(inner.deltaBcid)<=2 || (16-std::abs(inner.deltaBcid))<=2);
	  inner.goodTiming = (inner.bunch==sl.bunch && sl.bunch==0 && sl.muonMatched==1 && sl.isBiased==0);
	  sl.tile.push_back(&inner);
	}
	for(auto& inner : tgcTrigEifiMap){
	  if( sl.isForward == 1 )break;
	  if( sl.sector != inner.slSector )continue;
	  inner.roiEta = sl.eta;
	  inner.roiPhi = sl.phi;
	  inner.roiNum = sl.roi;
	  inner.deltaTiming = (sl.bunch==0 && sl.muonMatched==1 && sl.isBiased==0) ? (inner.bunch - sl.bunch) : -999;
	  inner.goodTiming = (inner.bunch==sl.bunch && sl.bunch==0 && sl.muonMatched==1 && sl.isBiased==0);
	  sl.eifi.push_back(&inner);
	}
      }

      std::vector< TgcTrigTile > tgcTrigTileMap_allmods;
      for(auto& inner : tgcTrigTileMap){
	if(inner.roiNum<0)continue;
	for(int i = 0 ; i < 12 ; i++){
	  TgcTrigTile inner2 = inner;
	  if((inner.tmdbDecisions>>i) & 0x1){
	    inner2.tmdbDecisions = (i+3)%3 + 1;
	    tgcTrigTileMap_allmods.push_back(inner2);
	  }
	}
      }



      MonVariables  tgcCoin_variables;
      tgcCoin_variables.push_back(mon_bcid);
      tgcCoin_variables.push_back(mon_pileup);
      tgcCoin_variables.push_back(mon_lb);
      
      auto mon_nTgcCoin_detElementIsNull = Monitored::Scalar<int>("nTgcCoinDetElementIsNull", n_TgcCoin_detElementIsNull);
      auto mon_nTgcCoin_postOutPtrIsNull = Monitored::Scalar<int>("nTgcCoinPostOutPtrIsNull", n_TgcCoin_postOutPtrIsNull);
      auto mon_sectors_endcap = Monitored::Collection("trigger_sectors_endcap", sectors_endcap);
      auto mon_sectors_forward = Monitored::Collection("trigger_sectors_forward", sectors_forward);
      auto mon_multiplicity_endcap = Monitored::Collection("trigger_multiplicity_endcap", multiplicity_endcap);
      auto mon_multiplicity_forward = Monitored::Collection("trigger_multiplicity_forward", multiplicity_forward);
      tgcCoin_variables.emplace_back(mon_nTgcCoin_detElementIsNull);
      tgcCoin_variables.emplace_back(mon_nTgcCoin_postOutPtrIsNull);
      tgcCoin_variables.emplace_back(mon_sectors_endcap);
      tgcCoin_variables.emplace_back(mon_sectors_forward);
      tgcCoin_variables.emplace_back(mon_multiplicity_endcap);
      tgcCoin_variables.emplace_back(mon_multiplicity_forward);


      std::vector<Monitored::ObjectsCollection<std::vector<TgcTrig>, double>> vo_coin;
      vo_coin.reserve(38 * 21);

      fillTgcCoin("SL",tgcTrigMap_SL,vo_coin,tgcCoin_variables);
      fillTgcCoin("SL_Endcap",tgcTrigMap_SL_Endcap,vo_coin,tgcCoin_variables);
      fillTgcCoin("SL_Forward",tgcTrigMap_SL_Forward,vo_coin,tgcCoin_variables);
      fillTgcCoin("HPT_Wire",tgcTrigMap_HPT_Wire,vo_coin,tgcCoin_variables);
      fillTgcCoin("HPT_Endcap_Wire",tgcTrigMap_HPT_Endcap_Wire,vo_coin,tgcCoin_variables);
      fillTgcCoin("HPT_Forward_Wire",tgcTrigMap_HPT_Forward_Wire,vo_coin,tgcCoin_variables);
      fillTgcCoin("HPT_Strip",tgcTrigMap_HPT_Strip,vo_coin,tgcCoin_variables);
      fillTgcCoin("HPT_Endcap_Strip",tgcTrigMap_HPT_Endcap_Strip,vo_coin,tgcCoin_variables);
      fillTgcCoin("HPT_Forward_Strip",tgcTrigMap_HPT_Forward_Strip,vo_coin,tgcCoin_variables);
      fillTgcCoin("LPT_Wire",tgcTrigMap_LPT_Wire,vo_coin,tgcCoin_variables);
      fillTgcCoin("LPT_Endcap_Wire",tgcTrigMap_LPT_Endcap_Wire,vo_coin,tgcCoin_variables);
      fillTgcCoin("LPT_Forward_Wire",tgcTrigMap_LPT_Forward_Wire,vo_coin,tgcCoin_variables);
      fillTgcCoin("LPT_Strip",tgcTrigMap_LPT_Strip,vo_coin,tgcCoin_variables);
      fillTgcCoin("LPT_Endcap_Strip",tgcTrigMap_LPT_Endcap_Strip,vo_coin,tgcCoin_variables);
      fillTgcCoin("LPT_Forward_Strip",tgcTrigMap_LPT_Forward_Strip,vo_coin,tgcCoin_variables);
      fillTgcCoin("EIFI_Wire",tgcTrigMap_EIFI_Wire,vo_coin,tgcCoin_variables);
      fillTgcCoin("EIFI_Endcap_Wire",tgcTrigMap_EIFI_Endcap_Wire,vo_coin,tgcCoin_variables);
      fillTgcCoin("EIFI_Forward_Wire",tgcTrigMap_EIFI_Forward_Wire,vo_coin,tgcCoin_variables);
      fillTgcCoin("EIFI_Strip",tgcTrigMap_EIFI_Strip,vo_coin,tgcCoin_variables);
      fillTgcCoin("EIFI_Endcap_Strip",tgcTrigMap_EIFI_Endcap_Strip,vo_coin,tgcCoin_variables);
      fillTgcCoin("EIFI_Forward_Strip",tgcTrigMap_EIFI_Forward_Strip,vo_coin,tgcCoin_variables);
      
      std::vector<Monitored::ObjectsCollection<std::vector<ExtTrigInfo>, double>> vo_exttriginfo;
      vo_exttriginfo.reserve(13 * 5);
      std::vector<ExtTrigInfo> extTrigInfo_SL;
      std::vector<ExtTrigInfo> extTrigInfo_HPT_Wire;
      std::vector<ExtTrigInfo> extTrigInfo_HPT_Strip;
      std::vector<ExtTrigInfo> extTrigInfo_LPT_Wire;
      std::vector<ExtTrigInfo> extTrigInfo_LPT_Strip;
      fillTgcCoinEff("SL",tgcTrigMap_SL,extpositions_pivot,extTrigInfo_SL,vo_exttriginfo,tgcCoin_variables);
      fillTgcCoinEff("HPT_Wire",tgcTrigMap_HPT_Wire,extpositions_pivot,extTrigInfo_HPT_Wire,vo_exttriginfo,tgcCoin_variables);
      fillTgcCoinEff("HPT_Strip",tgcTrigMap_HPT_Strip,extpositions_pivot,extTrigInfo_HPT_Strip,vo_exttriginfo,tgcCoin_variables);
      fillTgcCoinEff("LPT_Wire",tgcTrigMap_LPT_Wire,extpositions_pivot,extTrigInfo_LPT_Wire,vo_exttriginfo,tgcCoin_variables);
      fillTgcCoinEff("LPT_Strip",tgcTrigMap_LPT_Strip,extpositions_pivot,extTrigInfo_LPT_Strip,vo_exttriginfo,tgcCoin_variables);

      // TGC
      auto coin_inner_tgc_roi=Monitored::Collection("coin_inner_tgc_roi",tgcTrigMap_SL,[](const TgcTrig&m){
	  return m.roi;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_roi);
      auto coin_inner_tgc_sector=Monitored::Collection("coin_inner_tgc_sector",tgcTrigMap_SL,[](const TgcTrig&m){
	  return (m.bunch==0 && m.muonMatched==1 && m.isBiased==0) ? (m.sector) : -999;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_sector);
      auto coin_inner_tgc_fake_sector=Monitored::Collection("coin_inner_tgc_fake_sector",tgcTrigMap_SL,[](const TgcTrig&m){
	  return (m.bunch==0 && m.muonMatched==0 && m.loosemuonMatched==0 && m.isBiased==0) ? (m.sector) : -999;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_fake_sector);

      auto coin_inner_tgc_eta=Monitored::Collection("coin_inner_tgc_eta",tgcTrigMap_SL,[](const TgcTrig&m){
	  return (m.bunch==0 && m.muonMatched==1 && m.isBiased==0) ? (m.eta) : -999;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_eta);
      auto coin_inner_tgc_phi=Monitored::Collection("coin_inner_tgc_phi",tgcTrigMap_SL,[](const TgcTrig&m){
	  return (m.bunch==0 && m.muonMatched==1 && m.isBiased==0) ? (m.phi) : -999;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_phi);

      auto coin_inner_tgc_fake_eta=Monitored::Collection("coin_inner_tgc_fake_eta",tgcTrigMap_SL,[](const TgcTrig&m){
	  return (m.bunch==0 && m.muonMatched==0 && m.loosemuonMatched==0 && m.isBiased==0) ? (m.eta) : -999;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_fake_eta);
      auto coin_inner_tgc_fake_phi=Monitored::Collection("coin_inner_tgc_fake_phi",tgcTrigMap_SL,[](const TgcTrig&m){
	  return (m.bunch==0 && m.muonMatched==0 && m.loosemuonMatched==0 && m.isBiased==0) ? (m.phi) : -999;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_fake_phi);

      auto coin_inner_tgc_forward=Monitored::Collection("coin_inner_tgc_forward",tgcTrigMap_SL,[](const TgcTrig&m){
	  return m.isForward==1;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_forward);
      auto coin_inner_tgc_endcap=Monitored::Collection("coin_inner_tgc_endcap",tgcTrigMap_SL,[](const TgcTrig&m){
	  return m.isForward==0;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_endcap);
      auto coin_inner_tgc_etaupto1p3=Monitored::Collection("coin_inner_tgc_etaupto1p3",tgcTrigMap_SL,[](const TgcTrig&m){
	  return std::abs(m.eta) < 1.3;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_etaupto1p3);
      auto coin_inner_tgc_etafrom1p3_endcap=Monitored::Collection("coin_inner_tgc_etafrom1p3_endcap",tgcTrigMap_SL,[](const TgcTrig&m){
	  return std::abs(m.eta) > 1.3 && m.isForward==0;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_etafrom1p3_endcap);

      auto coin_inner_tgc_coinflagEifi=Monitored::Collection("coin_inner_tgc_coinflagEifi",tgcTrigMap_SL,[](const TgcTrig&m){
	  return (((m.pt>>CoinFlagEI)&0x1)!=0) ? 1.0 : 0.0;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_coinflagEifi);
      auto coin_inner_tgc_coinflagTile=Monitored::Collection("coin_inner_tgc_coinflagTile",tgcTrigMap_SL,[](const TgcTrig&m){
	  return (((m.pt>>CoinFlagTile)&0x1)!=0) ? 1.0 : 0.0;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_coinflagTile);
      auto coin_inner_tgc_coinflagRpc=Monitored::Collection("coin_inner_tgc_coinflagRpc",tgcTrigMap_SL,[](const TgcTrig&m){
	  return (((m.pt>>CoinFlagRPC)&0x1)!=0) ? 1.0 : 0.0;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_coinflagRpc);
      auto coin_inner_tgc_coinflagNsw=Monitored::Collection("coin_inner_tgc_coinflagNsw",tgcTrigMap_SL,[](const TgcTrig&m){
	  return (((m.pt>>CoinFlagNSW)&0x1)!=0) ? 1.0 : 0.0;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_coinflagNsw);
      auto coin_inner_tgc_coinflagC=Monitored::Collection("coin_inner_tgc_coinflagC",tgcTrigMap_SL,[](const TgcTrig&m){
	  return (((m.pt>>CoinFlagC)&0x1)!=0) ? 1.0 : 0.0;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_coinflagC);

      // RPC
      auto coin_inner_tgc_anyBcRpc=Monitored::Collection("coin_inner_tgc_anyBcRpc",tgcTrigMap_SL,[](const TgcTrig&m) -> double{
	  return (m.rpc.size()>0);
	});
      tgcCoin_variables.push_back(coin_inner_tgc_anyBcRpc);
      auto coin_inner_tgc_prevBcRpc=Monitored::Collection("coin_inner_tgc_prevBcRpc",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.rpc){
	    if(inner->bunch == -1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_prevBcRpc);
      auto coin_inner_tgc_currBcRpc=Monitored::Collection("coin_inner_tgc_currBcRpc",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.rpc){
	    if(inner->bunch == 0) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_currBcRpc);
      auto coin_inner_tgc_currBcRpc_goodBcid0=Monitored::Collection("coin_inner_tgc_currBcRpc_goodBcid0",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.rpc){
	    if(inner->bunch == 0 && inner->goodBcid0 == 1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_currBcRpc_goodBcid0);
      auto coin_inner_tgc_currBcRpc_goodBcid1=Monitored::Collection("coin_inner_tgc_currBcRpc_goodBcid1",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.rpc){
	    if(inner->bunch == 0 && inner->goodBcid1 == 1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_currBcRpc_goodBcid1);
      auto coin_inner_tgc_currBcRpc_goodBcid2=Monitored::Collection("coin_inner_tgc_currBcRpc_goodBcid2",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.rpc){
	    if(inner->bunch == 0 && inner->goodBcid2 == 1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_currBcRpc_goodBcid2);
      auto coin_inner_tgc_nextBcRpc=Monitored::Collection("coin_inner_tgc_nextBcRpc",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.rpc){
	    if(inner->bunch == 1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_nextBcRpc);
      auto coin_inner_tgc_nextnextBcRpc=Monitored::Collection("coin_inner_tgc_nextnextBcRpc",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.rpc){
	    if(inner->bunch == 2) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_nextnextBcRpc);

      auto coin_inner_tgc_prevBcRpc_goodBcid0=Monitored::Collection("coin_inner_tgc_prevBcRpc_goodBcid0",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.rpc){
	    if(inner->bunch == -1 && inner->goodBcid0 == 1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_prevBcRpc_goodBcid0);
      auto coin_inner_tgc_nextBcRpc_goodBcid0=Monitored::Collection("coin_inner_tgc_nextBcRpc_goodBcid0",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.rpc){
	    if(inner->bunch == 1 && inner->goodBcid0 == 1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_nextBcRpc_goodBcid0);
      auto coin_inner_tgc_nextnextBcRpc_goodBcid0=Monitored::Collection("coin_inner_tgc_nextnextBcRpc_goodBcid0",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.rpc){
	    if(inner->bunch == 2 && inner->goodBcid0 == 1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_nextnextBcRpc_goodBcid0);

      // NSW
      auto coin_inner_tgc_anyBcNsw=Monitored::Collection("coin_inner_tgc_anyBcNsw",tgcTrigMap_SL,[](const TgcTrig&m) -> double{
	  return (m.nsw.size()>0);
	});
      tgcCoin_variables.push_back(coin_inner_tgc_anyBcNsw);
      auto coin_inner_tgc_prevBcNsw=Monitored::Collection("coin_inner_tgc_prevBcNsw",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.nsw){
	    if(inner->bunch == -1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_prevBcNsw);
      auto coin_inner_tgc_currBcNsw=Monitored::Collection("coin_inner_tgc_currBcNsw",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.nsw){
	    if(inner->bunch == 0) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_currBcNsw);
      auto coin_inner_tgc_currBcNsw_goodBcid0=Monitored::Collection("coin_inner_tgc_currBcNsw_goodBcid0",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.nsw){
	    if(inner->bunch == 0 && inner->goodBcid0 == 1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_currBcNsw_goodBcid0);
      auto coin_inner_tgc_currBcNsw_goodBcid1=Monitored::Collection("coin_inner_tgc_currBcNsw_goodBcid1",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.nsw){
	    if(inner->bunch == 0 && inner->goodBcid1 == 1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_currBcNsw_goodBcid1);
      auto coin_inner_tgc_currBcNsw_goodBcid2=Monitored::Collection("coin_inner_tgc_currBcNsw_goodBcid2",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.nsw){
	    if(inner->bunch == 0 && inner->goodBcid2 == 1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_currBcNsw_goodBcid2);
      auto coin_inner_tgc_nextBcNsw=Monitored::Collection("coin_inner_tgc_nextBcNsw",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.nsw){
	    if(inner->bunch == 1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_nextBcNsw);
      auto coin_inner_tgc_nextnextBcNsw=Monitored::Collection("coin_inner_tgc_nextnextBcNsw",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.nsw){
	    if(inner->bunch == 2) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_nextnextBcNsw);

      auto coin_inner_tgc_prevBcNsw_goodBcid0=Monitored::Collection("coin_inner_tgc_prevBcNsw_goodBcid0",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.nsw){
	    if(inner->bunch == -1 && inner->goodBcid0 == 1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_prevBcNsw_goodBcid0);
      auto coin_inner_tgc_nextBcNsw_goodBcid0=Monitored::Collection("coin_inner_tgc_nextBcNsw_goodBcid0",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.nsw){
	    if(inner->bunch == 1 && inner->goodBcid0 == 1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_nextBcNsw_goodBcid0);
      auto coin_inner_tgc_nextnextBcNsw_goodBcid0=Monitored::Collection("coin_inner_tgc_nextnextBcNsw_goodBcid0",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.nsw){
	    if(inner->bunch == 2 && inner->goodBcid0 == 1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_nextnextBcNsw_goodBcid0);
      auto coin_inner_tgc_Nsw_bcmask1=Monitored::Collection("coin_inner_tgc_Nsw_bcmask1",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.nsw_unique){
	    if(inner->bcmask==1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_Nsw_bcmask1);
      auto coin_inner_tgc_Nsw_bcmask2=Monitored::Collection("coin_inner_tgc_Nsw_bcmask2",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.nsw_unique){
	    if(inner->bcmask==2) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_Nsw_bcmask2);
      auto coin_inner_tgc_Nsw_bcmask3=Monitored::Collection("coin_inner_tgc_Nsw_bcmask3",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.nsw_unique){
	    if(inner->bcmask==3) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_Nsw_bcmask3);
      auto coin_inner_tgc_Nsw_bcmask4=Monitored::Collection("coin_inner_tgc_Nsw_bcmask4",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.nsw_unique){
	    if(inner->bcmask==4) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_Nsw_bcmask4);
      auto coin_inner_tgc_Nsw_bcmask5=Monitored::Collection("coin_inner_tgc_Nsw_bcmask5",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.nsw_unique){
	    if(inner->bcmask==5) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_Nsw_bcmask5);
      auto coin_inner_tgc_Nsw_bcmask6=Monitored::Collection("coin_inner_tgc_Nsw_bcmask6",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.nsw_unique){
	    if(inner->bcmask==6) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_Nsw_bcmask6);
      auto coin_inner_tgc_Nsw_bcmask7=Monitored::Collection("coin_inner_tgc_Nsw_bcmask7",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.nsw_unique){
	    if(inner->bcmask==7) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_Nsw_bcmask7);

      // Tile
      auto coin_inner_tgc_anyBcTile=Monitored::Collection("coin_inner_tgc_anyBcTile",tgcTrigMap_SL,[](const TgcTrig&m) -> double{
	  return (m.tile.size()>0);
	});
      tgcCoin_variables.push_back(coin_inner_tgc_anyBcTile);
      auto coin_inner_tgc_prevBcTile=Monitored::Collection("coin_inner_tgc_prevBcTile",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.tile){
	    if(inner->bunch == -1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_prevBcTile);
      auto coin_inner_tgc_currBcTile=Monitored::Collection("coin_inner_tgc_currBcTile",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.tile){
	    if(inner->bunch == 0) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_currBcTile);
      auto coin_inner_tgc_currBcTile_goodBcid0=Monitored::Collection("coin_inner_tgc_currBcTile_goodBcid0",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.tile){
	    if(inner->bunch == 0 && inner->goodBcid0 == 1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_currBcTile_goodBcid0);
      auto coin_inner_tgc_currBcTile_goodBcid1=Monitored::Collection("coin_inner_tgc_currBcTile_goodBcid1",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.tile){
	    if(inner->bunch == 0 && inner->goodBcid1 == 1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_currBcTile_goodBcid1);
      auto coin_inner_tgc_currBcTile_goodBcid2=Monitored::Collection("coin_inner_tgc_currBcTile_goodBcid2",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.tile){
	    if(inner->bunch == 0 && inner->goodBcid2 == 1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_currBcTile_goodBcid2);
      auto coin_inner_tgc_nextBcTile=Monitored::Collection("coin_inner_tgc_nextBcTile",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.tile){
	    if(inner->bunch == 1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_nextBcTile);
      auto coin_inner_tgc_nextnextBcTile=Monitored::Collection("coin_inner_tgc_nextnextBcTile",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto inner : m.tile){
	    if(inner->bunch == 2) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_nextnextBcTile);

      auto coin_inner_tgc_prevBcTile_goodBcid0=Monitored::Collection("coin_inner_tgc_prevBcTile_goodBcid0",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.tile){
	    if(inner->bunch == -1 && inner->goodBcid0 == 1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_prevBcTile_goodBcid0);
      auto coin_inner_tgc_nextBcTile_goodBcid0=Monitored::Collection("coin_inner_tgc_nextBcTile_goodBcid0",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.tile){
	    if(inner->bunch == 1 && inner->goodBcid0 == 1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_nextBcTile_goodBcid0);
      auto coin_inner_tgc_nextnextBcTile_goodBcid0=Monitored::Collection("coin_inner_tgc_nextnextBcTile_goodBcid0",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto inner : m.tile){
	    if(inner->bunch == 2 && inner->goodBcid0 == 1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_nextnextBcTile_goodBcid0);

      // EIFI
      auto coin_inner_tgc_anyBcEifi=Monitored::Collection("coin_inner_tgc_anyBcEifi",tgcTrigMap_SL,[](const TgcTrig&m) -> double{
	  return (m.eifi.size()>0);
	});
      tgcCoin_variables.push_back(coin_inner_tgc_anyBcEifi);
      auto coin_inner_tgc_prevBcEifi=Monitored::Collection("coin_inner_tgc_prevBcEifi",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.eifi){
	    if(inner->bunch == -1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_prevBcEifi);
      auto coin_inner_tgc_currBcEifi=Monitored::Collection("coin_inner_tgc_currBcEifi",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.eifi){
	    if(inner->bunch == 0) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_currBcEifi);
      auto coin_inner_tgc_nextBcEifi=Monitored::Collection("coin_inner_tgc_nextBcEifi",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto& inner : m.eifi){
	    if(inner->bunch == 1) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_nextBcEifi);
      auto coin_inner_tgc_nextnextBcEifi=Monitored::Collection("coin_inner_tgc_nextnextBcEifi",tgcTrigMap_SL,[](const TgcTrig&m){
	  for(const auto inner : m.eifi){
	    if(inner->bunch == 2) return 1.;
	  }
	  return 0.;
	});
      tgcCoin_variables.push_back(coin_inner_tgc_nextnextBcEifi);


      // RPC BIS78 inner coincidence
      auto coin_inner_rpc_slSector=Monitored::Collection("coin_inner_rpc_slSector",tgcTrigRpcMap,[](const TgcTrigRpc&m){
	  return m.slSector;
	});
      tgcCoin_variables.push_back(coin_inner_rpc_slSector);
      auto coin_inner_rpc_slSector_goodTiming=Monitored::Collection("coin_inner_rpc_slSector_goodTiming",tgcTrigRpcMap,[](const TgcTrigRpc&m){
	  return (m.goodTiming) ? m.slSector : -999;
	});
      tgcCoin_variables.push_back(coin_inner_rpc_slSector_goodTiming);
      auto coin_inner_rpc_roiEta=Monitored::Collection("coin_inner_rpc_roiEta",tgcTrigRpcMap,[](const TgcTrigRpc&m){
	  return m.roiEta;
	});
      tgcCoin_variables.push_back(coin_inner_rpc_roiEta);
      auto coin_inner_rpc_roiPhi=Monitored::Collection("coin_inner_rpc_roiPhi",tgcTrigRpcMap,[](const TgcTrigRpc&m){
	  return m.roiPhi;
	});
      tgcCoin_variables.push_back(coin_inner_rpc_roiPhi);
      auto coin_inner_rpc_roiNum=Monitored::Collection("coin_inner_rpc_roiNum",tgcTrigRpcMap,[](const TgcTrigRpc&m){
	  return m.roiNum;
	});
      tgcCoin_variables.push_back(coin_inner_rpc_roiNum);
      auto coin_inner_rpc_deltaBcid=Monitored::Collection("coin_inner_rpc_deltaBcid",tgcTrigRpcMap,[](const TgcTrigRpc&m){
	  return m.deltaBcid;
	});
      tgcCoin_variables.push_back(coin_inner_rpc_deltaBcid);
      auto coin_inner_rpc_deltaTiming=Monitored::Collection("coin_inner_rpc_deltaTiming",tgcTrigRpcMap,[](const TgcTrigRpc&m){
	  return m.deltaTiming;
	});
      tgcCoin_variables.push_back(coin_inner_rpc_deltaTiming);
      auto coin_inner_rpc_rpcEta=Monitored::Collection("coin_inner_rpc_rpcEta",tgcTrigRpcMap,[](const TgcTrigRpc&m){
	  return m.rpcEta;
	});
      tgcCoin_variables.push_back(coin_inner_rpc_rpcEta);
      auto coin_inner_rpc_rpcPhi=Monitored::Collection("coin_inner_rpc_rpcPhi",tgcTrigRpcMap,[](const TgcTrigRpc&m){
	  return m.rpcPhi;
	});
      tgcCoin_variables.push_back(coin_inner_rpc_rpcPhi);
      auto coin_inner_rpc_rpcDEta=Monitored::Collection("coin_inner_rpc_rpcDEta",tgcTrigRpcMap,[](const TgcTrigRpc&m){
	  return m.rpcDEta;
	});
      tgcCoin_variables.push_back(coin_inner_rpc_rpcDEta);
      auto coin_inner_rpc_rpcDPhi=Monitored::Collection("coin_inner_rpc_rpcDPhi",tgcTrigRpcMap,[](const TgcTrigRpc&m){
	  return m.rpcDPhi;
	});
      tgcCoin_variables.push_back(coin_inner_rpc_rpcDPhi);
      auto coin_inner_rpc_currBc=Monitored::Collection("coin_inner_rpc_currBc",tgcTrigRpcMap,[](const TgcTrigRpc&m){
	  return m.currBc;
	});
      tgcCoin_variables.push_back(coin_inner_rpc_currBc);
      auto coin_inner_rpc_goodBcid0=Monitored::Collection("coin_inner_rpc_goodBcid0",tgcTrigRpcMap,[](const TgcTrigRpc&m){
	  return m.goodBcid0;
	});
      tgcCoin_variables.push_back(coin_inner_rpc_goodBcid0);
      auto coin_inner_rpc_goodBcid1=Monitored::Collection("coin_inner_rpc_goodBcid1",tgcTrigRpcMap,[](const TgcTrigRpc&m){
	  return m.goodBcid1;
	});
      tgcCoin_variables.push_back(coin_inner_rpc_goodBcid1);
      auto coin_inner_rpc_goodBcid2=Monitored::Collection("coin_inner_rpc_goodBcid2",tgcTrigRpcMap,[](const TgcTrigRpc&m){
	  return m.goodBcid2;
	});
      tgcCoin_variables.push_back(coin_inner_rpc_goodBcid2);
      auto coin_inner_rpc_goodTiming=Monitored::Collection("coin_inner_rpc_goodTiming",tgcTrigRpcMap,[](const TgcTrigRpc&m){
	  return m.goodTiming;
	});
      tgcCoin_variables.push_back(coin_inner_rpc_goodTiming);


      // NSW inner coincidence
      auto coin_inner_nsw_deltaR=Monitored::Collection("coin_inner_nsw_deltaR",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return m.deltaR;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_deltaR);
      auto coin_inner_nsw_slSector=Monitored::Collection("coin_inner_nsw_slSector",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return m.slSector;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_slSector);
      auto coin_inner_nsw_slSector_goodTiming=Monitored::Collection("coin_inner_nsw_slSector_goodTiming",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return (m.goodTiming) ? m.slSector : -999;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_slSector_goodTiming);
      auto coin_inner_nsw_slSector_endcap=Monitored::Collection("coin_inner_nsw_slSector_endcap",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return (std::abs(m.roiEta)>1.3 && m.isForward==0) ? m.slSector : -999;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_slSector_endcap);
      auto coin_inner_nsw_slSector_forward=Monitored::Collection("coin_inner_nsw_slSector_forward",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return (m.isForward==1) ? m.slSector : -999;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_slSector_forward);
      auto coin_inner_nsw_slSector_goodTiming_endcap=Monitored::Collection("coin_inner_nsw_slSector_goodTiming_endcap",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return (std::abs(m.roiEta)>1.3 && m.isForward==0 && m.goodTiming) ? m.slSector : -999;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_slSector_goodTiming_endcap);
      auto coin_inner_nsw_slSector_goodTiming_forward=Monitored::Collection("coin_inner_nsw_slSector_goodTiming_forward",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return (m.isForward==1 && m.goodTiming) ? m.slSector : -999;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_slSector_goodTiming_forward);
      auto coin_inner_nsw_roiEta=Monitored::Collection("coin_inner_nsw_roiEta",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return m.roiEta;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_roiEta);
      auto coin_inner_nsw_roiPhi=Monitored::Collection("coin_inner_nsw_roiPhi",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return m.roiPhi;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_roiPhi);
      auto coin_inner_nsw_roiNum=Monitored::Collection("coin_inner_nsw_roiNum",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return m.roiNum;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_roiNum);
      auto coin_inner_nsw_deltaBcid=Monitored::Collection("coin_inner_nsw_deltaBcid",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return m.deltaBcid;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_deltaBcid);
      auto coin_inner_nsw_deltaTiming=Monitored::Collection("coin_inner_nsw_deltaTiming",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return m.deltaTiming;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_deltaTiming);
      auto coin_inner_nsw_R=Monitored::Collection("coin_inner_nsw_R",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return m.R;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_R);
      auto coin_inner_nsw_Phi=Monitored::Collection("coin_inner_nsw_Phi",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return m.Phi;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_Phi);
      auto coin_inner_nsw_deltaTheta=Monitored::Collection("coin_inner_nsw_deltaTheta",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return m.deltaTheta;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_deltaTheta);
      auto coin_inner_nsw_isForward=Monitored::Collection("coin_inner_nsw_isForward",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return m.isForward==1;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_isForward);
      auto coin_inner_nsw_isEndcap=Monitored::Collection("coin_inner_nsw_isEndcap",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return m.isForward==0;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_isEndcap);
      auto coin_inner_nsw_currBc=Monitored::Collection("coin_inner_nsw_currBc",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return m.currBc;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_currBc);
      auto coin_inner_nsw_endcap_currBc=Monitored::Collection("coin_inner_nsw_endcap_currBc",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return (m.isForward==0 && m.currBc==1);
	});
      tgcCoin_variables.push_back(coin_inner_nsw_endcap_currBc);
      auto coin_inner_nsw_forward_currBc=Monitored::Collection("coin_inner_nsw_forward_currBc",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return (m.isForward==1 && m.currBc==1);
	});
      tgcCoin_variables.push_back(coin_inner_nsw_forward_currBc);
      auto coin_inner_nsw_goodBcid0=Monitored::Collection("coin_inner_nsw_goodBcid0",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return m.goodBcid0;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_goodBcid0);
      auto coin_inner_nsw_goodBcid1=Monitored::Collection("coin_inner_nsw_goodBcid1",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return m.goodBcid1;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_goodBcid1);
      auto coin_inner_nsw_goodBcid2=Monitored::Collection("coin_inner_nsw_goodBcid2",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return m.goodBcid2;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_goodBcid2);
      auto coin_inner_nsw_goodTiming=Monitored::Collection("coin_inner_nsw_goodTiming",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return m.goodTiming;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_goodTiming);

      auto coin_inner_nsw_slInputIndex=Monitored::Collection("coin_inner_nsw_slInputIndex",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return m.slInputIndex;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_slInputIndex);
      auto coin_inner_nsw_slInputIndex_AEndcap=Monitored::Collection("coin_inner_nsw_slInputIndex_AEndcap",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return (m.isAside==1 && m.isForward==0) ? m.slInputIndex : -999;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_slInputIndex_AEndcap);
      auto coin_inner_nsw_slInputIndex_CEndcap=Monitored::Collection("coin_inner_nsw_slInputIndex_CEndcap",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return (m.isAside==0 && m.isForward==0) ? m.slInputIndex : -999;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_slInputIndex_CEndcap);
      auto coin_inner_nsw_slInputIndex_AForward=Monitored::Collection("coin_inner_nsw_slInputIndex_AForward",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return (m.isAside==1 && m.isForward==1) ? m.slInputIndex : -999;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_slInputIndex_AForward);
      auto coin_inner_nsw_slInputIndex_CForward=Monitored::Collection("coin_inner_nsw_slInputIndex_CForward",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return (m.isAside==0 && m.isForward==1) ? m.slInputIndex : -999;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_slInputIndex_CForward);

      auto coin_inner_nsw_goodTimingBcid0=Monitored::Collection("coin_inner_nsw_goodTimingBcid0",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return (m.goodTiming==1 && m.goodBcid0==1);
	});
      tgcCoin_variables.push_back(coin_inner_nsw_goodTimingBcid0);
      auto coin_inner_nsw_goodTimingBcid1=Monitored::Collection("coin_inner_nsw_goodTimingBcid1",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return (m.goodTiming==1 && m.goodBcid1==1);
	});
      tgcCoin_variables.push_back(coin_inner_nsw_goodTimingBcid1);
      auto coin_inner_nsw_goodTimingBcid2=Monitored::Collection("coin_inner_nsw_goodTimingBcid2",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return (m.goodTiming==1 && m.goodBcid2==1);
	});
      tgcCoin_variables.push_back(coin_inner_nsw_goodTimingBcid2);
      auto coin_inner_nsw_BcPrev=Monitored::Collection("coin_inner_nsw_BcPrev",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return m.deltaTiming==-1;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_BcPrev);
      auto coin_inner_nsw_BcCurr=Monitored::Collection("coin_inner_nsw_BcCurr",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return m.deltaTiming==0;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_BcCurr);
      auto coin_inner_nsw_BcNext=Monitored::Collection("coin_inner_nsw_BcNext",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return m.deltaTiming==1;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_BcNext);
      auto coin_inner_nsw_BcNextNext=Monitored::Collection("coin_inner_nsw_BcNextNext",tgcTrigNswMap,[](const TgcTrigNsw&m){
	  return m.deltaTiming==2;
	});
      tgcCoin_variables.push_back(coin_inner_nsw_BcNextNext);

      // Tile inner coincidence
      auto coin_inner_tile_slSector=Monitored::Collection("coin_inner_tile_slSector",tgcTrigTileMap,[](const TgcTrigTile&m){
	  return m.slSector;
	});
      tgcCoin_variables.push_back(coin_inner_tile_slSector);
      auto coin_inner_tile_slSector_goodTiming=Monitored::Collection("coin_inner_tile_slSector_goodTiming",tgcTrigTileMap,[](const TgcTrigTile&m){
	  return (m.goodTiming) ? m.slSector : -999;
	});
      tgcCoin_variables.push_back(coin_inner_tile_slSector_goodTiming);
      auto coin_inner_tile_roiEta=Monitored::Collection("coin_inner_tile_roiEta",tgcTrigTileMap,[](const TgcTrigTile&m){
	  return m.roiEta;
	});
      tgcCoin_variables.push_back(coin_inner_tile_roiEta);
      auto coin_inner_tile_roiPhi=Monitored::Collection("coin_inner_tile_roiPhi",tgcTrigTileMap,[](const TgcTrigTile&m){
	  return m.roiPhi;
	});
      tgcCoin_variables.push_back(coin_inner_tile_roiPhi);
      auto coin_inner_tile_roiNum=Monitored::Collection("coin_inner_tile_roiNum",tgcTrigTileMap,[](const TgcTrigTile&m){
	  return m.roiNum;
	});
      tgcCoin_variables.push_back(coin_inner_tile_roiNum);
      auto coin_inner_tile_deltaBcid=Monitored::Collection("coin_inner_tile_deltaBcid",tgcTrigTileMap,[](const TgcTrigTile&m){
	  return m.deltaBcid;
	});
      tgcCoin_variables.push_back(coin_inner_tile_deltaBcid);
      auto coin_inner_tile_deltaTiming=Monitored::Collection("coin_inner_tile_deltaTiming",tgcTrigTileMap,[](const TgcTrigTile&m){
	  return m.deltaTiming;
	});
      tgcCoin_variables.push_back(coin_inner_tile_deltaTiming);
      auto coin_inner_tile_tmdbDecisions=Monitored::Collection("coin_inner_tile_tmdbDecisions",tgcTrigTileMap,[](const TgcTrigTile&m){
	  return m.tmdbDecisions;
	});
      tgcCoin_variables.push_back(coin_inner_tile_tmdbDecisions);
      auto coin_inner_tile_currBc=Monitored::Collection("coin_inner_tile_currBc",tgcTrigTileMap,[](const TgcTrigTile&m){
	  return m.currBc;
	});
      tgcCoin_variables.push_back(coin_inner_tile_currBc);
      auto coin_inner_tile_goodBcid0=Monitored::Collection("coin_inner_tile_goodBcid0",tgcTrigTileMap,[](const TgcTrigTile&m){
	  return m.goodBcid0;
	});
      tgcCoin_variables.push_back(coin_inner_tile_goodBcid0);
      auto coin_inner_tile_goodBcid1=Monitored::Collection("coin_inner_tile_goodBcid1",tgcTrigTileMap,[](const TgcTrigTile&m){
	  return m.goodBcid1;
	});
      tgcCoin_variables.push_back(coin_inner_tile_goodBcid1);
      auto coin_inner_tile_goodBcid2=Monitored::Collection("coin_inner_tile_goodBcid2",tgcTrigTileMap,[](const TgcTrigTile&m){
	  return m.goodBcid2;
	});
      tgcCoin_variables.push_back(coin_inner_tile_goodBcid2);
      auto coin_inner_tile_goodTiming=Monitored::Collection("coin_inner_tile_goodTiming",tgcTrigTileMap,[](const TgcTrigTile&m){
	  return m.goodTiming;
	});
      tgcCoin_variables.push_back(coin_inner_tile_goodTiming);
      // Tile inner coincidence (modified decisions)
      auto coin_inner_tile2_slSector=Monitored::Collection("coin_inner_tile2_slSector",tgcTrigTileMap_allmods,[](const TgcTrigTile&m){
	  return m.slSector;
	});
      tgcCoin_variables.push_back(coin_inner_tile2_slSector);
      auto coin_inner_tile2_currBc=Monitored::Collection("coin_inner_tile2_currBc",tgcTrigTileMap_allmods,[](const TgcTrigTile&m){
	  return m.currBc;
	});
      tgcCoin_variables.push_back(coin_inner_tile2_currBc);
      auto coin_inner_tile2_tmdbDecisions=Monitored::Collection("coin_inner_tile2_tmdbDecisions",tgcTrigTileMap_allmods,[](const TgcTrigTile&m){
	  return m.tmdbDecisions;
	});
      tgcCoin_variables.push_back(coin_inner_tile2_tmdbDecisions);

      // EIFI inner coincidence
      auto coin_inner_eifi_slSector_goodTiming=Monitored::Collection("coin_inner_eifi_slSector_goodTiming",tgcTrigEifiMap,[](const TgcTrigEifi&m){
	  return (m.goodTiming) ? m.slSector : -999;
	});
      tgcCoin_variables.push_back(coin_inner_eifi_slSector_goodTiming);
      auto coin_inner_eifi_slSector=Monitored::Collection("coin_inner_eifi_slSector",tgcTrigEifiMap,[](const TgcTrigEifi&m){
	  return m.slSector;
	});
      tgcCoin_variables.push_back(coin_inner_eifi_slSector);
      auto coin_inner_eifi_roiEta=Monitored::Collection("coin_inner_eifi_roiEta",tgcTrigEifiMap,[](const TgcTrigEifi&m){
	  return m.roiEta;
	});
      tgcCoin_variables.push_back(coin_inner_eifi_roiEta);
      auto coin_inner_eifi_roiPhi=Monitored::Collection("coin_inner_eifi_roiPhi",tgcTrigEifiMap,[](const TgcTrigEifi&m){
	  return m.roiPhi;
	});
      tgcCoin_variables.push_back(coin_inner_eifi_roiPhi);
      auto coin_inner_eifi_roiNum=Monitored::Collection("coin_inner_eifi_roiNum",tgcTrigEifiMap,[](const TgcTrigEifi&m){
	  return m.roiNum;
	});
      tgcCoin_variables.push_back(coin_inner_eifi_roiNum);
      auto coin_inner_eifi_deltaTiming=Monitored::Collection("coin_inner_eifi_deltaTiming",tgcTrigEifiMap,[](const TgcTrigEifi&m){
	  return m.deltaTiming;
	});
      tgcCoin_variables.push_back(coin_inner_eifi_deltaTiming);
      auto coin_inner_eifi_currBc=Monitored::Collection("coin_inner_eifi_currBc",tgcTrigEifiMap,[](const TgcTrigEifi&m){
	  return m.currBc;
	});
      tgcCoin_variables.push_back(coin_inner_eifi_currBc);
      auto coin_inner_eifi_goodTiming=Monitored::Collection("coin_inner_eifi_goodTiming",tgcTrigEifiMap,[](const TgcTrigEifi&m){
	  return m.goodTiming;
	});
      tgcCoin_variables.push_back(coin_inner_eifi_goodTiming);

      
      fill(m_packageName+"_TgcCoin", tgcCoin_variables);

      ATH_MSG_DEBUG("End filling TGC CoinData histograms");
    }
  }

  ///////////////// End filling TGC CoinData histograms /////////////////
  ATH_MSG_DEBUG("Done fillHistograms()");
  return StatusCode::SUCCESS;
}

void TgcRawDataMonitorAlgorithm::fillTgcCoin(const std::string & type,
					     const std::vector<TgcTrig>& tgcTrigs, 
					     std::vector<Monitored::ObjectsCollection<std::vector<TgcTrig>, double>>& varowner,
					     MonVariables& variables) const {
  varowner.push_back(Monitored::Collection(type+"_coin_lb",tgcTrigs,[](const TgcTrig&m){return m.lb;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_eta",tgcTrigs,[](const TgcTrig&m){return m.eta;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_phi",tgcTrigs,[](const TgcTrig&m){return m.phi + tgc_coin_phi_small_offset;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_bunch",tgcTrigs,[](const TgcTrig&m){return m.bunch;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_roi",tgcTrigs,[](const TgcTrig&m){return m.roi;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_sector",tgcTrigs,[](const TgcTrig&m){return m.sector;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_pt",tgcTrigs,[](const TgcTrig&m){return m.pt & 0xF;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_CoinFlagQ",tgcTrigs,[](const TgcTrig&m){return m.isPositiveDeltaR;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_CoinFlagQpos",tgcTrigs,[](const TgcTrig&m){return m.isPositiveDeltaR==0;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_CoinFlagQneg",tgcTrigs,[](const TgcTrig&m){return m.isPositiveDeltaR==1;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_CoinFlags",tgcTrigs,[](const TgcTrig&m){return (m.pt>>CoinFlags)&0x7;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_CoinFlagF",tgcTrigs,[](const TgcTrig&m){return (m.pt>>CoinFlagF)&0x1;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_CoinFlagC",tgcTrigs,[](const TgcTrig&m){return (m.pt>>CoinFlagC)&0x1;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_CoinFlagH",tgcTrigs,[](const TgcTrig&m){return (m.pt>>CoinFlagH)&0x1;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_InnerCoinType",tgcTrigs,[](const TgcTrig&m){return (m.pt>>InnerCoinFlags)&0xF;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_CoinFlagEI",tgcTrigs,[](const TgcTrig&m){return (m.pt>>CoinFlagEI)&0x1;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_CoinFlagTile",tgcTrigs,[](const TgcTrig&m){return (m.pt>>CoinFlagTile)&0x1;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_CoinFlagRPC",tgcTrigs,[](const TgcTrig&m){return (m.pt>>CoinFlagRPC)&0x1;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_CoinFlagNSW",tgcTrigs,[](const TgcTrig&m){return (m.pt>>CoinFlagNSW)&0x1;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_veto",tgcTrigs,[](const TgcTrig&m){return m.veto;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_veto_sector",tgcTrigs,[](const TgcTrig&m){return (m.veto==1)?(m.sector):(-1);}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_veto_roi",tgcTrigs,[](const TgcTrig&m){return (m.veto==1)?(m.roi):(-1);}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_isPositiveDeltaR",tgcTrigs,[](const TgcTrig&m){return m.isPositiveDeltaR;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_cutmask_pt1",tgcTrigs,[](const TgcTrig&m){return ((m.pt&0xF)==1);}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_cutmask_pt2",tgcTrigs,[](const TgcTrig&m){return ((m.pt&0xF)==2);}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_cutmask_pt3",tgcTrigs,[](const TgcTrig&m){return ((m.pt&0xF)==3);}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_cutmask_pt4",tgcTrigs,[](const TgcTrig&m){return ((m.pt&0xF)==4);}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_cutmask_pt5",tgcTrigs,[](const TgcTrig&m){return ((m.pt&0xF)==5);}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_cutmask_pt6",tgcTrigs,[](const TgcTrig&m){return ((m.pt&0xF)==6);}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_cutmask_pt7",tgcTrigs,[](const TgcTrig&m){return ((m.pt&0xF)==7);}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_cutmask_pt8",tgcTrigs,[](const TgcTrig&m){return ((m.pt&0xF)==8);}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_cutmask_pt9",tgcTrigs,[](const TgcTrig&m){return ((m.pt&0xF)==9);}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_cutmask_pt10",tgcTrigs,[](const TgcTrig&m){return ((m.pt&0xF)==10);}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_cutmask_pt11",tgcTrigs,[](const TgcTrig&m){return ((m.pt&0xF)==11);}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_cutmask_pt12",tgcTrigs,[](const TgcTrig&m){return ((m.pt&0xF)==12);}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_cutmask_pt13",tgcTrigs,[](const TgcTrig&m){return ((m.pt&0xF)==13);}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_cutmask_pt14",tgcTrigs,[](const TgcTrig&m){return ((m.pt&0xF)==14);}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_cutmask_pt15",tgcTrigs,[](const TgcTrig&m){return ((m.pt&0xF)==15);}));
  variables.push_back(varowner.back());
}
void TgcRawDataMonitorAlgorithm::fillTgcCoinEff(const std::string & type,
						const std::vector<TgcTrig>& tgcTrigs, 
						const std::vector<ExtPos>& extpositions_pivot,
						std::vector<ExtTrigInfo>& extTrigInfoVec,
						std::vector<Monitored::ObjectsCollection<std::vector<ExtTrigInfo>, double>>& varowner,
						MonVariables& variables) const {
  for(const auto& ext : extpositions_pivot){
    if(ext.muon->pt() < m_pTCutOnTrigExtrapolation.value() )continue;
    bool matched = false;
    bool matchedQ = false;
    bool matchedF = false;
    bool matchedC = false;
    bool matchedH = false;
    bool matchedEI = false;
    bool matchedTile = false;
    bool matchedRPC = false;
    bool matchedNSW = false;
    for(const auto& tgcTrig : tgcTrigs){
      if(tgcTrig.bunch!=0)continue; // only the current bunch
      if(tgcTrig.isAside==1 && ext.extPos.z()<0)continue;
      if(tgcTrig.isAside==0 && ext.extPos.z()>0)continue;
      if(tgcTrig.type == Muon::TgcCoinData::TYPE_SL){
	const Amg::Vector3D posOut(tgcTrig.x_Out,tgcTrig.y_Out,tgcTrig.z_Out);
	if( Amg::deltaR(posOut,ext.extPos) > m_l1trigMatchWindowPt15 )continue;
      }else{
	TVector2 vec(tgcTrig.x_Out,tgcTrig.y_Out);
	double deltaPhi = vec.DeltaPhi( TVector2(ext.extPos.x(), ext.extPos.y()) );
	double deltaR = vec.Mod() - TVector2(ext.extPos.x(), ext.extPos.y()).Mod();
	if( std::abs(deltaPhi) > m_dPhiCutOnM3 || std::abs(deltaR) > m_dRCutOnM3 )continue;
      }
      matched |= 1;
      int charge = (tgcTrig.isPositiveDeltaR==0) ? (-1) : (+1);
      matchedQ |= (ext.muon->charge()*charge>0);
      matchedF |= (tgcTrig.pt>>CoinFlagF) & 0x1;
      matchedC |= (tgcTrig.pt>>CoinFlagC) & 0x1;
      matchedH |= (tgcTrig.pt>>CoinFlagH) & 0x1;
      matchedEI |= (tgcTrig.pt>>CoinFlagEI) & 0x1;
      matchedTile |= (tgcTrig.pt>>CoinFlagTile) & 0x1;
      matchedRPC |= (tgcTrig.pt>>CoinFlagRPC) & 0x1;
      matchedNSW |= (tgcTrig.pt>>CoinFlagNSW) & 0x1;
    }
    ExtTrigInfo extTrigInfo;
    extTrigInfo.eta = ext.extPos.eta();
    extTrigInfo.phi = ext.extPos.phi();
    extTrigInfo.matched = matched;
    extTrigInfo.matchedQ = matchedQ;
    extTrigInfo.matchedF = matchedF;
    extTrigInfo.matchedC = matchedC;
    extTrigInfo.matchedH = matchedH;
    extTrigInfo.matchedEI = matchedEI;
    extTrigInfo.matchedTile = matchedTile;
    extTrigInfo.matchedRPC = matchedRPC;
    extTrigInfo.matchedNSW = matchedNSW;
    extTrigInfoVec.push_back(extTrigInfo);
  }
  varowner.push_back(Monitored::Collection(type+"_coin_ext_eta",extTrigInfoVec,[](const ExtTrigInfo&m){return m.eta;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_ext_phi",extTrigInfoVec,[](const ExtTrigInfo&m){return m.phi;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_ext_matched",extTrigInfoVec,[](const ExtTrigInfo&m){return m.matched;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_ext_matched_eta",extTrigInfoVec,[](const ExtTrigInfo&m){return (m.matched)?m.eta:(-10);}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_ext_matched_phi",extTrigInfoVec,[](const ExtTrigInfo&m){return (m.matched)?m.phi:(-10);}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_ext_matched_CoinFlagQ",extTrigInfoVec,[](const ExtTrigInfo&m){return m.matchedQ;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_ext_matched_CoinFlagF",extTrigInfoVec,[](const ExtTrigInfo&m){return m.matchedF;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_ext_matched_CoinFlagC",extTrigInfoVec,[](const ExtTrigInfo&m){return m.matchedC;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_ext_matched_CoinFlagH",extTrigInfoVec,[](const ExtTrigInfo&m){return m.matchedH;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_ext_matched_CoinFlagEI",extTrigInfoVec,[](const ExtTrigInfo&m){return m.matchedEI;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_ext_matched_CoinFlagTile",extTrigInfoVec,[](const ExtTrigInfo&m){return m.matchedTile;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_ext_matched_CoinFlagRPC",extTrigInfoVec,[](const ExtTrigInfo&m){return m.matchedRPC;}));
  variables.push_back(varowner.back());
  varowner.push_back(Monitored::Collection(type+"_coin_ext_matched_CoinFlagNSW",extTrigInfoVec,[](const ExtTrigInfo&m){return m.matchedNSW;}));
  variables.push_back(varowner.back());
}
double TgcRawDataMonitorAlgorithm::getNswRindexFromEta(const double& eta) const {
  double theta = 2.0 * std::atan( std::exp(-1.0 * std::abs(eta)) );
  double r = std::tan( theta ) * nsw_z;
  double rindex = (r - nsw_rmin) / nsw_rindex_step;
  return rindex;
}
double TgcRawDataMonitorAlgorithm::getMatchingWindow(const xAOD::Muon* muon) const {
  // matching window
  double max_dr = 999;
  double pt = muon->pt();
  if (pt > pt_15_cut) max_dr = m_l1trigMatchWindowPt15;
  else if (pt > pt_10_cut) max_dr = m_l1trigMatchWindowPt10a + m_l1trigMatchWindowPt10b * pt / Gaudi::Units::GeV;
  else max_dr = m_l1trigMatchWindowPt0a + m_l1trigMatchWindowPt0b * pt / Gaudi::Units::GeV;
  return max_dr;
}

void 
TgcRawDataMonitorAlgorithm::fillThresholdPatternHistograms(std::map<std::string,std::vector<const xAOD::MuonRoI*>> & menuAndRoIs, const std::vector < TimedMuon > & mymuons, const EventContext & ctx) const {
  if(m_monitorThresholdPatterns){
    for(const auto& item : m_thrMonList){
      std::vector<bool> passed;
      passed.reserve(mymuons.size());
      for(const auto& mymuon : mymuons){
    	  passed.push_back( mymuon.matchedL1Items.find(item) != mymuon.matchedL1Items.end() );
      }
      auto passed_rois = menuAndRoIs[item];

      MonVariables thrMonVariables;

      auto lumiBlock_l1item = Monitored::Scalar<int>(Form("lumiBlock_l1item_%s",item.data()),GetEventInfo(ctx)->lumiBlock());
      thrMonVariables.push_back(lumiBlock_l1item);

      auto muon_passed_l1item = Monitored::Collection(Form("muon_passed_l1item_%s",item.data()),passed);
      thrMonVariables.push_back(muon_passed_l1item);

      auto muon_eta_l1item=Monitored::Collection(Form("muon_eta_l1item_%s",item.data()),mymuons,[](const TimedMuon&m){
        return (m.muon->pt() > pt_30_cut) ? m.muon->eta() : -10;
      });
      thrMonVariables.push_back(muon_eta_l1item);
      auto muon_phi_l1item=Monitored::Collection(Form("muon_phi_l1item_%s",item.data()),mymuons,[](const TimedMuon&m){
        return (m.muon->pt() > pt_30_cut) ? m.muon->phi() : -10;
      });
      thrMonVariables.push_back(muon_phi_l1item);
      auto muon_pt_rpc_l1item=Monitored::Collection(Form("muon_pt_rpc_l1item_%s",item.data()),mymuons,[](const TimedMuon&m){
        return (std::abs(m.muon->eta()) < barrel_end) ? m.muon->pt() / Gaudi::Units::GeV : -10;
      });
      thrMonVariables.push_back(muon_pt_rpc_l1item);
      auto muon_pt_tgc_l1item=Monitored::Collection(Form("muon_pt_tgc_l1item_%s",item.data()),mymuons,[](const TimedMuon&m){
        return (std::abs(m.muon->eta()) > barrel_end && std::abs(m.muon->eta()) < trigger_end) ? m.muon->pt() / Gaudi::Units::GeV : -10;
      });
      thrMonVariables.push_back(muon_pt_tgc_l1item);
      auto muon_phi_rpc_l1item=Monitored::Collection(Form("muon_phi_rpc_l1item_%s",item.data()),mymuons,[](const TimedMuon&m){
        return (std::abs(m.muon->eta()) < barrel_end && m.muon->pt() > pt_30_cut) ? m.muon->phi() : -10;
      });
      thrMonVariables.push_back(muon_phi_rpc_l1item);
      auto muon_phi_tgc_l1item=Monitored::Collection(Form("muon_phi_tgc_l1item_%s",item.data()),mymuons,[](const TimedMuon&m){
        return (std::abs(m.muon->eta()) > barrel_end && std::abs(m.muon->eta()) < trigger_end && m.muon->pt() > pt_30_cut) ? m.muon->phi() : -10;
      });
      thrMonVariables.push_back(muon_phi_tgc_l1item);

      auto l1item_roi_eta=Monitored::Collection(Form("l1item_roi_eta_%s",item.data()),passed_rois,[](const xAOD::MuonRoI*m){return m->eta();});
      thrMonVariables.push_back(l1item_roi_eta);
      auto l1item_roi_phi=Monitored::Collection(Form("l1item_roi_phi_%s",item.data()),passed_rois,[](const xAOD::MuonRoI*m){return m->phi();});
      thrMonVariables.push_back(l1item_roi_phi);
      auto l1item_roi_phi_rpc=Monitored::Collection(Form("l1item_roi_phi_rpc_%s",item.data()),passed_rois,[](const xAOD::MuonRoI*m){
        return (m->getSource()==xAOD::MuonRoI::Barrel)?(m->phi()):(-10);
      });
      auto l1item_roi_phi_tgc=Monitored::Collection(Form("l1item_roi_phi_tgc_%s",item.data()),passed_rois,[](const xAOD::MuonRoI*m){
        return (m->getSource()!=xAOD::MuonRoI::Barrel)?(m->phi()):(-10);
      });
      thrMonVariables.push_back(l1item_roi_phi_tgc);

      auto l1item_roi_phi_barrel=Monitored::Collection(Form("l1item_roi_phi_barrel_%s",item.data()),passed_rois,[](const xAOD::MuonRoI*m){
        return (m->getSource() == xAOD::MuonRoI::Barrel) ? m->phi() : -10;
      });
      thrMonVariables.push_back(l1item_roi_phi_barrel);
      auto l1item_roi_phi_endcap=Monitored::Collection(Form("l1item_roi_phi_endcap_%s",item.data()),passed_rois,[](const xAOD::MuonRoI*m){
        return (m->getSource() == xAOD::MuonRoI::Endcap) ? m->phi() : -10;
      });
      thrMonVariables.push_back(l1item_roi_phi_endcap);
      auto l1item_roi_phi_forward=Monitored::Collection(Form("l1item_roi_phi_forward_%s",item.data()),passed_rois,[](const xAOD::MuonRoI*m){
        return (m->getSource() == xAOD::MuonRoI::Forward) ? m->phi() : -10;
      });
      thrMonVariables.push_back(l1item_roi_phi_forward);
      auto l1item_roi_sideA=Monitored::Collection(Form("l1item_roi_sideA_%s",item.data()),passed_rois,[](const xAOD::MuonRoI*m){
        return m->getHemisphere() == xAOD::MuonRoI::Positive;
      });
      thrMonVariables.push_back(l1item_roi_sideA);
      auto l1item_roi_sideC=Monitored::Collection(Form("l1item_roi_sideC_%s",item.data()),passed_rois,[](const xAOD::MuonRoI*m){
	  return m->getHemisphere() == xAOD::MuonRoI::Negative;
	});
      thrMonVariables.push_back(l1item_roi_sideC);

      auto l1item_roi_roiNumber=Monitored::Collection(Form("l1item_roi_roiNumber_%s",item.data()),passed_rois,[](const xAOD::MuonRoI*m){
	  return m->getRoI();
	});
      thrMonVariables.push_back(l1item_roi_roiNumber);

      auto l1item_roi_sector = Monitored::Collection(Form("l1item_roi_sector_%s",item.data()),passed_rois,[](const xAOD::MuonRoI*m) {
	  return (m->getHemisphere() == xAOD::MuonRoI::Positive)?(m->getSectorID()+1):(-1 * m->getSectorID()-1);
	});
      thrMonVariables.push_back(l1item_roi_sector);
      auto l1item_roi_barrel = Monitored::Collection(Form("l1item_roi_barrel_%s",item.data()),passed_rois,[](const xAOD::MuonRoI*m) {
	  return m->getSource() == xAOD::MuonRoI::Barrel;
	});
      thrMonVariables.push_back(l1item_roi_barrel);
      auto l1item_roi_endcap = Monitored::Collection(Form("l1item_roi_endcap_%s",item.data()),passed_rois,[](const xAOD::MuonRoI*m) {
	  return m->getSource() == xAOD::MuonRoI::Endcap;
	});
      thrMonVariables.push_back(l1item_roi_endcap);
      auto l1item_roi_forward = Monitored::Collection(Form("l1item_roi_forward_%s",item.data()),passed_rois,[](const xAOD::MuonRoI*m) {
	  return m->getSource() == xAOD::MuonRoI::Forward;
	});
      thrMonVariables.push_back(l1item_roi_forward);
      auto l1item_roi_thrNumber=Monitored::Collection(Form("l1item_roi_thrNumber_%s",item.data()),passed_rois,[](const xAOD::MuonRoI*m){
	  return m->getThrNumber();
	});
      thrMonVariables.push_back(l1item_roi_thrNumber);

      auto l1item_roi_ismorecand=Monitored::Collection(Form("l1item_roi_ismorecand_%s",item.data()),passed_rois,[](const xAOD::MuonRoI*m){
	  return (m->getSource()==xAOD::MuonRoI::Barrel)?(m->isMoreCandInRoI()):(-1);
	});
      thrMonVariables.push_back(l1item_roi_ismorecand);
      auto l1item_roi_bw3coin=Monitored::Collection(Form("l1item_roi_bw3coin_%s",item.data()),passed_rois,[](const xAOD::MuonRoI*m){
	  return (m->getSource()!=xAOD::MuonRoI::Barrel)?(m->getBW3Coincidence()):(-1);
	});
      thrMonVariables.push_back(l1item_roi_bw3coin);
      auto l1item_roi_innercoin=Monitored::Collection(Form("l1item_roi_innercoin_%s",item.data()),passed_rois,[](const xAOD::MuonRoI*m){
	  return (m->getSource()!=xAOD::MuonRoI::Barrel)?(m->getInnerCoincidence()):(-1);
	});
      thrMonVariables.push_back(l1item_roi_innercoin);
      auto l1item_roi_goodmf=Monitored::Collection(Form("l1item_roi_goodmf_%s",item.data()),passed_rois,[](const xAOD::MuonRoI*m){
	  return (m->getSource()!=xAOD::MuonRoI::Barrel)?(m->getGoodMF()):(-1);
	});
      thrMonVariables.push_back(l1item_roi_goodmf);

      fill(m_packageName + item.data(), thrMonVariables);
    }
  }
  ///////////////// End filling thresholdPattern histograms /////////////////
}

void 
TgcRawDataMonitorAlgorithm::fillTgcPrdHistograms(Monitored::Scalar<int> & mon_bcid, Monitored::Scalar<int> & mon_pileup, Monitored::Scalar<int> & mon_lb,std::map <std::string, std::vector< ExtPos>> & extpositions, const EventContext & ctx) const{
  if (m_anaTgcPrd) {
    ATH_MSG_DEBUG("m_anaTgcPrd = True");
    SG::ReadCondHandle<MuonGM::MuonDetectorManager> DetectorManagerHandle{m_DetectorManagerKey,ctx};
    const MuonGM::MuonDetectorManager* muonMgr = DetectorManagerHandle.cptr();
    SG::ReadHandle < Muon::TgcPrepDataContainer > tgcPrd(m_TgcPrepDataContainerKey, ctx);
    std::map<const xAOD::Muon*, std::set<std::string>> map_muon_and_tgchits;
    if(tgcPrd.isValid() && muonMgr!=nullptr){
      ATH_MSG_DEBUG("Filling TGC PRD histograms");
      const TgcIdHelper &tgcIdHelper = m_idHelperSvc->tgcIdHelper();
      std::vector < TGC::TgcHit > tgcHits;
      std::map<std::string, std::vector<TGC::TgcHit>> tgcHitsMap;
      for (const auto tgccnt : *tgcPrd) {
        for (const auto data : *tgccnt) {
          const MuonGM::TgcReadoutElement *element = data->detectorElement();
          const Identifier id = data->identify();
          const int gasGap = tgcIdHelper.gasGap(id);
          const int channel = tgcIdHelper.channel(id);
          const bool isStrip = tgcIdHelper.isStrip(id);
          const Amg::Vector3D &pos = isStrip ? element->stripPos(gasGap, channel) : element->wireGangPos(gasGap, channel);
          const double shortWidth = (isStrip)?(element->stripShortWidth(gasGap, channel)):(element->gangShortWidth(gasGap, channel));
          const double longWidth = (isStrip)?(element->stripLongWidth(gasGap, channel)):(element->gangLongWidth(gasGap, channel));
          const double length = (isStrip)?(element->stripLength()):(element->gangRadialLength(gasGap, channel));
          const int bcmask = data->getBcBitMap();
          TGC::TgcHit tgcHit(pos[0],pos[1],pos[2],
                 shortWidth,longWidth, length,
                 isStrip,gasGap,channel,tgcIdHelper.stationEta(id),tgcIdHelper.stationPhi(id),tgcIdHelper.stationName(id),
                 bcmask);
          if(extpositions.find(tgcHit.cham_name())!=extpositions.end()){
            for(auto& cham : extpositions[tgcHit.cham_name()]){
              double newX = cham.extPos.x() + cham.extVec.x() / cham.extVec.z() * ( tgcHit.Z() - cham.extPos.z() );
              double newY = cham.extPos.y() + cham.extVec.y() / cham.extVec.z() * ( tgcHit.Z() - cham.extPos.z() );
              Identifier id2 = muonMgr->tgcIdHelper()->elementID(tgcHit.StationName(), tgcHit.StationEta(), tgcHit.StationPhi());
              
              auto detEle = muonMgr->getTgcReadoutElement(id2);
              double chamPhi = detEle->center().phi();
              TVector2 extPos(newX,newY);
              TVector2 hitPos(tgcHit.X(),tgcHit.Y());
              TVector2 rot_extPos  = extPos.Rotate(-chamPhi + M_PI/2.);
              TVector2 rot_hitPos  = hitPos.Rotate(-chamPhi + M_PI/2.);
              double res = (tgcHit.isStrip())? std::sin( rot_extPos.DeltaPhi( rot_hitPos ) ) * rot_extPos.Mod(): rot_hitPos.Y() - rot_extPos.Y();
              tgcHit.addResidual( cham.muon, res );
              if( std::abs(res) < m_residualWindow ){
                cham.chambersHasHit.insert(tgcHit.type_name());
                map_muon_and_tgchits[cham.muon].insert(tgcHit.channel_name());
              }
            }
          }
          tgcHits.push_back(tgcHit);
          tgcHitsMap[tgcHit.cham_name() + ( (tgcHit.isStrip())?("S"):("W") )].push_back(tgcHit); // <- chamber-by-chamber residual plots
          tgcHitsMap[tgcHit.type_name()].push_back(tgcHit); // <- gap-by-gap channel occupancy plots
        }
      }

      std::map<std::string, std::vector<int>> tgcHitPhiMap;
      std::map<std::string, std::vector<int>> tgcHitEtaMap;
      std::map<std::string, std::vector<int>> tgcHitPhiMapGlobal;
      std::map<std::string, std::vector<int>> tgcHitPhiMapOnline;
      std::map<std::string, std::vector<int>> tgcHitEtaMapOnline;
      std::map<std::string, std::vector<int>> tgcHitPhiMapGlobalOnline;
      std::map<std::string, std::vector<int>> tgcHitTiming;
      std::map<std::string, std::vector<int>> tgcHitPhiMapGlobalWithTrack;
      std::map<std::string, std::vector<int>> tgcHitTimingWithTrack;
      std::map<const std::string, std::vector<TGC::TgcHit>> tgcHitBCMaskMap;
      std::vector <int> vec_bw24sectors; // 1..12 BW-A, -1..-12 BW-C
      std::vector <int> vec_bw24sectors_wire;
      std::vector <int> vec_bw24sectors_strip;
      std::vector <int> vec_bwfulleta; // 0(Forward), 1..4(M1), 1..5(M2,M3)
      std::vector <int> vec_bwfulleta_wire;
      std::vector <int> vec_bwfulleta_strip;
      std::vector <int> vec_bwtiming;
      std::vector <int> vec_bwtiming_wire;
      std::vector <int> vec_bwtiming_strip;
      std::vector <int> vec_bw24sectors_wTrack; // 1..12 BW-A, -1..-12 BW-C
      std::vector <int> vec_bw24sectors_wire_wTrack;
      std::vector <int> vec_bw24sectors_strip_wTrack;
      std::vector <int> vec_bwfulleta_wTrack; // 0(Forward), 1..4(M1), 1..5(M2,M3)
      std::vector <int> vec_bwfulleta_wire_wTrack;
      std::vector <int> vec_bwfulleta_strip_wTrack;
      std::vector <int> vec_bwtiming_wTrack;
      std::vector <int> vec_bwtiming_wire_wTrack;
      std::vector <int> vec_bwtiming_strip_wTrack;
      for(const auto& tgcHit : tgcHits){
        bool hasAssociatedGoodMuonTrack = false;
        for(const auto& res : tgcHit.residuals()){
          const xAOD::Muon* muon = res.first;
          if(map_muon_and_tgchits[muon].find(tgcHit.channel_name()) == map_muon_and_tgchits[muon].end()) continue;
          int nWhits = 0;
          int nShits = 0;
          for(const auto& chamHasHit : map_muon_and_tgchits[muon]){
            if( chamHasHit.find(tgcHit.gap_name()) != std::string::npos ) continue; // skipping the same gap
            if( chamHasHit.find("M04") != std::string::npos ) continue; // skipping EI/FI
            if( chamHasHit.find('W') != std::string::npos ) nWhits++;
            if( chamHasHit.find('S') != std::string::npos ) nShits++;
          }
          if(nWhits < m_nHitsInOtherBWTGCWire) continue;
          if(nShits < m_nHitsInOtherBWTGCStrip) continue;
          hasAssociatedGoodMuonTrack = true;
          break;
        }
      
        // debugging purpose: should be False by default
        if(m_dumpFullChannelList)ATH_MSG_INFO("TGCHIT: " << tgcHit.channel_name());
      
        // BCID analysis for TGC TTCrx delay scan
        if(hasAssociatedGoodMuonTrack) tgcHitBCMaskMap[tgcHit.channel_name()].push_back(tgcHit);
      
        std::string station_name = Form("%sM%02d%s",(tgcHit.iSide()==TGC::TGCSIDE::TGCASIDE)?("A"):("C"),tgcHit.iM(),(tgcHit.isStrip())?("S"):("W"));
        int phimap_index = 0;
        int etamap_index = 0;
        int phimap_global_index = 0; // no empty bins compare to the above index
        m_tgcMonTool->getMapIndex(tgcHit,etamap_index,phimap_index,phimap_global_index );
	int phimap_index_online = 0;
	int etamap_index_online = 0;
	int phimap_global_index_online = 0;
	m_tgcMonTool->getMapIndexOnline(tgcHit.type_name(),etamap_index_online,phimap_index_online,phimap_global_index_online );
	std::string wire_or_strip = (tgcHit.type_name().size()==16) ? Form("%c_%s",tgcHit.type_name()[0],(tgcHit.type_name()[15]=='W')?("wire"):("strip")) : "null";
        for(int bunch = -1 ; bunch <= +1 ; bunch++){
          if(bunch==-1 && (tgcHit.bcmask()&Muon::TgcPrepData::BCBIT_PREVIOUS)==0)continue;
          if(bunch== 0 && (tgcHit.bcmask()&Muon::TgcPrepData::BCBIT_CURRENT)==0)continue;
          if(bunch==+1 && (tgcHit.bcmask()&Muon::TgcPrepData::BCBIT_NEXT)==0)continue;
          tgcHitPhiMap[station_name].push_back(phimap_index);
          tgcHitEtaMap[station_name].push_back(etamap_index);
          tgcHitPhiMapGlobal[station_name].push_back(phimap_global_index);
          tgcHitTiming[station_name].push_back(bunch);
	  tgcHitPhiMapOnline[wire_or_strip].push_back(phimap_index_online);
	  tgcHitEtaMapOnline[wire_or_strip].push_back(etamap_index_online);
	  tgcHitPhiMapGlobalOnline[wire_or_strip].push_back(phimap_global_index_online);
          if(hasAssociatedGoodMuonTrack){
            tgcHitPhiMapGlobalWithTrack[station_name].push_back(phimap_global_index);
            tgcHitTimingWithTrack[station_name].push_back(bunch);
          }
      
          if(tgcHit.iM()!=4){
            vec_bw24sectors.push_back((tgcHit.iSide()==TGC::TGCSIDE::TGCASIDE)?(tgcHit.iSec()):(-tgcHit.iSec()));
            vec_bwfulleta.push_back(tgcHit.iEta());
            vec_bwtiming.push_back(bunch);
            if(hasAssociatedGoodMuonTrack){
              vec_bw24sectors_wTrack.push_back((tgcHit.iSide()==TGC::TGCSIDE::TGCASIDE)?(tgcHit.iSec()):(-tgcHit.iSec()));
              vec_bwfulleta_wTrack.push_back(tgcHit.iEta());
              vec_bwtiming_wTrack.push_back(bunch);
            }
            if(tgcHit.isStrip()){
              vec_bw24sectors_strip.push_back((tgcHit.iSide()==TGC::TGCSIDE::TGCASIDE)?(tgcHit.iSec()):(-tgcHit.iSec()));
              vec_bwfulleta_strip.push_back(tgcHit.iEta());
              vec_bwtiming_strip.push_back(bunch);
              if(hasAssociatedGoodMuonTrack){
          vec_bw24sectors_strip_wTrack.push_back((tgcHit.iSide()==TGC::TGCSIDE::TGCASIDE)?(tgcHit.iSec()):(-tgcHit.iSec()));
          vec_bwfulleta_strip_wTrack.push_back(tgcHit.iEta());
          vec_bwtiming_strip_wTrack.push_back(bunch);
              }
            }else{
              vec_bw24sectors_wire.push_back((tgcHit.iSide()==TGC::TGCSIDE::TGCASIDE)?(tgcHit.iSec()):(-tgcHit.iSec()));
              vec_bwfulleta_wire.push_back(tgcHit.iEta());
              vec_bwtiming_wire.push_back(bunch);
              if(hasAssociatedGoodMuonTrack){
          vec_bw24sectors_wire_wTrack.push_back((tgcHit.iSide()==TGC::TGCSIDE::TGCASIDE)?(tgcHit.iSec()):(-tgcHit.iSec()));
          vec_bwfulleta_wire_wTrack.push_back(tgcHit.iEta());
          vec_bwtiming_wire_wTrack.push_back(bunch);
              }
            }
          }
        }
      }
      
      ATH_MSG_DEBUG("filling hit_variables");
      
      MonVariables  hit_variables;
      hit_variables.push_back(mon_bcid);
      hit_variables.push_back(mon_pileup);
      hit_variables.push_back(mon_lb);

      auto hit_n = Monitored::Scalar<int>("hit_n", tgcHits.size());
      hit_variables.push_back(hit_n);

      auto hit_bcmask=Monitored::Collection("hit_bcmask",tgcHits,[](const TGC::TgcHit&m){return m.bcmask();});
      hit_variables.push_back(hit_bcmask);

      auto hit_sideA=Monitored::Collection("hit_sideA",tgcHits,[](const TGC::TgcHit&m){return m.Z()>0;});
      hit_variables.push_back(hit_sideA);

      auto hit_sideC=Monitored::Collection("hit_sideC",tgcHits,[](const TGC::TgcHit&m){return m.Z() < 0;});
      hit_variables.push_back(hit_sideC);

      auto hit_bw24sectors=Monitored::Collection("hit_bw24sectors",vec_bw24sectors,[](const int&m){return m;});
      hit_variables.push_back(hit_bw24sectors);
      auto hit_bw24sectors_strip=Monitored::Collection("hit_bw24sectors_strip",vec_bw24sectors_strip,[](const int&m){return m;});
      hit_variables.push_back(hit_bw24sectors_strip);
      auto hit_bw24sectors_wire=Monitored::Collection("hit_bw24sectors_wire",vec_bw24sectors_wire,[](const int&m){return m;});
      hit_variables.push_back(hit_bw24sectors_wire);
      auto hit_bwfulleta=Monitored::Collection("hit_bwfulleta",vec_bwfulleta,[](const int&m){return m;});
      hit_variables.push_back(hit_bwfulleta);
      auto hit_bwfulleta_strip=Monitored::Collection("hit_bwfulleta_strip",vec_bwfulleta_strip,[](const int&m){return m;});
      hit_variables.push_back(hit_bwfulleta_strip);
      auto hit_bwfulleta_wire=Monitored::Collection("hit_bwfulleta_wire",vec_bwfulleta_wire,[](const int&m){return m;});
      hit_variables.push_back(hit_bwfulleta_wire);
      auto hit_bwtiming=Monitored::Collection("hit_bwtiming",vec_bwtiming,[](const int&m){return m;});
      hit_variables.push_back(hit_bwtiming);
      auto hit_bwtiming_strip=Monitored::Collection("hit_bwtiming_strip",vec_bwtiming_strip,[](const int&m){return m;});
      hit_variables.push_back(hit_bwtiming_strip);
      auto hit_bwtiming_wire=Monitored::Collection("hit_bwtiming_wire",vec_bwtiming_wire,[](const int&m){return m;});
      hit_variables.push_back(hit_bwtiming_wire);

      auto hit_bw24sectors_wTrack=Monitored::Collection("hit_bw24sectors_wTrack",vec_bw24sectors_wTrack,[](const int&m){return m;});
      hit_variables.push_back(hit_bw24sectors_wTrack);
      auto hit_bw24sectors_strip_wTrack=Monitored::Collection("hit_bw24sectors_strip_wTrack",vec_bw24sectors_strip_wTrack,[](const int&m){return m;});
      hit_variables.push_back(hit_bw24sectors_strip_wTrack);
      auto hit_bw24sectors_wire_wTrack=Monitored::Collection("hit_bw24sectors_wire_wTrack",vec_bw24sectors_wire_wTrack,[](const int&m){return m;});
      hit_variables.push_back(hit_bw24sectors_wire_wTrack);
      auto hit_bwfulleta_wTrack=Monitored::Collection("hit_bwfulleta_wTrack",vec_bwfulleta_wTrack,[](const int&m){return m;});
      hit_variables.push_back(hit_bwfulleta_wTrack);
      auto hit_bwfulleta_strip_wTrack=Monitored::Collection("hit_bwfulleta_strip_wTrack",vec_bwfulleta_strip_wTrack,[](const int&m){return m;});
      hit_variables.push_back(hit_bwfulleta_strip_wTrack);
      auto hit_bwfulleta_wire_wTrack=Monitored::Collection("hit_bwfulleta_wire_wTrack",vec_bwfulleta_wire_wTrack,[](const int&m){return m;});
      hit_variables.push_back(hit_bwfulleta_wire_wTrack);
      auto hit_bwtiming_wTrack=Monitored::Collection("hit_bwtiming_wTrack",vec_bwtiming_wTrack,[](const int&m){return m;});
      hit_variables.push_back(hit_bwtiming_wTrack);
      auto hit_bwtiming_strip_wTrack=Monitored::Collection("hit_bwtiming_strip_wTrack",vec_bwtiming_strip_wTrack,[](const int&m){return m;});
      hit_variables.push_back(hit_bwtiming_strip_wTrack);
      auto hit_bwtiming_wire_wTrack=Monitored::Collection("hit_bwtiming_wire_wTrack",vec_bwtiming_wire_wTrack,[](const int&m){return m;});
      hit_variables.push_back(hit_bwtiming_wire_wTrack);

      std::vector<Monitored::ObjectsCollection<std::vector<int>, double>> varowner;
      varowner.reserve(tgcHitPhiMap.size() * 2 + tgcHitPhiMapGlobal.size() * 2 + tgcHitPhiMapOnline.size() * 3 + tgcHitPhiMapGlobalWithTrack.size() * 2);
      for (const auto &phimap : tgcHitPhiMap) {
	varowner.push_back(Monitored::Collection(Form("hit_x_%s",phimap.first.data()),tgcHitEtaMap[phimap.first],[](const int&m){return m;}));
      	hit_variables.push_back(varowner.back());
	varowner.push_back(Monitored::Collection(Form("hit_y_%s", phimap.first.data()),phimap.second,[](const int&m){return m;}));
      	hit_variables.push_back(varowner.back());
      }
      for (const auto &phimap : tgcHitPhiMapOnline) {
	varowner.push_back(Monitored::Collection(Form("hit_online_x_%s",phimap.first.data()),tgcHitEtaMapOnline[phimap.first],[](const int&m){return m;}));
	hit_variables.push_back(varowner.back());
	varowner.push_back(Monitored::Collection(Form("hit_online_y_%s", phimap.first.data()),phimap.second,[](const int&m){return m;}));
	hit_variables.push_back(varowner.back());
	varowner.push_back(Monitored::Collection(Form("hit_online_glblphi_%s", phimap.first.data()),tgcHitPhiMapGlobalOnline[phimap.first],[](const int&m){return m;}));
	hit_variables.push_back(varowner.back());
      }

      for (const auto &phimap : tgcHitPhiMapGlobal) {
	varowner.push_back(Monitored::Collection(Form("hit_glblphi_%s", phimap.first.data()),phimap.second,[](const int&m){return m;}));
	hit_variables.push_back(varowner.back());
	varowner.push_back(Monitored::Collection(Form("hit_bunch_%s", phimap.first.data()),tgcHitTiming[phimap.first],[](const int&m){return m;}));
	hit_variables.push_back(varowner.back());
      }
      for (const auto &phimap : tgcHitPhiMapGlobalWithTrack) {
	varowner.push_back(Monitored::Collection(Form("hit_glblphi_wTrack_%s", phimap.first.data()),phimap.second,[](const int&m){return m;}));
	hit_variables.push_back(varowner.back());
	varowner.push_back(Monitored::Collection(Form("hit_bunch_wTrack_%s", phimap.first.data()),tgcHitTimingWithTrack[phimap.first],[](const int&m){return m;}));
	hit_variables.push_back(varowner.back());
      }
      
      // BCMask plots (for TTCrx gate delay scan)
      std::map<std::string, std::vector<int>> tgcHitBCMaskGlobalIndex;
      std::map<std::string, std::vector<int>> tgcHitBCMask;
      std::map<std::string, std::vector<int>> tgcHitBCMaskBWSectors;
      std::map<std::string, std::vector<int>> tgcHitBCMaskForBWSectors;
      for(const auto& channelNameAndBCMask : tgcHitBCMaskMap){
	if(m_maskChannelList.find(channelNameAndBCMask.first)!=m_maskChannelList.end())continue; // skipping problematic channels
	std::string chamberNameWithWS = channelNameAndBCMask.first.substr(0,16); // e.g. A01M01f01E01L01W
	int thisChannel = std::atoi( channelNameAndBCMask.first.substr(18,3).data() ); // e.g. 001 of "Ch001"
	std::string prev1ChannelName = Form("%sCh%03d",chamberNameWithWS.data(),thisChannel-1);
	std::string next1ChannelName = Form("%sCh%03d",chamberNameWithWS.data(),thisChannel+1);
	// vetoing if neighboring channels have hits to avoid cross-talk effect
	if(tgcHitBCMaskMap.find(prev1ChannelName)!=tgcHitBCMaskMap.end())continue;
	if(tgcHitBCMaskMap.find(next1ChannelName)!=tgcHitBCMaskMap.end())continue;
	std::string cham_name = channelNameAndBCMask.first.substr(0,12); // e.g. A01M01f01E01
	int iLay = std::atoi( channelNameAndBCMask.first.substr(13,2).data() );
	TGC::TgcChamber cham; cham.initChamber(cham_name);
	int phimap_index = 0;
	int etamap_index = 0;
	int phimap_global_index = 0;
	if(!m_tgcMonTool->getMapIndex(cham,iLay,etamap_index,phimap_index,phimap_global_index ))continue;
	std::string station_name = Form("%sM%02d%s",(cham.iSide()==TGC::TGCSIDE::TGCASIDE)?("A"):("C"),cham.iM(),channelNameAndBCMask.first.substr(15,1).data());
	for(const auto& tgcHit : channelNameAndBCMask.second){
	  tgcHitBCMaskGlobalIndex[station_name].push_back(phimap_global_index);
	  tgcHitBCMask[station_name].push_back(tgcHit.bcmask());
	  if(cham.iM()!=4){
	    tgcHitBCMaskBWSectors["All"].push_back( (cham.iSide()==TGC::TGCSIDE::TGCASIDE)?( +1 * cham.iSec() ):(-1 * cham.iSec()) );
	    tgcHitBCMaskForBWSectors["All"].push_back(tgcHit.bcmask());
	    if(chamberNameWithWS.find('W')!=std::string::npos){
	      tgcHitBCMaskBWSectors["Wire"].push_back( (cham.iSide()==TGC::TGCSIDE::TGCASIDE)?( +1 * cham.iSec() ):(-1 * cham.iSec()) );
	      tgcHitBCMaskForBWSectors["Wire"].push_back(tgcHit.bcmask());
	    }else{
	      tgcHitBCMaskBWSectors["Strip"].push_back( (cham.iSide()==TGC::TGCSIDE::TGCASIDE)?( +1 * cham.iSec() ):(-1 * cham.iSec()) );
	      tgcHitBCMaskForBWSectors["Strip"].push_back(tgcHit.bcmask());
	    }
	  }
	}
      }
      std::vector<Monitored::ObjectsCollection<std::vector<int>, double>> varowner_bcmask;
      varowner_bcmask.reserve(tgcHitBCMask.size() * 2 + tgcHitBCMaskBWSectors.size() * 2);
      for(const auto& chamType : tgcHitBCMaskBWSectors){
        varowner_bcmask.push_back(Monitored::Collection(Form("hit_bcmask_bw24sectors_%s",chamType.first.data()),chamType.second,[](const int&m){return m;}));
        hit_variables.push_back(varowner_bcmask.back());
        varowner_bcmask.push_back(Monitored::Collection(Form("hit_bcmask_for_bw24sectors_%s",chamType.first.data()),tgcHitBCMaskForBWSectors[chamType.first],[](const int&m){return m;}));
        hit_variables.push_back(varowner_bcmask.back());
      }
      for(const auto& stationNameAndBCMask : tgcHitBCMask){
        varowner_bcmask.push_back(Monitored::Collection(Form("hit_bcmask_glblphi_%s",stationNameAndBCMask.first.data()),tgcHitBCMaskGlobalIndex[stationNameAndBCMask.first],[](const int&m){return m;}));
        hit_variables.push_back(varowner_bcmask.back());
        varowner_bcmask.push_back(Monitored::Collection(Form("hit_bcmask_%s",stationNameAndBCMask.first.data()),stationNameAndBCMask.second,[](const int&m){return m;}));
        hit_variables.push_back(varowner_bcmask.back());
      }

      // gap-by-gap efficiency by track extrapolation
      ATH_MSG_DEBUG("preparing for efficiency plots");
      std::map<std::string, std::vector<double>> tgcEffPhiMap_Denominator;
      std::map<std::string, std::vector<double>> tgcEffEtaMap_Denominator;
      std::map<std::string, std::vector<double>> tgcEffPhiMapGlobal_Denominator;
      std::map<std::string, std::vector<double>> tgcEffPhiMap_Numerator;
      std::map<std::string, std::vector<double>> tgcEffEtaMap_Numerator;
      std::map<std::string, std::vector<double>> tgcEffPhiMapGlobal_Numerator;
      std::map<std::string, std::vector<double>> tgcEffPhiMapOnline_Denominator;
      std::map<std::string, std::vector<double>> tgcEffEtaMapOnline_Denominator;
      std::map<std::string, std::vector<double>> tgcEffPhiMapGlobalOnline_Denominator;
      std::map<std::string, std::vector<double>> tgcEffPhiMapOnline_Numerator;
      std::map<std::string, std::vector<double>> tgcEffEtaMapOnline_Numerator;
      std::map<std::string, std::vector<double>> tgcEffPhiMapGlobalOnline_Numerator;
      std::map<std::string, std::vector<double>> tgcEffMapExtX;
      std::map<std::string, std::vector<double>> tgcEffMapExtY;
      std::map<std::string, std::vector<double>> tgcEffMapHasHit;
      for(const auto& exts : extpositions){
	const std::string& cham_name = exts.first;
	TGC::TgcChamber cham; cham.initChamber(cham_name);
	// local-coordinate x'-y'
	Identifier id2 = muonMgr->tgcIdHelper()->elementID(cham.StationName(), cham.StationEta(), cham.StationPhi());
	auto detEle = muonMgr->getTgcReadoutElement(id2);
	for(const auto& ext : exts.second){ // how often tracks are extrapolated to this chamber surface,e.i. denominator
	  Amg::Vector3D extPosLocal = detEle->transform().inverse() * ext.extPos;
	  Amg::Vector3D extVecLocal = detEle->transform().inverse() * ext.extVec;
	  for(int iLay = 1 ; iLay <= 3 ; iLay++){
	    int phimap_index = 0;
	    int etamap_index = 0;
	    int phimap_global_index = 0;
	    if(!m_tgcMonTool->getMapIndex(cham,iLay,etamap_index,phimap_index,phimap_global_index ))continue;

	    double newX = extPosLocal.x() - extVecLocal.x() / extVecLocal.z() * extPosLocal.z();
	    double newY = extPosLocal.y() - extVecLocal.y() / extVecLocal.z() * extPosLocal.z();
	    for(int iSorW = 0 ; iSorW < 2 ; iSorW++){
	      if(cham.iM()==1 && iLay==2 && iSorW==0)continue;
	      std::string gap_name = Form("%sL%02d",cham_name.data(),iLay);
	      std::string type_name = Form("%sL%02d%s",cham_name.data(),iLay,(iSorW==0)?("S"):("W"));
	      int nWhits = 0;
	      int nShits = 0;
	      for(const auto& chamHasHit : map_muon_and_tgchits[ext.muon]){
		if( chamHasHit.find(gap_name) != std::string::npos ) continue; // skipping the same gap
		if( chamHasHit.find("M04") != std::string::npos ) continue; // skipping EI/FI
		if( chamHasHit.find('W') != std::string::npos ) nWhits++;
		if( chamHasHit.find('S') != std::string::npos ) nShits++;
	      }
	      if(nWhits < m_nHitsInOtherBWTGCWire)continue;
	      if(nShits < m_nHitsInOtherBWTGCStrip)continue;
	      std::string station_name = Form("%sM%02d%s",(cham.iSide()==TGC::TGCSIDE::TGCASIDE)?("A"):("C"),cham.iM(),(iSorW==0)?("S"):("W"));
	      int phimap_index_online = 0;
	      int etamap_index_online = 0;
	      int phimap_global_index_online = 0;
	      m_tgcMonTool->getMapIndexOnline(type_name,etamap_index_online,phimap_index_online,phimap_global_index_online );
	      std::string wire_or_strip = (type_name.size()==16) ? Form("%c_%s",type_name[0],(type_name[15]=='W')?("wire"):("strip")) : ("null");
	      tgcEffPhiMap_Denominator[station_name].push_back(phimap_index);
	      tgcEffEtaMap_Denominator[station_name].push_back(etamap_index);
	      tgcEffPhiMapGlobal_Denominator[station_name].push_back(phimap_global_index);
	      tgcEffPhiMapOnline_Denominator[wire_or_strip].push_back(phimap_index_online);
	      tgcEffEtaMapOnline_Denominator[wire_or_strip].push_back(etamap_index_online);
	      tgcEffPhiMapGlobalOnline_Denominator[wire_or_strip].push_back(phimap_global_index_online);
	      tgcEffMapExtX[type_name].push_back(newX);
	      tgcEffMapExtY[type_name].push_back(newY);
	      double hitExist = 0;
	      if( ext.chambersHasHit.find(type_name) != ext.chambersHasHit.end()) hitExist=1;
	      tgcEffPhiMap_Numerator[station_name].push_back(hitExist);
	      tgcEffEtaMap_Numerator[station_name].push_back(hitExist);
	      tgcEffPhiMapGlobal_Numerator[station_name].push_back(hitExist);
	      tgcEffPhiMapOnline_Numerator[wire_or_strip].push_back(hitExist);
	      tgcEffEtaMapOnline_Numerator[wire_or_strip].push_back(hitExist);
	      tgcEffPhiMapGlobalOnline_Numerator[wire_or_strip].push_back(hitExist);
	      tgcEffMapHasHit[type_name].push_back(hitExist);

	    }
	  }
	}
      }

      std::vector<Monitored::ObjectsCollection<std::vector<double>, double>> varowner_hiteff;
      std::vector<Monitored::ObjectsCollection<std::vector<TGC::TgcHit>, double>> varowner_eachchamber;
      std::vector<Monitored::ObjectsCollection<std::vector<double>, double>> varowner_eachchamber_double;
      std::map<std::string,std::vector<double>> cham_and_res;

      if(m_fillGapByGapHistograms){

	ATH_MSG_DEBUG("hit efficiency plots");
	varowner_hiteff.reserve(tgcEffPhiMap_Denominator.size() * 4 + tgcEffPhiMapGlobal_Denominator.size() * 2 + tgcEffMapHasHit.size() * 3 + tgcEffPhiMapOnline_Denominator.size() * 6);
	for (const auto &phimap : tgcEffPhiMap_Denominator) {
	  varowner_hiteff.push_back(Monitored::Collection(Form("hit_effden_x_%s",phimap.first.data()),tgcEffEtaMap_Denominator[phimap.first],[](const double&m){return m;}));
	  hit_variables.push_back(varowner_hiteff.back());
	  varowner_hiteff.push_back(Monitored::Collection(Form("hit_effden_y_%s", phimap.first.data()),tgcEffPhiMap_Denominator[phimap.first],[](const double&m){return m;}));
	  hit_variables.push_back(varowner_hiteff.back());
	  varowner_hiteff.push_back(Monitored::Collection(Form("hit_effnum_x_%s",phimap.first.data()),tgcEffEtaMap_Numerator[phimap.first],[](const double&m){return m;}));
	  hit_variables.push_back(varowner_hiteff.back());
	  varowner_hiteff.push_back(Monitored::Collection(Form("hit_effnum_y_%s", phimap.first.data()),tgcEffPhiMap_Numerator[phimap.first],[](const double&m){return m;}));
	  hit_variables.push_back(varowner_hiteff.back());
	}
	for (const auto &phimap : tgcEffPhiMapGlobal_Denominator) {
	  varowner_hiteff.push_back(Monitored::Collection(Form("hit_glblphi_effden_%s", phimap.first.data()),tgcEffPhiMapGlobal_Denominator[phimap.first],[](const double&m){return m;}));
	  hit_variables.push_back(varowner_hiteff.back());
	  varowner_hiteff.push_back(Monitored::Collection(Form("hit_glblphi_effnum_%s", phimap.first.data()),tgcEffPhiMapGlobal_Numerator[phimap.first],[](const double&m){return m;}));
	  hit_variables.push_back(varowner_hiteff.back());
	}
	for (const auto &phimap : tgcEffPhiMapOnline_Denominator) {
	  varowner_hiteff.push_back(Monitored::Collection(Form("hit_online_effden_x_%s",phimap.first.data()),tgcEffEtaMapOnline_Denominator[phimap.first],[](const double&m){return m;}));
	  hit_variables.push_back(varowner_hiteff.back());
	  varowner_hiteff.push_back(Monitored::Collection(Form("hit_online_effden_y_%s", phimap.first.data()),tgcEffPhiMapOnline_Denominator[phimap.first],[](const double&m){return m;}));
	  hit_variables.push_back(varowner_hiteff.back());
	  varowner_hiteff.push_back(Monitored::Collection(Form("hit_online_effnum_x_%s",phimap.first.data()),tgcEffEtaMapOnline_Numerator[phimap.first],[](const double&m){return m;}));
	  hit_variables.push_back(varowner_hiteff.back());
	  varowner_hiteff.push_back(Monitored::Collection(Form("hit_online_effnum_y_%s", phimap.first.data()),tgcEffPhiMapOnline_Numerator[phimap.first],[](const double&m){return m;}));
	  hit_variables.push_back(varowner_hiteff.back());
	  varowner_hiteff.push_back(Monitored::Collection(Form("hit_online_glblphi_effden_%s", phimap.first.data()),tgcEffPhiMapGlobalOnline_Denominator[phimap.first],[](const double&m){return m;}));
	  hit_variables.push_back(varowner_hiteff.back());
	  varowner_hiteff.push_back(Monitored::Collection(Form("hit_online_glblphi_effnum_%s", phimap.first.data()),tgcEffPhiMapGlobalOnline_Numerator[phimap.first],[](const double&m){return m;}));
	  hit_variables.push_back(varowner_hiteff.back());
	}
	for(const auto& hiteffmap : tgcEffMapHasHit){
	  varowner_hiteff.push_back(Monitored::Collection(Form("hit_localX_effden_%s", hiteffmap.first.data()),tgcEffMapExtX[hiteffmap.first],[](const double&m){return m;}));
	  hit_variables.push_back(varowner_hiteff.back());
	  varowner_hiteff.push_back(Monitored::Collection(Form("hit_localY_effden_%s", hiteffmap.first.data()),tgcEffMapExtY[hiteffmap.first],[](const double&m){return m;}));
	  hit_variables.push_back(varowner_hiteff.back());
	  varowner_hiteff.push_back(Monitored::Collection(Form("hit_effnum_%s", hiteffmap.first.data()),tgcEffMapHasHit[hiteffmap.first],[](const double&m){return m;}));
	  hit_variables.push_back(varowner_hiteff.back());
	}

	ATH_MSG_DEBUG("gap-by-gap occupancy plots and residual plots");
	varowner_eachchamber.reserve(tgcHitsMap.size());
	varowner_eachchamber_double.reserve(tgcHitsMap.size());
      	for (const auto &tgcHitMap : tgcHitsMap) {
	  auto chanName = tgcHitMap.first;
	  if(chanName.find('L')!=std::string::npos){ // individual gaps
	    varowner_eachchamber.push_back(Monitored::Collection(Form("hit_on_%s",chanName.data()),tgcHitMap.second,[](const TGC::TgcHit&m){return m.channel();}));
	    hit_variables.push_back(varowner_eachchamber.back());
	  }else{ // only summed over the gaps
	    for(const auto&tgcHit:tgcHitMap.second){
	      for(const auto&tgcRes:tgcHit.residuals()){
          cham_and_res[chanName].push_back(tgcRes.second);
        }
      }
      varowner_eachchamber_double.push_back(Monitored::Collection(Form("hit_residual_on_%s",chanName.data()),cham_and_res[chanName],[](const double&m){return m;}));
      hit_variables.push_back(varowner_eachchamber_double.back());
    }
      	}

      }

      ATH_MSG_DEBUG("before fill for hits");
      fill(m_packageName+"_TgcHit", hit_variables);
      ATH_MSG_DEBUG("End filling TGC PRD histograms");
    }else{
      ATH_MSG_WARNING("Couldn't get TGC PRD");
    }
  }
}

void
TgcRawDataMonitorAlgorithm::fillMuonRoisInThresholdPattern(std::map<const xAOD::MuonRoI*,std::set<std::string>> &roiAndMenu, std::map<std::string,std::vector<const xAOD::MuonRoI*>> & menuAndRoIs, std::vector<TimedMuonRoI> & AllBCMuonRoIs, const EventContext & ctx) const {

  if(m_monitorThresholdPatterns && AllBCMuonRoIs.size()>0 ){
    ATH_MSG_DEBUG("Filling histograms for MuonRoIs in thresholdPattern");
    SG::ReadHandle<TrigConf::L1Menu> l1Menu = SG::makeHandle(m_L1MenuKey, ctx);
    SG::ReadDecorHandle<xAOD::MuonRoIContainer,uint64_t> thrPatternAcc = SG::makeHandle<uint64_t>(m_thresholdPatternsKey, ctx);
    if(l1Menu.isValid() && thrPatternAcc.isPresent() && thrPatternAcc.isAvailable()){
      for(const auto& item : m_thrMonList){
        ATH_MSG_DEBUG("Item = " << item);
        bool ok = false;
        for(const auto& m : l1Menu->thresholdNames()){
          ATH_MSG_DEBUG("item = " << m);
          if( m == item ){
            ok = true;
            break;
          }
        }
        if(!ok){
          ATH_MSG_DEBUG("skipping " << item);
          continue;
        }
        ATH_MSG_DEBUG("continue checking " << item);
        const TrigConf::L1Threshold& thr = l1Menu->threshold(item.data());
        std::vector<const xAOD::MuonRoI*> passed_rois;
        for(const auto& allBcMuonRoI : AllBCMuonRoIs){
          if(allBcMuonRoI.timing!=0)continue; // only current BC
          const xAOD::MuonRoI* roi = allBcMuonRoI.muonRoI;
          const uint64_t thrPattern = thrPatternAcc(*roi);
          bool passed = ( thrPattern & (1 << thr.mapping()) );
          if(passed){
            passed_rois.push_back(roi);
            ATH_MSG_DEBUG("This RoI passed "<< item <<", roiWord=" << roi->roiWord() << ", thrNumber=" << roi->getThrNumber() << " eta=" << roi->eta() << " phi=" << roi->phi());
            if(roiAndMenu.count(roi)==0){
              std::set<std::string> items;
              roiAndMenu.insert(std::make_pair(roi,items));
            }
            roiAndMenu[roi].insert(item);
          }
        }
        menuAndRoIs.insert(std::make_pair(item,passed_rois));
      }
    }
    ATH_MSG_DEBUG("End filling histograms for MuonRoIs in thresholdPattern");
  }
}



