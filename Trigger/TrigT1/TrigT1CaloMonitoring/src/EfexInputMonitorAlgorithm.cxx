/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "EfexInputMonitorAlgorithm.h"
#include "TTree.h"
#include "PathResolver/PathResolver.h"
#include "TFile.h"

EfexInputMonitorAlgorithm::EfexInputMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
{
}

StatusCode EfexInputMonitorAlgorithm::initialize() {

  ATH_MSG_DEBUG("EfexInputMonitorAlgorith::initialize");
  ATH_MSG_DEBUG("Package Name "<< m_packageName);
  ATH_MSG_DEBUG("m_eFexTowerContainer"<< m_eFexTowerContainerKey);

  // we initialise all the containers that we need
  ATH_CHECK( m_eFexTowerContainerKey.initialize() );
  ATH_CHECK( m_eFexTowerContainerRefKey.initialize() );

    ATH_CHECK( m_bcContKey.initialize() );


    // load the scid map
    if (auto fileName = PathResolverFindCalibFile( "L1CaloFEXByteStream/2023-02-13/scToEfexTowers.root" ); !fileName.empty()) {
        std::unique_ptr<TFile> f( TFile::Open(fileName.c_str()) );
        if (f) {
            TTree* t = f->Get<TTree>("mapping");
            if(t) {
                unsigned long long scid = 0;
                std::pair<int,int> coord = {0,0};
                std::pair<int,int> slot;
                t->SetBranchAddress("scid",&scid);
                t->SetBranchAddress("etaIndex",&coord.first);
                t->SetBranchAddress("phiIndex",&coord.second);
                t->SetBranchAddress("slot1",&slot.first);
                t->SetBranchAddress("slot2",&slot.second);
                for(Long64_t i=0;i<t->GetEntries();i++) {
                    t->GetEntry(i);
                    m_scMap[std::make_pair(coord,slot.first)].first.insert(scid);
                    m_scMap[std::make_pair(coord,slot.second)].first.insert(scid);
                }
                // now convert scid list to list of strings
                for(auto& [key,scs] : m_scMap) {
                    std::stringstream s;
                    for(auto id : scs.first) {
                        s << (id>>32) << ",";
                    }
                    scs.second = s.str();
                    scs.first.clear(); // not needed any more, so clear it
                }
            }
        }
    }



  
  return AthMonitorAlgorithm::initialize();
}

StatusCode EfexInputMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

  ATH_MSG_DEBUG("EfexInputMonitorAlgorithm::fillHistograms");

  // Access eFex eTower container - should always have a collection, even if empty
  SG::ReadHandle<xAOD::eFexTowerContainer> eFexTowerContainer{m_eFexTowerContainerKey, ctx};
  if(!eFexTowerContainer.isValid()){
    ATH_MSG_ERROR("No eFex Tower container found in storegate  "<< m_eFexTowerContainerKey);
    return StatusCode::FAILURE;
  }


    auto evtNumber = Monitored::Scalar<ULong64_t>("EventNumber",GetEventInfo(ctx)->eventNumber());
    auto lbnString = Monitored::Scalar<std::string>("LBNString",std::to_string(GetEventInfo(ctx)->lumiBlock()));
    auto lbn = Monitored::Scalar<int>("LBN",GetEventInfo(ctx)->lumiBlock());

    // mismatches can be caused by recent/imminent OTF maskings, so track timings
    auto timeSince = Monitored::Scalar<int>("timeSince", -1);
    auto timeUntil = Monitored::Scalar<int>("timeUntil", -1);
    auto timeUntilCapped = Monitored::Scalar<int>("timeUntilCapped", -1); // capped at 199 for histograms
    SG::ReadCondHandle<LArBadChannelCont> larBadChan{ m_bcContKey, ctx };
    if(larBadChan.isValid()) {
        timeSince = ctx.eventID().time_stamp() - larBadChan.getRange().start().time_stamp();
        timeUntil = larBadChan.getRange().stop().time_stamp() - ctx.eventID().time_stamp();
        timeUntilCapped = std::min(int(timeUntil),199);
    }
    auto Decision = Monitored::Scalar<std::string>("Error","");
    auto ErrorAndLocation = Monitored::Scalar<std::string>("ErrorAndLocation","");

    auto TowerId = Monitored::Scalar<int32_t>("TowerId",0);
    auto Towereta = Monitored::Scalar<float>("TowerEta",0.0);
    auto Towerphi = Monitored::Scalar<float>("TowerPhi",0.0);
    auto TowerRefCount = Monitored::Scalar<int>("RefTowerCount",0);
    auto Toweremstatus = Monitored::Scalar<uint32_t>("TowerEmstatus",0);
    auto Towerhadstatus = Monitored::Scalar<uint32_t>("TowerHadstatus",0);
    auto TowerSlot = Monitored::Scalar<int32_t>("TowerSlot",0);
    auto TowerCount = Monitored::Scalar<int>("TowerCount",0);
    auto SlotSCID = Monitored::Scalar<std::string>("SlotSCID","");

    // first test dataTowers not empty unless prescaled event
    bool isPrescaled = (((GetEventInfo(ctx)->extendedLevel1ID()&0xffffff) % 200) != 0);
    size_t badTowers = 0;
    if(!isPrescaled && eFexTowerContainer->empty()) {
        Decision = "MissingDataTowers";
        fill("errors", Decision,timeSince,timeUntil,evtNumber,lbn,lbnString,TowerId,Towereta,Towerphi,Toweremstatus,Towerhadstatus,TowerSlot,TowerCount,TowerRefCount,SlotSCID);
    } else if(isPrescaled && !eFexTowerContainer->empty()) {
        // check if all the towers have errors, if so then this readout IS expected
        for(auto eFexTower : *eFexTowerContainer) {
            if(eFexTower->em_status()||eFexTower->had_status()) badTowers++;
        }
        if(badTowers != eFexTowerContainer->size()) {
            Decision = "UnexpectedDataTowers";
            fill("errors", Decision, timeSince, timeUntil, evtNumber, lbn, lbnString, TowerId, Towereta, Towerphi,
                 Toweremstatus, Towerhadstatus, TowerSlot, TowerCount, TowerRefCount, SlotSCID);
        }
    }

    if(eFexTowerContainer->empty() || badTowers==eFexTowerContainer->size()) return StatusCode::SUCCESS; // don't do the rest of the monitoring unless processing a DataTowers event


    // Access eFexTower ref container, if possible
    SG::ReadHandle<xAOD::eFexTowerContainer> eFexTowerContainerRef{m_eFexTowerContainerRefKey, ctx};
    auto etaIndex = [](float eta) { return int( eta*10 ) + ((eta<0) ? -1 : 1); };
    auto phiIndex = [](float phi) { return int( phi*32./M_PI ) + (phi<0 ? -1 : 1); };
    std::map<std::pair<int,int>,const xAOD::eFexTower*> refTowers;
    bool missingLAr = false;
    if (eFexTowerContainerRef.isValid()) {
        if(eFexTowerContainerRef->empty()) missingLAr=true;
        for (auto eTower: *eFexTowerContainerRef) {
            refTowers[std::pair(etaIndex(eTower->eta() + 0.025), phiIndex(eTower->phi() + 0.025))] = eTower;
            // fill profile histograms for each layer (ECAL,HCAL) so that we can identify when a layer is being noisy
            Towereta = eTower->eta(); Towerphi = eTower->phi();
            std::vector<uint16_t> Toweret_count=eTower->et_count();
            for(size_t i=0;i<Toweret_count.size();i++) {
                TowerRefCount = Toweret_count[i];
                if (TowerRefCount==1025) missingLAr=true; // 1025 comes from eFexTowerBuilder if it had no supercells
                if(TowerRefCount==1025 || TowerRefCount==1022) TowerRefCount=0; // unavailable or invalid code
                fill((i<10) ? "ecal" : "hcal",lbn,Towereta,Towerphi,TowerRefCount);
            }
        }
    }
    if(missingLAr) {
        Decision = "MissingSCells";
        fill("errors", Decision,timeSince,timeUntil,evtNumber,lbn,lbnString,TowerId,Towereta,Towerphi,Toweremstatus,Towerhadstatus,TowerSlot,TowerCount,TowerRefCount,SlotSCID);
        ATH_MSG_WARNING(std::string(Decision) << " in event " << evtNumber << " in lb " << std::string(lbnString));
    }



  std::set<std::pair<std::pair<int,int>,int>> doneCounts; // only fill each count once (there are duplicates in DataTowers readout b.c. of module overlap)

    auto IsMonReady = Monitored::Scalar<bool>("IsMonReady",true); // used to fill into eta/phi map only certain types of error.
    auto OnPed = Monitored::Scalar<double>("OnPedestal",0.);
    auto AboveCut = Monitored::Scalar<bool>("AboveCut",false);
    auto BelowCut = Monitored::Scalar<bool>("BelowCut",false);
    auto binNumber = Monitored::Scalar<int>("binNumber",0);

    std::set<std::string> reportedErrors;

  for(const xAOD::eFexTower* eTower : *eFexTowerContainer) {
    TowerId = eTower->id();
    Towereta=eTower->eta();
    Towerphi=eTower->phi();
    Toweremstatus=eTower->em_status();
    Towerhadstatus=eTower->had_status();
    if(eTower->em_status()) {
        Decision="BadEMStatus";
        ErrorAndLocation = std::string("#splitline{") + Decision + "}{" + std::to_string(TowerId) + "}";
        fill("errors",Decision,ErrorAndLocation,timeSince,timeUntil,evtNumber,lbn,lbnString,TowerId,Towereta,Towerphi,Toweremstatus,Towerhadstatus,TowerSlot,TowerCount,TowerRefCount,SlotSCID,IsMonReady);
        if(!reportedErrors.count(Decision)) {
            ATH_MSG_WARNING(std::string(Decision) << " in event " << evtNumber << " in lb " << std::string(lbnString));
            reportedErrors.insert(Decision);
        }
    }
      if(eTower->had_status()) {
          Decision="BadHadStatus";
          ErrorAndLocation = std::string("#splitline{") + Decision + "}{" + std::to_string(TowerId) + "}";
          fill("errors",Decision,ErrorAndLocation,timeSince,timeUntil,evtNumber,lbn,lbnString,TowerId,Towereta,Towerphi,Toweremstatus,Towerhadstatus,TowerSlot,TowerCount,TowerRefCount,SlotSCID,IsMonReady);
          if(!reportedErrors.count(Decision)) {
            ATH_MSG_WARNING(std::string(Decision) << " in event " << evtNumber << " in lb " << std::string(lbnString));
            reportedErrors.insert(Decision);
          }
      }
      std::vector<uint16_t> counts=eTower->et_count();
      std::vector<uint16_t> refcounts;
      auto coord = std::pair(etaIndex(eTower->eta() + 0.025), phiIndex(eTower->phi() + 0.025));
      if(!refTowers.empty()) {
          if(auto itr = refTowers.find(coord); itr != refTowers.end()) {
              refcounts = itr->second->et_count();
          }
      }
      for(size_t i=0;i<counts.size();i++) {
          if (eTower->disconnectedCount(i)) continue; // skip disconnected counts
          // only do each tower once (skip duplications across modules)
          if (doneCounts.find(std::pair(coord,i))==doneCounts.end()) {
              doneCounts.insert(std::pair(coord,i));
          } else {
              continue;
          }
          bool isLAr = !(i==10 && std::abs(Towereta)<=1.5);
          if(counts[i]==1025) {
              // 1025 is the code used by bytestream decoder if there is no input for that slot
              if(isLAr) {
                  if (auto itr = m_scMap.find(std::make_pair(coord, i)); itr != m_scMap.end()) {
                      SlotSCID = itr->second.second;
                  }
                  Decision = "MissingLAr";
                  fill("errors", Decision,timeSince,timeUntil,evtNumber,lbn,lbnString,TowerId,Towereta,Towerphi,Toweremstatus,Towerhadstatus,TowerSlot,TowerCount,TowerRefCount,SlotSCID);
                  if(!reportedErrors.count(Decision)) {
                      ATH_MSG_WARNING(std::string(Decision) << " in event " << evtNumber << " in lb " << std::string(lbnString));
                      reportedErrors.insert(Decision);
                  }
              } else {
                  Decision = "MissingTile";
                  SlotSCID="";
                  fill("errors", Decision,timeSince,timeUntil,evtNumber,lbn,lbnString,TowerId,Towereta,Towerphi,Toweremstatus,Towerhadstatus,TowerSlot,TowerCount,TowerRefCount,SlotSCID);
                  if(!reportedErrors.count(Decision)) {
                      ATH_MSG_WARNING(std::string(Decision) << " in event " << evtNumber << " in lb " << std::string(lbnString));
                      reportedErrors.insert(Decision);
                  }
              }
              continue;
          }
          TowerSlot = i;
          TowerCount = counts[i];
          TowerRefCount = -1;
          Decision = "";
          if(!refTowers.empty()) { // only do slot-by-slot comparison if we actually have ref towers
              if(refcounts.size() != counts.size()) {
                  Decision = "NumSlotMismatch";
              } else {
                  TowerRefCount = refcounts.at(i);
                  if(isLAr) {
                      if(TowerCount==1022) {
                          //Decision = "LArInvalidCode"; - not an error, or at least not one we (L1Calo) should be tracking for now
                          if(TowerRefCount!=1022) {
                              Decision = "LArInvalidCodeMismatched";
                          }
                      } else if(!TowerCount && TowerRefCount) {
                          Decision="LArMissingMask";
                      } else if(TowerCount && !TowerRefCount) {
                          Decision="LArExtraMask";
                      } else if(TowerCount != TowerRefCount) {
                          if(TowerRefCount==1025) {
                              Decision = "MissingRefLAr";
                          } else {
                              Decision = "LArMismatch";
                          }
                      }
                      // should we also monitor saturations (code 1023?)
                  } else if(TowerCount != TowerRefCount) {
                      if(TowerRefCount==1025) {
                          Decision = "MissingRefTile";
                      } else {
                          Decision="TileMismatch";
                      }
                  }
              }
              if(!std::string(Decision).empty()) {
                  if(isLAr) {
                      if (auto itr = m_scMap.find(std::make_pair(coord, i)); itr != m_scMap.end()) {
                          SlotSCID = itr->second.second;
                      }
                  }
                  if(!(isLAr && timeUntil>=0 && timeUntil<=1)) { // lar errors within 1s of an otf masking update are not treated as errors
                      ErrorAndLocation = std::string("#splitline{") + Decision + "}{" + std::to_string(TowerId) + "}";
                      fill("errors", Decision, ErrorAndLocation, timeSince, timeUntil, evtNumber, lbn, lbnString,
                           TowerId, Towereta, Towerphi, Toweremstatus, Towerhadstatus, TowerSlot, TowerCount,
                           TowerRefCount, SlotSCID);
                      if (!reportedErrors.count(Decision)) {
                          ATH_MSG_WARNING(std::string(Decision) << " in event " << evtNumber << " in lb "
                                                                << std::string(lbnString));
                          reportedErrors.insert(Decision);
                      }
                  }
              }
          }
          // since lar invalid codes will be treated as a 0 energy, don't fill into plot
          if ((!isLAr) || (TowerCount != 1022)) {
              AboveCut = (TowerCount >= (isLAr ? 52:1)); // count of 52 ~= 500 MeV
              BelowCut = (TowerCount < (isLAr ?  23:0) && TowerCount>0); // count of 22 ~= -500 MeV
              if(AboveCut || BelowCut) {
                  int etaIdx = TowerId/100000;
                  if(etaIdx>0) etaIdx--; // so goes from -25 to 24
                  int phiIdx = std::abs(TowerId % 100000)/1000;
                  if(phiIdx>31) phiIdx -= 64; // so goes from -32 to 31
                  binNumber = (25 + etaIdx)*64 + phiIdx+33; // = 64*(x-1)+y as displayed in standard histogram
                  fill((i<10) ? "ecal" : "hcal",lbn,Towereta,Towerphi,TowerCount,AboveCut,BelowCut,binNumber);
              }
          }
      }
  }


  return StatusCode::SUCCESS;
}


