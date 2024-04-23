/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "GfexSimMonitorAlgorithm.h"

GfexSimMonitorAlgorithm::GfexSimMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator ) : AthMonitorAlgorithm(name,pSvcLocator) {}

StatusCode GfexSimMonitorAlgorithm::initialize() {

    ATH_MSG_DEBUG("Initializing GfexSimMonitorAlgorithm algorithm with name: "<< name());

    ATH_MSG_DEBUG("m_data_key_gFexRho" << m_data_gFexRho);
    ATH_MSG_DEBUG("m_data_gFexBlock" << m_data_gFexBlock );
    ATH_MSG_DEBUG("m_data_gFexJet" << m_data_gFexJet );
    ATH_MSG_DEBUG("m_data_gScalarEJwoj" << m_data_gScalarEJwoj );
    ATH_MSG_DEBUG("m_data_gMETComponentsJwoj" << m_data_gMETComponentsJwoj );
    ATH_MSG_DEBUG("m_data_gMHTComponentsJwoj" <<  m_data_gMHTComponentsJwoj );
    ATH_MSG_DEBUG("m_data_gMSTComponentsJwoj" << m_data_gMSTComponentsJwoj );
    ATH_MSG_DEBUG("m_data_gMETComponentsNoiseCut" << m_data_gMETComponentsNoiseCut );
    ATH_MSG_DEBUG("m_data_gMETComponentsRms" << m_data_gMETComponentsRms );
    ATH_MSG_DEBUG("m_data_gScalarENoiseCut" << m_data_gScalarENoiseCut );
    ATH_MSG_DEBUG("m_data_gScalarERms" << m_data_gScalarERms );


    ATH_MSG_DEBUG("m_simu_key_gFexRho" << m_simu_gFexRho);
    ATH_MSG_DEBUG("m_simu_gFexBlock" << m_simu_gFexBlock );
    ATH_MSG_DEBUG("m_simu_gFexJet" << m_simu_gFexJet );
    ATH_MSG_DEBUG("m_simu_gScalarEJwoj" << m_simu_gScalarEJwoj );
    ATH_MSG_DEBUG("m_simu_gMETComponentsJwoj" << m_simu_gMETComponentsJwoj );
    ATH_MSG_DEBUG("m_simu_gMHTComponentsJwoj" <<  m_simu_gMHTComponentsJwoj );
    ATH_MSG_DEBUG("m_simu_gMSTComponentsJwoj" << m_simu_gMSTComponentsJwoj );
    ATH_MSG_DEBUG("m_simu_gMETComponentsNoiseCut" << m_simu_gMETComponentsNoiseCut );
    ATH_MSG_DEBUG("m_simu_gMETComponentsRms" << m_simu_gMETComponentsRms );
    ATH_MSG_DEBUG("m_simu_gScalarENoiseCut" << m_simu_gScalarENoiseCut );
    ATH_MSG_DEBUG("m_simu_gScalarERms" << m_simu_gScalarERms );


/*    ATH_MSG_DEBUG("m_data_key_gGlob "  << m_data_key_gGlob  );
    ATH_MSG_DEBUG("m_data_key_gJ "  << m_data_key_gJ  );

    ATH_MSG_DEBUG("m_simu_key_gGlob "   << m_simu_key_gGlob   );
    ATH_MSG_DEBUG("m_simu_key_gJ "  << m_simu_key_gJ  );
*/


    // we initialise all the containers
    ATH_CHECK ( m_data_gFexRho.initialize() );
    ATH_CHECK ( m_data_gFexBlock.initialize() );
    ATH_CHECK ( m_data_gFexJet.initialize() );
    ATH_CHECK ( m_data_gScalarEJwoj.initialize() );
    ATH_CHECK ( m_data_gMETComponentsJwoj.initialize() );
    ATH_CHECK ( m_data_gMHTComponentsJwoj.initialize() );
    ATH_CHECK ( m_data_gMSTComponentsJwoj.initialize() );
    ATH_CHECK ( m_data_gMETComponentsNoiseCut.initialize() );
    ATH_CHECK ( m_data_gMETComponentsRms.initialize() );
    ATH_CHECK ( m_data_gScalarENoiseCut.initialize() );
    ATH_CHECK ( m_data_gScalarERms.initialize() );


    ATH_CHECK ( m_simu_gFexRho.initialize() );
    ATH_CHECK ( m_simu_gFexBlock.initialize() );
    ATH_CHECK ( m_simu_gFexJet.initialize() );
    ATH_CHECK ( m_simu_gScalarEJwoj.initialize() );
    ATH_CHECK ( m_simu_gMETComponentsJwoj.initialize() );
    ATH_CHECK ( m_simu_gMHTComponentsJwoj.initialize() );
    ATH_CHECK ( m_simu_gMSTComponentsJwoj.initialize() );
    ATH_CHECK ( m_simu_gMETComponentsNoiseCut.initialize() );
    ATH_CHECK ( m_simu_gMETComponentsRms.initialize() );
    ATH_CHECK ( m_simu_gScalarENoiseCut.initialize() );
    ATH_CHECK ( m_simu_gScalarERms.initialize() );

    ATH_CHECK( m_bcContKey.initialize() );



    // TOBs may come from trigger bytestream - renounce from scheduler


    renounce ( m_data_gFexRho );
    renounce ( m_data_gFexBlock );
    renounce ( m_data_gFexJet );
    renounce ( m_data_gScalarEJwoj );
    renounce ( m_data_gMETComponentsJwoj );
    renounce ( m_data_gMHTComponentsJwoj );
    renounce ( m_data_gMSTComponentsJwoj );
    renounce ( m_data_gMETComponentsNoiseCut );
    renounce ( m_data_gMETComponentsRms );
    renounce ( m_data_gScalarENoiseCut );
    renounce ( m_data_gScalarERms );


    return AthMonitorAlgorithm::initialize();
}

StatusCode GfexSimMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

    compareJetRoI("gLJ",m_data_gFexJet,m_simu_gFexJet,ctx,true);
    compareJetRoI("gJ",m_data_gFexBlock,m_simu_gFexBlock,ctx,true);
    compareJetRoI("gLJRho",m_data_gFexRho,m_simu_gFexRho,ctx);
    compareGlobalRoI("gXEJWOJ",m_data_gScalarEJwoj,m_simu_gScalarEJwoj,ctx);
    compareGlobalRoI("gXEJWOJ",m_data_gMETComponentsJwoj,m_simu_gMETComponentsJwoj,ctx);
    compareGlobalRoI("gXEJWOJ",m_data_gMHTComponentsJwoj,m_simu_gMHTComponentsJwoj,ctx);
    compareGlobalRoI("gXEJWOJ",m_data_gMSTComponentsJwoj,m_simu_gMSTComponentsJwoj,ctx);
    compareGlobalRoI("gXENC",m_data_gMETComponentsNoiseCut,m_simu_gMETComponentsNoiseCut,ctx);
    compareGlobalRoI("gXERHO",m_data_gMETComponentsRms,m_simu_gMETComponentsRms,ctx);
    compareGlobalRoI("gXENC",m_data_gScalarENoiseCut,m_simu_gScalarENoiseCut,ctx);
    compareGlobalRoI("gXERHO",m_data_gScalarERms,m_simu_gScalarERms,ctx);

    return StatusCode::SUCCESS;
}

bool GfexSimMonitorAlgorithm::compareJetRoI(const std::string& label,
                                            const SG::ReadHandleKey<xAOD::gFexJetRoIContainer>& tobs1Key,
                                            const SG::ReadHandleKey<xAOD::gFexJetRoIContainer>& tobs2Key,
                                            const EventContext& ctx, bool simReadyFlag) const {
    SG::ReadHandle<xAOD::gFexJetRoIContainer> tobs1Cont{tobs1Key, ctx};
    if(!tobs1Cont.isValid()) {
        return false;
    }
    SG::ReadHandle<xAOD::gFexJetRoIContainer> tobs2Cont{tobs2Key, ctx};
    if(!tobs1Cont.isValid()) {
        return false;
    }

    bool mismatches = (tobs1Cont->size()!=tobs2Cont->size());
    //bool mismatchesExlStatusAndSat = mismatches;

    auto eventType = Monitored::Scalar<std::string>("EventType","DataTowers"); // always have data towers
    auto Signature = Monitored::Scalar<std::string>("Signature",label);
    auto tobMismatched = Monitored::Scalar<double>("tobMismatched",0);
    auto simReady = Monitored::Scalar<bool>("SimulationReady",simReadyFlag);

    for(const auto tob1 : *tobs1Cont) {
        bool isMatched = false;
        //bool isPartMatched = false;
        auto word1 = tob1->word();
        auto gfex1 = tob1->gFexType();
        for (auto tob2 : *tobs2Cont) {
            if(gfex1 == tob2->gFexType()) {
                if(word1 == tob2->word()) {
                    isMatched = true;
                    break;
                } /*else if( (word1&0x7FFFFF7F) == (tob2->word()&0x7FFFFF7F) ) {
                    // matches after ignore saturation bit (31st bit) and status bit (7th bit) (first bit is 0th)
                    isPartMatched=true;
                }*/
            }
        }
        if(!isMatched) {
            mismatches = true;
        }
        tobMismatched = (isMatched) ? 0 : 100;
        fill("mismatches",eventType,Signature,tobMismatched);
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
        fill("mismatches",lbn,lbnString,evtNumber,dtobEtas,dtobPhis,dtobWord0s,stobEtas,stobPhis,stobWord0s,Signature,simReady);
    }

    return !mismatches;

}


bool GfexSimMonitorAlgorithm::compareGlobalRoI(const std::string& label,
                                            const SG::ReadHandleKey<xAOD::gFexGlobalRoIContainer>& tobs1Key,
                                            const SG::ReadHandleKey<xAOD::gFexGlobalRoIContainer>& tobs2Key,
                                            const EventContext& ctx) const {
    SG::ReadHandle<xAOD::gFexGlobalRoIContainer> tobs1Cont{tobs1Key, ctx};
    if(!tobs1Cont.isValid()) {
        return false;
    }
    SG::ReadHandle<xAOD::gFexGlobalRoIContainer> tobs2Cont{tobs2Key, ctx};
    if(!tobs1Cont.isValid()) {
        return false;
    }

    auto eventType = Monitored::Scalar<std::string>("EventType","DataTowers"); // always have data towers
    auto Signature = Monitored::Scalar<std::string>("Signature",label);
    auto tobMismatched = Monitored::Scalar<double>("tobMismatched",0);


    bool mismatches = (tobs1Cont->size()!=tobs2Cont->size());
    //bool mismatchesExlStatusAndSat = mismatches;

    for(const auto tob1 : *tobs1Cont) {
        bool isMatched = false;
        bool isPartMatched = false;
        auto word1 = tob1->word();
        auto gfex1 = tob1->globalType();
        for (auto tob2 : *tobs2Cont) {
            if(gfex1 == tob2->globalType()) {
                if(word1 == tob2->word()) {
                    isMatched = true;
                    break;
                } else if( (word1&0x7CFFFFFF) == (tob2->word()&0x7CFFFFFF) ) {
                    // matches after ignore saturation bit (31st bit) and status bits (24th and 25 bit)
                    isPartMatched=true;
                }
            }

        }
        if(!isMatched) {
            mismatches = true;
            if(!isPartMatched) {
                //mismatchesExlStatusAndSat = true;
            }
        }
        tobMismatched = (isMatched) ? 0 : 100; //100*(!isMatched && !isPartMatched); - commented out. Is from when was treating part-matches as matches
        fill("mismatches",eventType,Signature,tobMismatched);
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
        std::vector<float> detas{};std::vector<float> setas{}; // will be empty, b.c. meaningless for global TOBs
        std::vector<float> dphis{};std::vector<float> sphis{};
        std::vector<unsigned int> dword0s{};std::vector<unsigned int> sword0s{};
        for(const auto tob1 : *tobs1Cont) {
            dword0s.push_back(tob1->word());
        }
        for(const auto tob2 : *tobs2Cont) {
            sword0s.push_back(tob2->word());
        }
        auto dtobEtas = Monitored::Collection("dataEtas", detas);
        auto dtobPhis = Monitored::Collection("dataPhis", dphis);
        auto dtobWord0s = Monitored::Collection("dataWord0s", dword0s);
        auto stobEtas = Monitored::Collection("simEtas", setas);
        auto stobPhis = Monitored::Collection("simPhis", sphis);
        auto stobWord0s = Monitored::Collection("simWord0s", sword0s);
        if(msgLvl(MSG::DEBUG)) {
            std::cout << "LBN: " << std::string(lbnString) << " EventNumber: " << ULong64_t(evtNumber) << " signature: " << label << std::endl;
            std::cout << "  data : " << std::hex;
            for (const auto w: dword0s) std::cout << w << " ";
            std::cout << std::endl << "  sim  : ";
            for (const auto w: sword0s) std::cout << w << " ";
            std::cout << std::endl << std::dec;
        }
        fill("mismatches",lbn,lbnString,evtNumber,dtobEtas,dtobPhis,dtobWord0s,stobEtas,stobPhis,stobWord0s,Signature);
    }


    return !mismatches;

}
