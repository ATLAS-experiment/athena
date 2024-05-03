/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "JfexSimMonitorAlgorithm.h"

JfexSimMonitorAlgorithm::JfexSimMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator ) : AthMonitorAlgorithm(name,pSvcLocator) {}

StatusCode JfexSimMonitorAlgorithm::initialize() {

    ATH_MSG_DEBUG("Initializing JfexSimMonitorAlgorithm algorithm with name: "<< name());


    ATH_MSG_DEBUG("m_data_key_jJ "   << m_data_key_jJ   );
    ATH_MSG_DEBUG("m_data_key_jLJ "  << m_data_key_jLJ  );
    ATH_MSG_DEBUG("m_data_key_jTau " << m_data_key_jTau );
    ATH_MSG_DEBUG("m_data_key_jEM "  << m_data_key_jEM  );
    ATH_MSG_DEBUG("m_data_key_jXE "  << m_data_key_jXE  );
    ATH_MSG_DEBUG("m_data_key_jTE "  << m_data_key_jTE  );    

    ATH_MSG_DEBUG("m_simu_key_jJ "   << m_simu_key_jJ   );
    ATH_MSG_DEBUG("m_simu_key_jLJ "  << m_simu_key_jLJ  );
    ATH_MSG_DEBUG("m_simu_key_jTau " << m_simu_key_jTau );
    ATH_MSG_DEBUG("m_simu_key_jEM "  << m_simu_key_jEM  );
    ATH_MSG_DEBUG("m_simu_key_jXE "  << m_simu_key_jXE  );
    ATH_MSG_DEBUG("m_simu_key_jTE "  << m_simu_key_jTE  );    
    

    // we initialise all the containers
    ATH_CHECK( m_data_key_jJ.initialize()   );
    ATH_CHECK( m_data_key_jLJ.initialize()  );
    ATH_CHECK( m_data_key_jTau.initialize() );
    ATH_CHECK( m_data_key_jEM.initialize()  );
    ATH_CHECK( m_data_key_jXE.initialize()  );
    ATH_CHECK( m_data_key_jTE.initialize()  );

    ATH_CHECK( m_simu_key_jJ.initialize()   );
    ATH_CHECK( m_simu_key_jLJ.initialize()  );
    ATH_CHECK( m_simu_key_jTau.initialize() );
    ATH_CHECK( m_simu_key_jEM.initialize()  );
    ATH_CHECK( m_simu_key_jXE.initialize()  );
    ATH_CHECK( m_simu_key_jTE.initialize()  );
    
    ATH_CHECK( m_jFexTowerKey.initialize()  );

    ATH_CHECK( m_bcContKey.initialize() );
    

    // TOBs may come from trigger bytestream - renounce from scheduler
    renounce(m_data_key_jJ);
    renounce(m_data_key_jLJ);
    renounce(m_data_key_jTau);
    renounce(m_data_key_jEM);
    renounce(m_data_key_jXE);
    renounce(m_data_key_jTE);  
    

    return AthMonitorAlgorithm::initialize();
}

StatusCode JfexSimMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

    ATH_MSG_DEBUG("JfexMonitorAlgorithm::fillHistograms");
    
    SG::ReadHandle<xAOD::jFexTowerContainer> jFexTowerContainer{m_jFexTowerKey, ctx};
    if(!jFexTowerContainer.isValid()) {
        ATH_MSG_ERROR("No jFex Tower container found in storegate  "<< m_jFexTowerKey);
        return StatusCode::SUCCESS;
    }
    
    std::string inputTower = jFexTowerContainer->empty() ? "EmulatedTowers" : "DataTowers";

    // mismatches can be caused by recent/imminent OTF maskings, so track timings
    auto timeSince = Monitored::Scalar<int>("timeSince", -1);
    auto timeUntil = Monitored::Scalar<int>("timeUntil", -1);
    SG::ReadCondHandle<LArBadChannelCont> larBadChan{ m_bcContKey, ctx };
    if(larBadChan.isValid()) {
        timeSince = ctx.eventID().time_stamp() - larBadChan.getRange().start().time_stamp();
        timeUntil = larBadChan.getRange().stop().time_stamp() - ctx.eventID().time_stamp();
    }
    auto EventType = Monitored::Scalar<std::string>("EventType","DataTowers");
    if(jFexTowerContainer->empty()) {
        EventType = "EmulatedTowers";
    }

    const std::string LB = std::to_string(GetEventInfo(ctx)->lumiBlock());
    auto lbnString = Monitored::Scalar<std::string>("LBNString",LB);
    auto evtNumber = Monitored::Scalar<ULong64_t>("EventNumber",GetEventInfo(ctx)->eventNumber());


    compareRoI("jJ",EventType,m_data_key_jJ, m_simu_key_jJ,ctx,!jFexTowerContainer->empty());
    //compareRoI("jLJ",EventType,m_data_key_jLJ, m_simu_key_jLJ,ctx,false); - commented out b.c. jFEX doesn't produce Large jets now
    compareRoI("jTAU",EventType,m_data_key_jTau, m_simu_key_jTau,ctx,!jFexTowerContainer->empty());
    compareRoI("jEM",EventType,m_data_key_jEM, m_simu_key_jEM,ctx,false);
    compareRoI("jXE",EventType,m_data_key_jXE, m_simu_key_jXE,ctx,!jFexTowerContainer->empty());
    compareRoI("jTE",EventType,m_data_key_jTE, m_simu_key_jTE,ctx,!jFexTowerContainer->empty());


    return StatusCode::SUCCESS;
}

template <typename T> bool JfexSimMonitorAlgorithm::compareRoI(const std::string& label, const std::string& evenType,
                                            const SG::ReadHandleKey<T>& tobs1Key,
                                            const SG::ReadHandleKey<T>& tobs2Key,
                                            const EventContext& ctx, bool simReadyFlag) const {
    SG::ReadHandle<T> tobs1Cont{tobs1Key, ctx};
    if(!tobs1Cont.isValid()) {
        return false;
    }
    SG::ReadHandle<T> tobs2Cont{tobs2Key, ctx};
    if(!tobs2Cont.isValid()) {
        return false;
    }

    bool mismatches = (tobs1Cont->size()!=tobs2Cont->size());

    auto eventType = Monitored::Scalar<std::string>("EventType",evenType);
    auto Signature = Monitored::Scalar<std::string>("Signature",label);
    auto tobMismatched = Monitored::Scalar<double>("tobMismatched",0);
    auto simReady = Monitored::Scalar<bool>("SimulationReady",simReadyFlag);
    auto IsDataTowers = Monitored::Scalar<bool>("IsDataTowers",evenType=="DataTowers");
    auto IsEmulatedTowers = Monitored::Scalar<bool>("IsEmulatedTowers",!IsDataTowers);

    for(const auto tob1 : *tobs1Cont) {
        bool isMatched = false;
        auto word1 = tob1->tobWord();
        auto jfex1 = tob1->jFexNumber();
        auto fpga1 = tob1->fpgaNumber();

        for (const auto tob2 : *tobs2Cont) {
            if(word1==0 || (word1 == tob2->tobWord() && jfex1 == tob2->jFexNumber() && fpga1 == tob2->fpgaNumber())) { // not 100% sure about the word1=0 skip, but came from old code comparing global RoI
                isMatched = true;
                break;
            }
        }
        if(!isMatched) {
            mismatches = true;
        }
        tobMismatched = (isMatched) ? 0 : 100;
        fill("mismatches",eventType,Signature,tobMismatched);
    }
    if(tobs2Cont.isValid() && tobs1Cont->size() < tobs2Cont->size()) {
        tobMismatched=100;
        mismatches=true;
        for(unsigned int i=0;i<(tobs2Cont->size()-tobs1Cont->size());i++) {
            fill("mismatches",eventType,Signature,tobMismatched);
        }
    }

    if(mismatches) {
        // fill the debugging tree with all the words for this signature
        auto lbnString = Monitored::Scalar<std::string>("LBNString",std::to_string(GetEventInfo(ctx)->lumiBlock()));
        auto lbn = Monitored::Scalar<ULong64_t>("LBN",GetEventInfo(ctx)->lumiBlock());
        auto evtNumber = Monitored::Scalar<ULong64_t>("EventNumber",GetEventInfo(ctx)->eventNumber());
        {
            std::scoped_lock lock(m_firstEventsMutex);
            auto itr = m_firstEvents.find(lbn);
            if(itr==m_firstEvents.end()) {
                m_firstEvents[lbn] = std::to_string(lbn)+":"+std::to_string(evtNumber);
                itr = m_firstEvents.find(lbn);
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
        fillVectors(tobs1Key,ctx,detas,dphis,dword0s);
        fillVectors(tobs2Key,ctx,setas,sphis,sword0s);
        if(msgLvl(MSG::DEBUG)) {
            std::cout << "LBN: " << std::string(lbnString) << " EventNumber: " << ULong64_t(evtNumber) << " signature: " << label << std::endl;
            std::cout << "  data : " << std::hex;
            for (const auto w: dword0s) std::cout << w << " ";
            std::cout << std::endl << "  sim  : ";
            for (const auto w: sword0s) std::cout << w << " ";
            std::cout << std::endl << std::dec;
        }
        fill("mismatches",lbn,lbnString,evtNumber,dtobEtas,dtobPhis,dtobWord0s,stobEtas,stobPhis,stobWord0s,Signature,eventType,IsDataTowers,IsEmulatedTowers,simReady);
    }

    return !mismatches;

}

template <> void JfexSimMonitorAlgorithm::fillVectors(const SG::ReadHandleKey<xAOD::jFexMETRoIContainer>& key, const EventContext& ctx, std::vector<float>& etas, std::vector<float>& phis, std::vector<unsigned int>& word0s) const {
    etas.clear();phis.clear();word0s.clear();
    SG::ReadHandle<xAOD::jFexMETRoIContainer> tobs{key, ctx};
    if(tobs.isValid()) {
        etas.reserve(tobs->size());
        phis.reserve(tobs->size());
        word0s.reserve(tobs->size());
        std::vector<SortableTob> sortedTobs;
        sortedTobs.reserve(tobs->size());
        for(const auto& tob : *tobs) {
            sortedTobs.emplace_back(SortableTob{tob->tobWord(),0.,0.});
        }
        std::sort(sortedTobs.begin(),sortedTobs.end(),[](const SortableTob& lhs, const SortableTob& rhs) { return lhs.word0<rhs.word0; });
        for(const auto& tob : sortedTobs) {
            etas.push_back(tob.eta);
            phis.push_back(tob.phi);
            word0s.push_back(tob.word0);
        }
    }
}
template <> void JfexSimMonitorAlgorithm::fillVectors(const SG::ReadHandleKey<xAOD::jFexSumETRoIContainer>& key, const EventContext& ctx, std::vector<float>& etas, std::vector<float>& phis, std::vector<unsigned int>& word0s) const {
    etas.clear();phis.clear();word0s.clear();
    SG::ReadHandle<xAOD::jFexSumETRoIContainer> tobs{key, ctx};
    if(tobs.isValid()) {
        etas.reserve(tobs->size());
        phis.reserve(tobs->size());
        word0s.reserve(tobs->size());
        std::vector<SortableTob> sortedTobs;
        sortedTobs.reserve(tobs->size());
        for(const auto tob : *tobs) {
            sortedTobs.emplace_back(SortableTob{tob->tobWord(),0.,0.});
        }
        std::sort(sortedTobs.begin(),sortedTobs.end(),[](const SortableTob& lhs, const SortableTob& rhs) { return lhs.word0<rhs.word0; });
        for(const auto& tob : sortedTobs) {
            etas.push_back(tob.eta);
            phis.push_back(tob.phi);
            word0s.push_back(tob.word0);
        }
    }
}