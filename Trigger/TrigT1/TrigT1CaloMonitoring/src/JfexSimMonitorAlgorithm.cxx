/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
    ATH_CHECK( m_scellKey.initialize()      );

    ATH_CHECK( m_bcContKey.initialize() );
    
    

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
        if (timeUntil>=0 && timeUntil<=5) {
            EventType += "+JustBeforeOTF";
        }

        // check if any supercells are missing, there should be 34048
        SG::ReadHandle<CaloCellContainer> scells(m_scellKey,ctx);
        if (!scells.isValid() || scells->size() != 34048) {
            EventType += "+MissingSCells";
        }
    }


    //maximum number of TOBs (not xTOBs) that fit on the realtime path in hardware
    static constexpr int jJmaxTobs   = 7;
    static constexpr int jTAUmaxTobs = 6;
    static constexpr int jEMmaxTobs  = 5;
    bool simReady = !jFexTowerContainer->empty();
    compareRoI("jJ",EventType,m_data_key_jJ, m_simu_key_jJ,ctx,simReady,jJmaxTobs);
    //compareRoI("jLJ",EventType,m_data_key_jLJ, m_simu_key_jLJ,ctx,false); - commented out b.c. jFEX doesn't produce Large jets now
    compareRoI("jTAU",EventType,m_data_key_jTau, m_simu_key_jTau,ctx,simReady,jTAUmaxTobs);
    compareRoI("jEM",EventType,m_data_key_jEM, m_simu_key_jEM,ctx,simReady,jEMmaxTobs);
    compareRoI("jXE",EventType,m_data_key_jXE, m_simu_key_jXE,ctx,simReady);
    compareRoI("jTE",EventType,m_data_key_jTE, m_simu_key_jTE,ctx,simReady);


    return StatusCode::SUCCESS;
}

template<typename T> uint16_t tobEt(const T* tob) { return tob->tobEt(); }
template<> uint16_t tobEt(const xAOD::jFexMETRoI*) { return 0; }
template<> uint16_t tobEt(const xAOD::jFexSumETRoI*) { return 0; }

template <typename T> bool JfexSimMonitorAlgorithm::compareRoI(const std::string& label, const std::string& evenType,
                                            const SG::ReadHandleKey<T>& tobsDataKey,
                                            const SG::ReadHandleKey<T>& tobsSimKey,
                                            const EventContext& ctx, bool simReadyFlag, size_t maxTobs) const {
    SG::ReadHandle<T> tobsDataCont{tobsDataKey, ctx};
    if(!tobsDataCont.isValid()) {
        return false;
    }
    SG::ReadHandle<T> tobsSimCont{tobsSimKey, ctx};
    if(!tobsSimCont.isValid()) {
        return false;
    }

    bool mismatches = false;

    auto eventType = Monitored::Scalar<std::string>("EventType",evenType);
    auto Signature = Monitored::Scalar<std::string>("Signature",label);
    auto signatureEventType = Monitored::Scalar<std::string>("SignatureEventType",label+":"+evenType);
    auto tobMismatched = Monitored::Scalar<double>("tobMismatched",0);
    auto simReady = Monitored::Scalar<bool>("SimulationReady",simReadyFlag);
    auto IsDataTowers = Monitored::Scalar<bool>("IsDataTowers",evenType=="DataTowers");
    auto IsEmulatedTowers = Monitored::Scalar<bool>("IsEmulatedTowers",evenType=="EmulatedTowers");

    // saturation bits currently not properly simulated. But because they aren't used anywhere downstream
    // in the trigger, we will allow mismatches in these bits. 
    // The saturation bit is the lowest bit on all TOBs except jTE where it is also the highest bit (2 bits):
    auto mask = (label=="jTE") ? 0x7FFFFFFE : 0xFFFFFFFE;

    // if have a max tobs count, need to see if we reached max count in any fpga in any jfex module
    // if we did, then we allow mismatches of any tob that has the min et
    std::map<std::pair<uint8_t,uint8_t>,std::multiset<uint16_t>> tobEts_byFpga;

    unsigned zeroTobs1 = 0;
    unsigned zeroTobs2 = 0;
    for(const auto tob1 : *tobsDataCont) {
        bool isMatched = false;
        auto word1 = tob1->tobWord();
        auto jfex1 = tob1->jFexNumber();
        auto fpga1 = tob1->fpgaNumber();
        
        for (const auto tob2 : *tobsSimCont) {
            if(word1==0 || ((word1&mask) == (tob2->tobWord()&mask) && jfex1 == tob2->jFexNumber() && fpga1 == tob2->fpgaNumber())) { // do not flag as mismatch if the TOB word is zero, it might simply be (zero) suppressed in the other container!
                isMatched = true;
                break;
            }
        }
        if(!isMatched) {
            // if this signature has a max number of TOBs (in an FPGA),
            // possible the mismatch is an ambiguity in the lowest ET TOB
            // so treat as a match if this data TOB has same ET as the lowest ET sim TOB in the same FPGA
            if(maxTobs>0) {
                // first populate the fpga->tobs map with simulation tobs if it hasn't already been filled
                if(tobEts_byFpga.empty()) {
                    for (const auto tob: *tobsSimCont) {
                        // use of multiset ensures all the TOBs are automatically ordered by ET
                        // the first tob in the multiset will be the lowest ET tob.
                        tobEts_byFpga[std::pair(tob->jFexNumber(), tob->fpgaNumber())].insert(tobEt(tob));
                    }
                }
                // now check if the FPGA that produced the data TOB reached its max number of TOBs and
                // the lowest ET TOB has the same ET
                if(auto itr = tobEts_byFpga.find(std::pair(jfex1,fpga1)); itr != tobEts_byFpga.end()
                    && itr->second.size() == maxTobs       // number of TOBs in the FPGA reached the maximum
                    && (*itr->second.begin())==tobEt(tob1) // tob has same ET as lowest ET TOB (first tob in the multiset)
                    ) {
                    // possible ambiguity ... treat as a match
                    isMatched = true;
                }
            }
            if(!isMatched) {
                mismatches = true;
            }
        }
        if (word1 == 0) {
            zeroTobs1++;
        }
    }
    
    for (const auto tob2: *tobsSimCont) {
        if (tob2->tobWord() == 0) {
          zeroTobs2++;
        }
    }

    // check for extra non-zero sim tobs (compared to number of non-zero data tobs)
    if(tobsSimCont.isValid() && (tobsDataCont->size() - zeroTobs1) < (tobsSimCont->size() - zeroTobs2) ) {
        mismatches=true;
    }

    auto lbn = Monitored::Scalar<ULong64_t>("LBN",GetEventInfo(ctx)->lumiBlock());
    if(mismatches) {
        // fill the debugging tree with all the words for this signature
        auto lbnString = Monitored::Scalar<std::string>("LBNString",std::to_string(GetEventInfo(ctx)->lumiBlock()));
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
        fillVectors(tobsDataKey,ctx,detas,dphis,dword0s);
        fillVectors(tobsSimKey,ctx,setas,sphis,sword0s);
        if(msgLvl(MSG::DEBUG)) {
            std::cout << "LBN: " << std::string(lbnString) << " EventNumber: " << ULong64_t(evtNumber) << " signature: " << label << std::endl;
            std::cout << "  data : " << std::hex;
            for (const auto w: dword0s) std::cout << w << " ";
            std::cout << std::endl << "  sim  : ";
            for (const auto w: sword0s) std::cout << w << " ";
            std::cout << std::endl << std::dec;
        }
        tobMismatched=100;
        auto simReadyMismatch = Monitored::Scalar<bool>("SimulationReadyMismatch",simReady);
        fill("mismatches",simReadyMismatch,tobMismatched,lbn,lbnString,evtNumber,dtobEtas,dtobPhis,dtobWord0s,stobEtas,stobPhis,stobWord0s,Signature,eventType,IsDataTowers,IsEmulatedTowers,simReady,signatureEventType);
    } else {
        tobMismatched=0;
        fill("mismatches",lbn,Signature,tobMismatched,simReady,eventType);
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
        for(const xAOD::jFexMETRoI* tob : *tobs) {
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
