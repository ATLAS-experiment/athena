/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "EfexSimMonitorAlgorithm.h"
#include "eFEXTOBSimDataCompare.h"
#include <set>
#include "StoreGate/ReadDecorHandle.h"

EfexSimMonitorAlgorithm::EfexSimMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
{
}

StatusCode EfexSimMonitorAlgorithm::initialize() {

  ATH_MSG_DEBUG("EfexSimMonitorAlgorith::initialize");
  ATH_MSG_DEBUG("m_eFexEmContainer"<< m_eFexEmContainerKey); 
  ATH_MSG_DEBUG("m_eFexEmSimContainer"<< m_eFexEmSimContainerKey); 
  ATH_MSG_DEBUG("m_eFexTauContainer"<< m_eFexTauContainerKey);
  ATH_MSG_DEBUG("m_eFexTauSimContainer"<< m_eFexTauSimContainerKey);

  // we initialise all the containers that we need
  ATH_CHECK( m_eFexEmContainerKey.initialize() );
  ATH_CHECK( m_eFexEmSimContainerKey.initialize() );
  ATH_CHECK( m_eFexTauContainerKey.initialize() );
  ATH_CHECK( m_eFexTauSimContainerKey.initialize() );
    ATH_CHECK( m_scellKey.initialize() );
    ATH_CHECK( m_eFexEmxContainerKey.initialize(SG::AllowEmpty) );
    ATH_CHECK( m_eFexEmxSimContainerKey.initialize(SG::AllowEmpty) );
    ATH_CHECK( m_eFexTauxContainerKey.initialize(SG::AllowEmpty) );
    ATH_CHECK( m_eFexTauxSimContainerKey.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_eFexTowerContainerKey.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_bcContKey.initialize() );
  
  return AthMonitorAlgorithm::initialize();
}

StatusCode EfexSimMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

    fillHistos(m_eFexEmSimContainerKey,m_eFexEmContainerKey,ctx,"eEM");
    fillHistos(m_eFexTauSimContainerKey,m_eFexTauContainerKey,ctx,"eTAU");
    if(!m_eFexEmxContainerKey.empty()) fillHistos(m_eFexEmxSimContainerKey,m_eFexEmxContainerKey,ctx,"eEMx");
    if(!m_eFexTauxContainerKey.empty()) fillHistos(m_eFexTauxSimContainerKey,m_eFexTauxContainerKey,ctx,"eTAUx");
    return StatusCode::SUCCESS;
}

template <typename T> unsigned int EfexSimMonitorAlgorithm::fillHistos(const SG::ReadHandleKey<T>& key1, const SG::ReadHandleKey<T>& key2, const EventContext& ctx, const std::string& signa ) const {


    auto fexReadout = Monitored::Scalar<unsigned int>("fexReadout", 0);
    if(!m_eFexTowerContainerKey.empty()) {
        SG::ReadHandle<xAOD::eFexTowerContainer> towers{m_eFexTowerContainerKey, ctx};
        if(towers.isValid() && !towers->empty()) {
            // check towers aren't all in error ... if they are
            // this is a debug readout event not a fexReadout event
            size_t badTowers=0;
            for(auto eFexTower : *towers) {
                if(eFexTower->em_status()||eFexTower->had_status()) badTowers++;
            }
            if(badTowers != towers->size()) {
                fexReadout = 1;
            }
        }
    }
    auto IsDataTowers = Monitored::Scalar<bool>("IsDataTowers",fexReadout==1);
    auto IsEmulatedTowers = Monitored::Scalar<bool>("IsEmulatedTowers",fexReadout==0);

    // mismatches can be caused by recent/imminent OTF maskings, so track timings
    auto timeSince = Monitored::Scalar<int>("timeSince", -1);
    auto timeUntil = Monitored::Scalar<int>("timeUntil", -1);
    SG::ReadCondHandle<LArBadChannelCont> larBadChan{ m_bcContKey, ctx };
    if(larBadChan.isValid()) {
        timeSince = ctx.eventID().time_stamp() - larBadChan.getRange().start().time_stamp();
        timeUntil = larBadChan.getRange().stop().time_stamp() - ctx.eventID().time_stamp();
    }

    std::string EventType = "DataTowers";
    if(fexReadout==0) {
        EventType = "EmulatedTowers";
        // removing next two lines until further investigation of cause of mismatches by LATOME
        //if((timeSince>=0&&timeSince<10)) EventType+="+JustAfter";
        if((timeUntil>=0&&timeUntil<=5)) { // events within 5s of an OTF masking change may have mismatches
            EventType+="+JustBeforeOTF";
            IsEmulatedTowers=false; // wont fill emulated tower plots with mismatches from these types of events
        }

        // also check if any supercells are missing ... mismatches will get an entry in the TTree (and entries)
        // but not feature in the EmulatedTowers mismatches plots
        SG::ReadHandle<CaloCellContainer> scells(m_scellKey,ctx); // n.b. 34048 is a full complement of scells
        if(!scells.isValid() || scells->size()!=34048){
            IsEmulatedTowers=false;
            EventType+="+MissingSCells";
        }

    }

    SG::ReadHandle<T> tobs1{key1, ctx};
    SG::ReadHandle<T> tobs2{key2, ctx};

    if(!tobs1.isValid()) {
        return 0;
    }

    std::set<uint32_t> word0s2;
    if(tobs2.isValid()) {
        for(const auto tob : *tobs2) {
            word0s2.insert(tob->word0());
        }
    }

    auto signature = Monitored::Scalar<std::string>("Signature",signa);
    auto evtType = Monitored::Scalar<std::string>("EventType",EventType);
    auto tobMismatched = Monitored::Scalar<float>("tobMismatched",0.0);

    bool mismatches=false;

    // for each collection record if TOB is matched or not

    for(const auto tob : *tobs1) {
        if(word0s2.find(tob->word0()) == word0s2.end()) {
            mismatches=true;
            break;
        }
    }
    if(tobs2.isValid() && tobs1->size() < tobs2->size()) {
        mismatches=true;
    }

    auto lbn = Monitored::Scalar<ULong64_t>("LBN",GetEventInfo(ctx)->lumiBlock());
    auto simReady = Monitored::Scalar<bool>("SimulationReady",fexReadout==1); // used to control if filling plot that is actually monitored in DQM
    if (mismatches) {
        // record all tobs to the debug tree .. one entry in the tree = 1 tobType for 1 event
        auto evtNumber = Monitored::Scalar<ULong64_t>("EventNumber",GetEventInfo(ctx)->eventNumber());

        auto lbnString = Monitored::Scalar<std::string>("LBNString","");
        auto& firstEvents = (fexReadout==1) ? m_firstEvents_DataTowers : m_firstEvents_EmulatedTowers;
        {
            std::scoped_lock lock(m_firstEventsMutex);
            auto itr = firstEvents.find(lbn);
            if(itr==firstEvents.end()) {
                firstEvents[lbn] = std::to_string(lbn)+":"+std::to_string(evtNumber);
                itr = firstEvents.find(lbn);
            }
            lbnString = itr->second;
        }

        std::vector<float> detas{};std::vector<float> setas{};
        std::vector<float> dphis{};std::vector<float> sphis{};
        std::vector<unsigned int> dword0s{};std::vector<unsigned int> sword0s{};
        auto dtobEtas = Monitored::Collection("dataEtas", detas);
        auto dtobPhis = Monitored::Collection("dataPhis", dphis);
        auto dtobWord0s = Monitored::Collection("dataWord0s", dword0s);
        auto stobEtas = Monitored::Collection("simEtas", setas);
        auto stobPhis = Monitored::Collection("simPhis", sphis);
        auto stobWord0s = Monitored::Collection("simWord0s", sword0s);
        fillVectors(key2,ctx,detas,dphis,dword0s);
        fillVectors(key1,ctx,setas,sphis,sword0s);


        std::set<uint32_t> word0s1;
        for(const auto tob : *tobs1) {
            word0s1.insert(tob->word0());
        }

        auto locIdx = Monitored::Scalar<std::string>("locIdx","");
        for(size_t i = 0; i < tobs1->size();i++) {
            if(word0s2.find(tobs1->at(i)->word0())==word0s2.end()) {
                locIdx = std::to_string(tobs1->at(i)->shelfNumber()*12+tobs1->at(i)->eFexNumber()) + ":" + std::to_string(tobs1->at(i)->fpga()) + ":" +
                        std::to_string(tobs1->at(i)->iEta()) + ":" + std::to_string(tobs1->at(i)->iPhi());
                fill(signa + "_mismatches",lbn,locIdx,simReady,IsDataTowers,IsEmulatedTowers);
            }
        }
        for(size_t i = 0; i < tobs2->size();i++) {
            if(word0s1.find(tobs2->at(i)->word0())==word0s1.end()) {
                locIdx = std::to_string(tobs2->at(i)->shelfNumber()*12+tobs2->at(i)->eFexNumber()) + ":" + std::to_string(tobs2->at(i)->fpga()) + ":" +
                        std::to_string(tobs2->at(i)->iEta()) + ":" + std::to_string(tobs2->at(i)->iPhi());
                fill(signa + "_mismatches",lbn,locIdx,simReady,IsDataTowers,IsEmulatedTowers);
            }
        }

        tobMismatched=100;
        auto simReadyMismatch = Monitored::Scalar<bool>("SimulationReadyMismatch",simReady);
        ATH_MSG_WARNING(signa << " mismatch in lbn " << lbn << " evtNumber " << evtNumber);
        if(msgLvl(MSG::DEBUG)) {
            std::stringstream s;
            s << "Data: " << std::hex;
            for(auto w : dword0s) s << w << " ";
            ATH_MSG_DEBUG(s.str());
            s.str("");
            s << "Simu: " << std::hex;
            for(auto w : sword0s) s << w << " ";
            ATH_MSG_DEBUG(s.str());
        }
        auto signatureEvtType = Monitored::Scalar<std::string>("SignatureEvtType",signa+":"+static_cast<std::string>(evtType));
        fill("mismatches",signatureEvtType,simReadyMismatch,tobMismatched,lbn,lbnString,evtNumber,dtobEtas,dtobPhis,dtobWord0s,stobEtas,stobPhis,stobWord0s,evtType,timeSince,timeUntil,IsDataTowers,IsEmulatedTowers,signature,simReady);
    } else {
        tobMismatched=0;
        fill("mismatches",tobMismatched,lbn,signature,simReady,evtType);
    }


    return mismatches;

}
