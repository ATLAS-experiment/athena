/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           jFexRoiByteStreamTool  -  This tool decodes Run3 jFEX RoI data!
//                              -------------------
//     begin                : 01 01 2022
//     email                : Sergi.Rodriguez@cern.ch
//  ***************************************************************************/

#include "jFexRoiByteStreamTool.h"
#include "jFexBits.h"
#include "CxxUtils/span.h"
#include "eformat/SourceIdentifier.h"
#include "eformat/Status.h"

#include <fstream>

using ROBF = OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;
using WROBF = OFFLINE_FRAGMENTS_NAMESPACE_WRITE::ROBFragment;

namespace jBits = LVL1::jFEXBits;

struct color {
    std::string RED      ="\033[1;31m";
    std::string ORANGE   ="\033[1;38;5;208m";
    std::string YELLOW   ="\033[1;33m";
    std::string GREEN    ="\033[1;32m";
    std::string BLUE     ="\033[1;34m";
    std::string PURPLE   ="\033[1;35m";
    std::string END      ="\033[0m";
    std::string B_BLUE   ="\033[1;44m";
    std::string B_PURPLE ="\033[1;45m";
    std::string B_ORANGE ="\033[1;48;5;208;30m";
    std::string B_GRAY   ="\033[1;100m";
    std::string B_RED    ="\033[1;41m";
    std::string B_GREEN  ="\033[1;42m";
} const C;

namespace {
    // Per-(jfex,fpga) bucket of TOB / xTOB raw words for encoder serialisation.
    // jXE / jTE only have non-empty vectors when fpga is U1 or U4 (firmware constraint).
    struct FpgaBlock {
        std::vector<uint32_t> jJ, jLJ, jTau, jEM;
        std::vector<uint32_t> xjJ, xjLJ, xjTau, xjEM;
        std::vector<uint32_t> jTE, jXE;
        void addTob(const xAOD::jFexSRJetRoI&  t) { (t.isTOB() ? jJ   : xjJ  ).push_back(t.tobWord()); }
        void addTob(const xAOD::jFexLRJetRoI&  t) { (t.isTOB() ? jLJ  : xjLJ ).push_back(t.tobWord()); }
        void addTob(const xAOD::jFexTauRoI&    t) { (t.isTOB() ? jTau : xjTau).push_back(t.tobWord()); }
        void addTob(const xAOD::jFexFwdElRoI&  t) { (t.isTOB() ? jEM  : xjEM ).push_back(t.tobWord()); }
        void addTob(const xAOD::jFexSumETRoI&  t) { jTE.push_back(t.tobWord()); }
        void addTob(const xAOD::jFexMETRoI&    t) { jXE.push_back(t.tobWord()); }
    };

    // (jfex, fpga) -> bucket. std::map sorts by < so iteration order is deterministic.
    using FragmentMap = std::map<std::pair<uint32_t,uint32_t>, FpgaBlock>;

    template<typename T>
    void bucketContainer(const xAOD::TrigComposite& l1, FragmentMap& blocks) {
        using ContT = DataVector<T>;
        for (const std::string& name : l1.getObjectNames<ContT>()) {
            auto link = l1.objectLink<ContT>(name);
            if (!link.isValid()) continue;
            const ContT* c = link.getStorableObjectPointer();
            if (!c) continue;
            for (const T* tob : *c) {
                blocks[{tob->jFexNumber(), tob->fpgaNumber()}].addTob(*tob);
            }
        }
    }
}

jFexRoiByteStreamTool::jFexRoiByteStreamTool(const std::string& type,
        const std::string& name,
        const IInterface* parent)
    : base_class(type, name, parent) {}

StatusCode jFexRoiByteStreamTool::initialize() {
    // Decoder write keys (BS->xAOD); each one is initialised only if configured.
    ATH_CHECK(m_jJWriteKey  .initialize(!m_jJWriteKey  .empty()));
    ATH_CHECK(m_jLJWriteKey .initialize(!m_jLJWriteKey .empty()));
    ATH_CHECK(m_jTauWriteKey.initialize(!m_jTauWriteKey.empty()));
    ATH_CHECK(m_jEMWriteKey .initialize(!m_jEMWriteKey .empty()));
    ATH_CHECK(m_jTEWriteKey .initialize(!m_jTEWriteKey .empty()));
    ATH_CHECK(m_jXEWriteKey .initialize(!m_jXEWriteKey .empty()));

    ATH_CHECK(m_l1MenuKey.initialize());
    

    //Reading from CVMFS TOB mapping
    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_TobMapping)));
    
    // Initialize monitoring tool if not empty
    if (!m_monTool.empty()) {
        ATH_CHECK(m_monTool.retrieve());
        ATH_MSG_INFO("Logging errors to " << m_monTool.name() << " monitoring tool");
        m_UseMonitoring = true;
    }
    
    return StatusCode::SUCCESS;
}

StatusCode jFexRoiByteStreamTool::start() {
    
    // Retrieve the L1 menu configuration
    SG::ReadHandle<TrigConf::L1Menu> l1Menu (m_l1MenuKey);
    ATH_CHECK(l1Menu.isValid());

    try {
        const auto& thrExtraInfo = l1Menu->thrExtraInfo();
        const TrigConf::L1ThrExtraInfo_jTAU & thr_jTAU = thrExtraInfo.jTAU(); 
        const TrigConf::L1ThrExtraInfo_jJ   & thr_jJ   = thrExtraInfo.jJ();  
        const TrigConf::L1ThrExtraInfo_jLJ  & thr_jLJ  = thrExtraInfo.jLJ(); 
        const TrigConf::L1ThrExtraInfo_jEM  & thr_jEM  = thrExtraInfo.jEM(); 
        const TrigConf::L1ThrExtraInfo_jTE  & thr_jTE  = thrExtraInfo.jTE(); 
        const TrigConf::L1ThrExtraInfo_jXE  & thr_jXE  = thrExtraInfo.jXE();    

        ATH_CHECK(thr_jTAU.isValid());
        ATH_CHECK(thr_jJ.isValid()  );
        ATH_CHECK(thr_jLJ.isValid() );
        ATH_CHECK(thr_jEM.isValid() );
        ATH_CHECK(thr_jTE.isValid() );
        ATH_CHECK(thr_jXE.isValid() );
        
        m_jTauRes = thr_jTAU.resolutionMeV();
        m_jJRes   = thr_jJ.resolutionMeV();
        m_jLJRes  = thr_jLJ.resolutionMeV();
        m_jEMRes  = thr_jEM.resolutionMeV();
        m_jXERes  = thr_jXE.resolutionMeV();
        m_jTERes  = thr_jTE.resolutionMeV();
    
    } catch (const std::exception& e) {
        ATH_MSG_ERROR("Exception reading L1Menu: " << e.what());
        return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
    
}


// BS->xAOD conversion
StatusCode jFexRoiByteStreamTool::convertFromBS(const std::vector<const ROBF*>& vrobf, const EventContext& ctx) const {
    
    //WriteHandle for jFEX EDMs
    
    //---SRJet EDM
    SG::WriteHandle<xAOD::jFexSRJetRoIContainer> jJContainer(m_jJWriteKey, ctx);
    ATH_CHECK(jJContainer.record(std::make_unique<xAOD::jFexSRJetRoIContainer>(), std::make_unique<xAOD::jFexSRJetRoIAuxContainer>()));
    ATH_MSG_DEBUG("Recorded jFexSRJetRoIContainer with key " << jJContainer.key());
    
    //---LRJet EDM
    SG::WriteHandle<xAOD::jFexLRJetRoIContainer> jLJContainer(m_jLJWriteKey, ctx);
    ATH_CHECK(jLJContainer.record(std::make_unique<xAOD::jFexLRJetRoIContainer>(), std::make_unique<xAOD::jFexLRJetRoIAuxContainer>()));
    ATH_MSG_DEBUG("Recorded jFexLRJetRoIContainer with key " << jLJContainer.key());    
    
    //---Tau EDM
    SG::WriteHandle<xAOD::jFexTauRoIContainer> jTauContainer(m_jTauWriteKey, ctx);
    ATH_CHECK(jTauContainer.record(std::make_unique<xAOD::jFexTauRoIContainer>(), std::make_unique<xAOD::jFexTauRoIAuxContainer>()));
    ATH_MSG_DEBUG("Recorded jFexTauRoIContainer with key " << jTauContainer.key());      
    
    //---EM EDM
    SG::WriteHandle<xAOD::jFexFwdElRoIContainer> jEMContainer(m_jEMWriteKey, ctx);
    ATH_CHECK(jEMContainer.record(std::make_unique<xAOD::jFexFwdElRoIContainer>(), std::make_unique<xAOD::jFexFwdElRoIAuxContainer>()));
    ATH_MSG_DEBUG("Recorded jFexFwdElRoIContainer with key " << jEMContainer.key());      
    
    //---SumET EDM
    SG::WriteHandle<xAOD::jFexSumETRoIContainer> jTEContainer(m_jTEWriteKey, ctx);
    ATH_CHECK(jTEContainer.record(std::make_unique<xAOD::jFexSumETRoIContainer>(), std::make_unique<xAOD::jFexSumETRoIAuxContainer>()));
    ATH_MSG_DEBUG("Recorded jFexSumETRoIContainer with key " << jTEContainer.key());      
    
    //---MET EDM 
    SG::WriteHandle<xAOD::jFexMETRoIContainer> jXEContainer(m_jXEWriteKey, ctx);
    ATH_CHECK(jXEContainer.record(std::make_unique<xAOD::jFexMETRoIContainer>(), std::make_unique<xAOD::jFexMETRoIAuxContainer>()));
    ATH_MSG_DEBUG("Recorded jFexMETRoIContainer with key " << jXEContainer.key());    
    
    
    

    // Iterate over ROBFragments to decode
    for (const ROBF* rob : vrobf) {
        // Iterate over ROD words and decode
        
        ATH_MSG_DEBUG("Starting to decode " << rob->rod_ndata() << " ROD words from ROB 0x" << std::hex << rob->rob_source_id() << std::dec);
        
        //There is no data to decode.. not even the ROD trailers
        if(rob->rod_ndata() <= 0){
            std::stringstream sdetail;
            sdetail  << "Not enough ROB words to read: "<<rob->rod_ndata() ;
            std::stringstream slocation;
            slocation  << "0x"<< std::hex << rob->rob_source_id();
            std::stringstream stitle;
            stitle  << "Invalid amount of ROB words" ;
            printError(slocation.str(),stitle.str(),MSG::WARNING,sdetail.str());             
            
            continue;
        }
        
        const auto dataArray = CxxUtils::span{rob->rod_data(), rob->rod_ndata()};
        std::vector<uint32_t> vec_words(dataArray.begin(),dataArray.end());
        
        std::stringstream myPrint;
        
        myPrint << "jFEX TOB words to decode:"<< std::endl;
        int aux = 1;
        for (const uint32_t word : vec_words) {
            myPrint << aux << " raw word ---> 0x"<< std::hex << word << std::dec << std::endl;
            aux++;
        }     
        ATH_MSG_DEBUG(myPrint.str());
           
        // Starting to loop over the different jFEX blocks
        bool READ_TOBS = true;
        // jFEX to ROD trailer position after the ROD trailer
        unsigned int trailers_pos = rob->rod_ndata() - jBits::ROD_WORDS;
        
        if(vec_words.size() < (jBits::ROD_WORDS+jBits::jFEX2ROD_WORDS) || trailers_pos < (jBits::ROD_WORDS+jBits::jFEX2ROD_WORDS) ){
            std::stringstream sdetail;
            sdetail  << "Not enough jFEX TOB words to decode (<4). Number of word to decode: "<<vec_words.size()<< ". Position within the vector: "<<trailers_pos ;
            std::stringstream slocation;
            slocation  << "0x"<< std::hex << rob->rob_source_id();
            std::stringstream stitle;
            stitle  << "Less than 4 trailers" ;
            printError(slocation.str(),stitle.str(),MSG::WARNING,sdetail.str());              
            
            continue;
        }
        
        //check ROD error
        const auto [RODerror] = RODTrailer  ( vec_words.at(rob->rod_ndata()-2), vec_words.at(rob->rod_ndata()-1) );
        
        // if ROD errors return gracefully
        if(RODerror != 0){
            bool corTrailer   = ((RODerror >> jBits::ROD_ERROR_CORR_TRAILER    ) & jBits::ROD_TRAILER_1b);
            bool payloadCRC   = ((RODerror >> jBits::ROD_ERROR_PAYLOAD_CRC     ) & jBits::ROD_TRAILER_1b);
            bool headerCRC    = ((RODerror >> jBits::ROD_ERROR_HEADER_CRC      ) & jBits::ROD_TRAILER_1b);
            bool reserved     = ((RODerror >> jBits::ROD_ERROR_RESERVED        ) & jBits::ROD_TRAILER_1b);
            bool lenerror     = ((RODerror >> jBits::ROD_ERROR_LENGTH_MISMATCH ) & jBits::ROD_TRAILER_1b);
            bool headmismatch = ((RODerror >> jBits::ROD_ERROR_HEADER_MISMATCH ) & jBits::ROD_TRAILER_1b);
            bool processerror = ((RODerror >> jBits::ROD_ERROR_PROC_TIMEOUT    ) & jBits::ROD_TRAILER_1b);  
            
            std::stringstream sdetail;
            sdetail  << "ROD Trailer Error bits set - 7-bits error word: 0x"<< std::hex <<RODerror << std::dec<< std::endl;
            sdetail  << "Corrective Trailer: "<< corTrailer   << std::endl;
            sdetail  << "Payload CRC       : "<< payloadCRC   << std::endl;
            sdetail  << "Header CRC        : "<< headerCRC    << std::endl;
            sdetail  << "Reserved (=0)     : "<< reserved     << std::endl;
            sdetail  << "Length Mismatch   : "<< lenerror     << std::endl;
            sdetail  << "Header Mismatch   : "<< headmismatch << std::endl;
            sdetail  << "Processor timeout : "<< processerror << std::endl;
            std::stringstream slocation;
            slocation  << "ROD Error";            
            
            //Returning Status code failure, it needs to go to debug stream
            if( corTrailer ){
                std::stringstream stitle;
                stitle  << "Corrective Trailer" ;
                printError(slocation.str(),stitle.str(),MSG::ERROR,stitle.str() + " - "+ sdetail.str());
                return StatusCode::FAILURE;
            }             
            if( payloadCRC ){
                std::stringstream stitle;
                stitle  << "Payload CRC" ;
                printError(slocation.str(),stitle.str(),MSG::ERROR,stitle.str() + " - "+ sdetail.str());
                return StatusCode::FAILURE;
            }             
            if( headerCRC ){
                std::stringstream stitle;
                stitle  << "Header CRC" ;
                printError(slocation.str(),stitle.str(),MSG::ERROR,stitle.str() + " - "+ sdetail.str());
                return StatusCode::FAILURE;
            }             
            if( lenerror ){
                std::stringstream stitle;
                stitle  << "Length mismatch" ;
                printError(slocation.str(),stitle.str(),MSG::ERROR,stitle.str() + " - "+ sdetail.str()); 
                return StatusCode::FAILURE;                   
            }               
            if( headmismatch ){
                std::stringstream stitle;
                stitle  << "Header mismatch" ;
                printError(slocation.str(),stitle.str(),MSG::ERROR,stitle.str() + " - "+ sdetail.str()); 
                return StatusCode::FAILURE;                   
            }   
            if( processerror ){
                std::stringstream stitle;
                stitle  << "Processor Timeout" ;
                printError(slocation.str(),stitle.str(),MSG::ERROR,stitle.str() + " - "+ sdetail.str()); 
                return StatusCode::FAILURE;                   
            }            
            
            return StatusCode::FAILURE;       
        }
        
        while(READ_TOBS){
            
            const auto [payload, fpga, jfex, error]               = jFEXtoRODTrailer  ( vec_words.at(trailers_pos-2), vec_words.at(trailers_pos-1) );
            const auto [n_xjJ, n_xjLJ, n_xjTau, n_xjEM]           = xTOBCounterTrailer( vec_words.at(trailers_pos-3) );    
            const auto [n_jJ, n_jLJ, n_jTau, n_jEM, n_jTE, n_jXE] = TOBCounterTrailer ( vec_words.at(trailers_pos-4) );
            unsigned int n_tobs  = n_jJ  + n_jLJ  + n_jTau  + n_jEM  + n_jTE + n_jXE;
            unsigned int n_xtobs = n_xjJ + n_xjLJ + n_xjTau + n_xjEM;
            unsigned int total_tobs = n_tobs+n_xtobs;
            //printf("----------> trailer_pos:%3d payload:%3d N.Words:%3d fpga:%3d  jfex:%3d\n",trailers_pos,payload,total_tobs+jBits::TOB_TRAILERS,fpga,jfex);
            //printf(" TOBs(0x%08x):   jJ:%3d   jLJ:%3d   jTau:%3d   jEM:%3d   jTE:%3d   jXE:%3d\n",vec_words.at(trailers_pos-4),n_jJ,n_jLJ,n_jTau,n_jEM,n_jTE,n_jXE);  
            //printf("xTOBs(0x%08x):  xjJ:%3d  xjLJ:%3d  xjTau:%3d  xjEM:%3d\n",vec_words.at(trailers_pos-3),n_xjJ,n_xjLJ,n_xjTau,n_xjEM);            

            // First we need to check if there is a padding word.
            //Padding wordis when there is an even number in the payload, but odd number of xTOB and TOB.. need to check and remove the extra word.
            unsigned int paddingWord = 0;
            if(total_tobs % 2){
                ATH_MSG_DEBUG("Odd number of TOBs + xTOBs:"<< total_tobs<<", there is a padding word!");
                paddingWord = 1;
            }

            if(error != 0){
                
                bool corTrailer   = ((error >> jBits::ERROR_CORR_TRAILER    ) & jBits::ROD_TRAILER_1b);
                bool safemode     = ((error >> jBits::ERROR_SAFE_MODE       ) & jBits::ROD_TRAILER_1b);
                bool proterror    = ((error >> jBits::ERROR_PROTOCOL_ERROR  ) & jBits::ROD_TRAILER_1b);
                bool lenerror     = ((error >> jBits::ERROR_LENGTH_MISMATCH ) & jBits::ROD_TRAILER_1b);
                bool headmismatch = ((error >> jBits::ERROR_HEADER_MISMATCH ) & jBits::ROD_TRAILER_1b);
                bool processerror = ((error >> jBits::ERROR_PROC_TIMEOUT    ) & jBits::ROD_TRAILER_1b);
                
                std::stringstream sdetail;
                sdetail  << "jFEX to ROD Trailer Error bits set - 6-bits error word: 0x"<< std::hex <<error << std::dec<< std::endl;
                sdetail  << "Corrective Trailer: "<< corTrailer << std::endl;
                sdetail  << "Safe Mode         : "<< safemode << std::endl;
                sdetail  << "Protocol error    : "<< proterror << std::endl;
                sdetail  << "Length Mismatch   : "<< lenerror << std::endl;
                sdetail  << "Header Mismatch   : "<< headmismatch << std::endl;
                sdetail  << "Processor timeout : "<< processerror << std::endl;
                std::stringstream slocation;
                slocation  << "Error bit set";
                
                if( corTrailer ){
                    std::stringstream stitle;
                    stitle  << "Corrective Trailer" ;
                    printError(slocation.str(),stitle.str(),MSG::ERROR,sdetail.str());
                    
                    //Returning Status code failure needs to go to debug stream
                    return StatusCode::FAILURE;           
                }   
                if( safemode ){
                    std::stringstream stitle;
                    stitle  << "Safe Mode" ;
                    printError(slocation.str(),stitle.str(),MSG::WARNING,sdetail.str());                    
                }   
                if( proterror ){
                    std::stringstream stitle;
                    stitle  << "Protocol error" ;
                    printError(slocation.str(),stitle.str(),MSG::WARNING,sdetail.str());                    
                }   
                if( lenerror ){
                    std::stringstream stitle;
                    stitle  << "Length mismatch" ;
                    printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());                    
                }   
                if( headmismatch ){
                    std::stringstream stitle;
                    stitle  << "Header mismatch" ;
                    printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());                    
                }   
                if( processerror ){
                    std::stringstream stitle;
                    stitle  << "Processor Timeout" ;
                    printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());                    
                }   

            }
            
            if(payload != (total_tobs + jBits::TOB_TRAILERS + paddingWord)){
                std::stringstream sdetail;
                sdetail  << "Payload="<< payload<<" is different from TOBs+Trailers+padding words="<< total_tobs + jBits::TOB_TRAILERS + paddingWord <<" in FPGA: "<< fpga << " and jFEX: "<< jfex <<". SKIPPED!" ;
                std::stringstream slocation;
                slocation  << "jFEX "<< jfex << " FPGA "<< fpga << " in 0x"<< std::hex << rob->rob_source_id();
                std::stringstream stitle;
                stitle  << "Wrong payload" ;
                printError(slocation.str(),stitle.str(),MSG::WARNING,sdetail.str());                        
                
                //Checking if we can continue decoding data for the rest of FPGAs. No negative positions
                int neg_positions = trailers_pos - (payload + jBits::TOB_TRAILERS);
                //If negative, whole event must be discarded!
                if(neg_positions < 0){
                    std::stringstream sdetail;
                    sdetail  << "jFEX TOB decoder has discarded the whole event. Due to a wrong payload cannot continue decoding" ;
                    std::stringstream slocation;
                    slocation  << "jFEX "<< jfex << " FPGA "<< fpga << " in 0x"<< std::hex << rob->rob_source_id();
                    std::stringstream stitle;
                    stitle  << "Event discarded" ;
                    
                    printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());                     
                    break;
                }
                
                //Jumping to next FPGA
                trailers_pos -= (payload+jBits::TOB_TRAILERS);
                
                //Is it 0? then we finish the loop, next event
                if(trailers_pos == 0) {
                    READ_TOBS = false;
                }
                continue;
            }
            
            //The minimum number for the payload should be 2 (The TOB/xTOB counters). If lower send an error message
            if(payload < jBits::TOB_TRAILERS){
                std::stringstream sdetail;
                sdetail  << "Payload: "<< payload<<" is lower than the expected size (at least" << jBits::TOB_TRAILERS << "trailers)" ;
                std::stringstream slocation;
                slocation  << "jFEX "<< jfex << " FPGA "<< fpga << " in 0x"<< std::hex << rob->rob_source_id();
                std::stringstream stitle;
                stitle  << "Event discarded" ;
                
                printError(slocation.str(),stitle.str(),MSG::WARNING,sdetail.str());                  
                break;
            }
                        
            //removing jFEX to ROD, TOB and xTOB Trailers from trailers_pos (4 positions), possible padding word added on the data to get even number of 32bit words
            unsigned int tobIndex = trailers_pos - (jBits::jFEX2ROD_WORDS + jBits::TOB_TRAILERS + paddingWord);
            
            if(paddingWord == 1){
                ATH_MSG_DEBUG("Padding word: "<< std::hex << vec_words.at(tobIndex) <<std::dec);
            }
            
            if(m_convertExtendedTOBs) {
                /************************************************** DECODING xTOBS **************************************************/
                
                //saving xjEM into the EDM container
                for(unsigned int i=tobIndex; i>tobIndex-n_xjEM; i--) {
                    const auto [eta, phi ] = getEtaPhi(jfex, fpga, vec_words.at(i-1),"jEM xTOB");
                    jEMContainer->push_back( std::make_unique<xAOD::jFexFwdElRoI>() );
                    jEMContainer->back()->initialize(jfex, fpga, vec_words.at(i-1),0, m_jEMRes, eta, phi);
                }
                //removing xjEM counter from TOBs
                tobIndex -= n_xjEM;

                //saving xjTau into the EDM container
                for(unsigned int i=tobIndex; i>tobIndex-n_xjTau; i--) {
                    const auto [eta, phi ] = getEtaPhi(jfex, fpga, vec_words.at(i-1),"jTau xTOB");
                    jTauContainer->push_back( std::make_unique<xAOD::jFexTauRoI>() );
                    jTauContainer->back()->initialize(jfex, fpga, vec_words.at(i-1),0, m_jTauRes, eta, phi);
                }
                //removing xjTau counter from TOBs
                tobIndex -= n_xjTau;

                //saving xjLJ into the EDM container
                /* Aug 2025 - commented out so that we no longer fill jLJ collections. May be resurrected for run4
                 * so am leaving this code commented
                for(unsigned int i=tobIndex; i>tobIndex-n_xjLJ; i--) {
                    const auto [eta, phi ] = getEtaPhi(jfex, fpga, vec_words.at(i-1),"jLJ xTOB");
                    jLJContainer->push_back( std::make_unique<xAOD::jFexLRJetRoI>() );
                    jLJContainer->back()->initialize(jfex, fpga, vec_words.at(i-1),0, m_jLJRes, eta, phi);
                } */
                //removing xjLJ counter from TOBs
                tobIndex -= n_xjLJ;

                //saving xjJ into the EDM container
                for(unsigned int i=tobIndex; i>tobIndex-n_xjJ; i--) {
                    const auto [eta, phi ] = getEtaPhi(jfex, fpga, vec_words.at(i-1),"jJ xTOB");
                    jJContainer->push_back( std::make_unique<xAOD::jFexSRJetRoI>() );
                    jJContainer->back()->initialize(jfex, fpga, vec_words.at(i-1),0, m_jJRes, eta, phi);
                }
                //removing xjJ counter from TOBs
                tobIndex -= n_xjJ;
                
                // skipping TOBs to jump to the next set of data
                tobIndex -= n_tobs;

            }
            else {
                /************************************************** DECODING TOBS **************************************************/
                
                // skipping extended-TOBs
                tobIndex -= n_xtobs;
                
                //saving jXE into the EDM container
                //Only FPGA U1 and U4 are sent to L1Topo, it cover the full phi
                if(fpga == jBits::FPGA_U1 || fpga == jBits::FPGA_U4  ){
                    for(unsigned int i=tobIndex; i>tobIndex-n_jXE; i--) {
                        jXEContainer->push_back( std::make_unique<xAOD::jFexMETRoI>() );
                        jXEContainer->back()->initialize(jfex, fpga, vec_words.at(i-1), m_jXERes);
                    }                    
                }
                //removing jXE counter from TOBs
                tobIndex -= n_jXE;

                //saving jTE into the EDM container
                //Only FPGA U1 and U4 are sent to L1Topo, it cover the full phi
                if(fpga == jBits::FPGA_U1 || fpga == jBits::FPGA_U4  ) {
                    for(unsigned int i=tobIndex; i>tobIndex-n_jTE; i--) {
                        jTEContainer->push_back( std::make_unique<xAOD::jFexSumETRoI>() );
                        jTEContainer->back()->initialize(jfex, fpga, vec_words.at(i-1), m_jTERes);
                    }
                }
                //removing jTE counter from TOBs
                tobIndex -= n_jTE;

                //saving jEM into the EDM container
                for(unsigned int i=tobIndex; i>tobIndex-n_jEM; i--) {
                    const auto [eta, phi ] = getEtaPhi(jfex, fpga, vec_words.at(i-1),"jEM TOB");
                    jEMContainer->push_back( std::make_unique<xAOD::jFexFwdElRoI>() );
                    jEMContainer->back()->initialize(jfex, fpga, vec_words.at(i-1),1, m_jEMRes, eta, phi);
                }
                //removing jEM counter from TOBs
                tobIndex -= n_jEM;

                //saving jTau into the EDM container
                for(unsigned int i=tobIndex; i>tobIndex-n_jTau; i--) {
                    const auto [eta, phi ] = getEtaPhi(jfex, fpga, vec_words.at(i-1),"jTau TOB");
                    jTauContainer->push_back( std::make_unique<xAOD::jFexTauRoI>() );
                    jTauContainer->back()->initialize(jfex, fpga, vec_words.at(i-1),1, m_jTauRes, eta, phi);
                }
                //removing jTau counter from TOBs
                tobIndex -= n_jTau;

                //saving jLJ into the EDM container
                for(unsigned int i=tobIndex; i>tobIndex-n_jLJ; i--) {
                    const auto [eta, phi ] = getEtaPhi(jfex, fpga, vec_words.at(i-1),"jLJ TOB");
                    jLJContainer->push_back( std::make_unique<xAOD::jFexLRJetRoI>() );
                    jLJContainer->back()->initialize(jfex, fpga, vec_words.at(i-1),1, m_jLJRes, eta, phi);
                }
                //removing jLJ counter from TOBs
                tobIndex -= n_jLJ;

                //saving jJ into the EDM container
                for(unsigned int i=tobIndex; i>tobIndex-n_jJ; i--) {
                    const auto [eta, phi ] = getEtaPhi(jfex, fpga, vec_words.at(i-1),"jJ TOB");
                    jJContainer->push_back( std::make_unique<xAOD::jFexSRJetRoI>() );
                    jJContainer->back()->initialize(jfex, fpga, vec_words.at(i-1),1, m_jJRes, eta, phi);
                }
                //removing jJ counter from TOBs
                tobIndex -= n_jJ;
            }
            //moving trailer position index to the next jFEX data block
            trailers_pos -= (payload + jBits::jFEX2ROD_WORDS);
            
            if(trailers_pos != tobIndex){
                std::stringstream sdetail;
                sdetail  << "Something went wrong decoding jFEX BS data. Trailer position: " << trailers_pos << " should match the TOB index position:" << tobIndex ;
                std::stringstream slocation;
                slocation  << "jFEX "<< jfex << " FPGA "<< fpga << " in 0x"<< std::hex << rob->rob_source_id();
                std::stringstream stitle;
                stitle  << "Wrong amount of words" ;
                
                printError(slocation.str(),stitle.str(),MSG::ERROR,sdetail.str());                    
                
                return StatusCode::FAILURE;
            }
            

            if(trailers_pos == 0){
                READ_TOBS = false;
            }
        }
    }
        
    return StatusCode::SUCCESS;
}


// Unpack number of TOBs in Trailer "TOB,XTOB Counter Trailer"
std::array<uint32_t,6> jFexRoiByteStreamTool::TOBCounterTrailer (uint32_t word) const {
    
    uint32_t jJ    = ((word >> jBits::jJ_TOB_COUNTS  ) & jBits::TOB_COUNTS_6b);
    uint32_t jLJ   = ((word >> jBits::jLJ_TOB_COUNTS ) & jBits::TOB_COUNTS_6b);
    uint32_t jTau  = ((word >> jBits::jTau_TOB_COUNTS) & jBits::TOB_COUNTS_6b);
    uint32_t jEM   = ((word >> jBits::jEM_TOB_COUNTS ) & jBits::TOB_COUNTS_6b);
    uint32_t jTE   = ((word >> jBits::jTE_TOB_COUNTS ) & jBits::TOB_COUNTS_1b);
    uint32_t jXE   = ((word >> jBits::jXE_TOB_COUNTS ) & jBits::TOB_COUNTS_1b);
    //uint32_t TSM   = ((word                          ) & jBits::TOB_COUNTS_1b); // Trigger Safe Mode. Not used for now, maybe is needed in the future

    std::stringstream sdetail;
    sdetail  << "TOB Counter Trailer. Word: 0x"<< std::hex <<word << std::dec << std::endl;
    sdetail  << "jJ Item   : "<< jJ << std::endl;
    sdetail  << "jLJ Item  : "<< jLJ << std::endl;
    sdetail  << "jTau Item : "<< jTau << std::endl;
    sdetail  << "jEM Item  : "<< jEM << std::endl;
    sdetail  << "jTE Item  : "<< jTE << std::endl;
    sdetail  << "jXE Item  : "<< jXE << std::endl;
    ATH_MSG_DEBUG(sdetail.str());
          
    return {jJ,jLJ,jTau,jEM,jTE,jXE};
        
}

// Unpack number of xTOBs in Trailer "TOB,XTOB Counter Trailer"
std::array<uint32_t,4> jFexRoiByteStreamTool::xTOBCounterTrailer (uint32_t word) const {
    
    uint32_t xjJ    = ((word >> jBits::jJ_TOB_COUNTS  ) & jBits::TOB_COUNTS_6b);
    uint32_t xjLJ   = ((word >> jBits::jLJ_TOB_COUNTS ) & jBits::TOB_COUNTS_6b);
    uint32_t xjTau  = ((word >> jBits::jTau_TOB_COUNTS) & jBits::TOB_COUNTS_6b);
    uint32_t xjEM   = ((word >> jBits::jEM_TOB_COUNTS ) & jBits::TOB_COUNTS_6b);

    std::stringstream sdetail;
    sdetail  << "xTOB Counter Trailer. Word: 0x"<< std::hex <<word << std::dec << std::endl;
    sdetail  << "xjJ Item   : "<< xjJ << std::endl;
    sdetail  << "xjLJ Item  : "<< xjLJ << std::endl;
    sdetail  << "xjTau Item : "<< xjTau << std::endl;
    sdetail  << "xjEM Item  : "<< xjEM << std::endl;
    ATH_MSG_DEBUG(sdetail.str());
    
    return {xjJ,xjLJ,xjTau,xjEM};
    
}

// Unpack jFEX to ROD Trailer
std::array<uint32_t,4> jFexRoiByteStreamTool::jFEXtoRODTrailer (uint32_t word0, uint32_t word1) const {
    
    uint32_t payload    = ((word0 >> jBits::PAYLOAD_ROD_TRAILER ) & jBits::ROD_TRAILER_16b);
    uint32_t fpga       = ((word0 >> jBits::FPGA_ROD_TRAILER    ) & jBits::ROD_TRAILER_2b );
    uint32_t jfex       = ((word0 >> jBits::jFEX_ROD_TRAILER    ) & jBits::ROD_TRAILER_4b );
    uint32_t ROslice    = ((word0 >> jBits::RO_ROD_TRAILER      ) & jBits::ROD_TRAILER_4b );
    uint32_t TSN        = ((word0 >> jBits::TSN_ROD_TRAILER     ) & jBits::ROD_TRAILER_4b );
    
    uint32_t error      = ((word1 >> jBits::ERROR_ROD_TRAILER   ) & jBits::ROD_TRAILER_6b );
    uint32_t CRC        = ((word1 >> jBits::CRC_ROD_TRAILER     ) & jBits::ROD_TRAILER_20b);
    
    
    std::stringstream sdetail;
    sdetail  << "jFEX to ROD Trailer. Word0: 0x"<< std::hex <<word0 << " and Word1: 0x"<< word1  << std::endl;
    sdetail  << "Payload  (from word0): "<< payload << std::endl;
    sdetail  << "FPGA     (from word0): "<< fpga    << std::endl;
    sdetail  << "jFEX     (from word0): "<< jfex    << std::endl;
    sdetail  << "RO slice (from word0): "<< ROslice << std::endl;
    sdetail  << "TSN      (from word0): "<< TSN     << std::endl;
    sdetail  << "Error    (from word1): "<< error   << std::endl;
    sdetail  << "CRC      (from word1): "<< CRC     << std::dec << std::endl;
    
    ATH_MSG_DEBUG(sdetail.str());
    
    return {payload, fpga, jfex, error};
   
}

// Unpack ROD Trailer
std::array<uint32_t,1> jFexRoiByteStreamTool::RODTrailer (uint32_t /*word0*/, uint32_t word1) const {

    uint32_t error      = ((word1 >> jBits::ERROR_ROD_TRAILER   ) & jBits::ROD_TRAILER_7b );

    //return an array since we can implement more features in the future
    return {error};

}


// Pack TOB counts into "TOB Counter Trailer" (inverse of TOBCounterTrailer)
uint32_t jFexRoiByteStreamTool::buildTOBCounterTrailer(uint32_t n_jJ, uint32_t n_jLJ, uint32_t n_jTau,
                                                       uint32_t n_jEM, uint32_t has_jTE, uint32_t has_jXE) const {
    uint32_t word = 0;
    word |= (n_jJ    & jBits::TOB_COUNTS_6b) << jBits::jJ_TOB_COUNTS;
    word |= (n_jLJ   & jBits::TOB_COUNTS_6b) << jBits::jLJ_TOB_COUNTS;
    word |= (n_jTau  & jBits::TOB_COUNTS_6b) << jBits::jTau_TOB_COUNTS;
    word |= (n_jEM   & jBits::TOB_COUNTS_6b) << jBits::jEM_TOB_COUNTS;
    word |= (has_jTE & jBits::TOB_COUNTS_1b) << jBits::jTE_TOB_COUNTS;
    word |= (has_jXE & jBits::TOB_COUNTS_1b) << jBits::jXE_TOB_COUNTS;
    return word;
}


// Pack xTOB counts into "xTOB Counter Trailer" (inverse of xTOBCounterTrailer)
uint32_t jFexRoiByteStreamTool::buildxTOBCounterTrailer(uint32_t n_xjJ, uint32_t n_xjLJ,
                                                        uint32_t n_xjTau, uint32_t n_xjEM) const {
    uint32_t word = 0;
    word |= (n_xjJ   & jBits::TOB_COUNTS_6b) << jBits::jJ_TOB_COUNTS;
    word |= (n_xjLJ  & jBits::TOB_COUNTS_6b) << jBits::jLJ_TOB_COUNTS;
    word |= (n_xjTau & jBits::TOB_COUNTS_6b) << jBits::jTau_TOB_COUNTS;
    word |= (n_xjEM  & jBits::TOB_COUNTS_6b) << jBits::jEM_TOB_COUNTS;
    return word;
}


// Pack jFEX-to-ROD trailer (inverse of jFEXtoRODTrailer).
// ROslice / TSN (in word0) and error / CRC (in word1) are left at 0; round-trip
// decoder does not use them for histogram comparison.
std::array<uint32_t,2> jFexRoiByteStreamTool::buildjFEXtoRODTrailer(uint32_t payload, uint32_t fpga,
                                                                   uint32_t jfex) const {
    uint32_t word0 = 0;
    word0 |= (payload & jBits::ROD_TRAILER_16b) << jBits::PAYLOAD_ROD_TRAILER;
    word0 |= (fpga    & jBits::ROD_TRAILER_2b ) << jBits::FPGA_ROD_TRAILER;
    word0 |= (jfex    & jBits::ROD_TRAILER_4b ) << jBits::jFEX_ROD_TRAILER;
    uint32_t word1 = 0;
    return {word0, word1};
}


// Unpack jFEX to ROD Header
void jFexRoiByteStreamTool::jFEXtoRODHeader (uint32_t /*word0*/, uint32_t /*word1*/) const {
    
    //std::cout << "jFEXtoRODHeader: word0 0x" <<  std::hex << word0  << std::dec << std::endl;
    //std::cout << "jFEXtoRODHeader: word1 0x" <<  std::hex << word1  << std::dec << std::endl;
    
    //std::cout << "\tWord0  : " <<  std::bitset<32>(word0) << std::endl;
    //std::cout << "\tWord1  : " <<  std::bitset<32>(word1) << std::endl;
    

    //std::cout << "\tWord0 L1Calo: " <<  ((word0 >> jBits::L1CALO_STREAM_ID_ROD_HEADER ) & jBits::ROD_HEADER_8b  ) << std::endl;
    //std::cout << "\t\t L1Calo SLOT: " <<  ((word0 >> (jBits::L1CALO_STREAM_ID_ROD_HEADER + jBits::L1CALO_STREAM_ID_SLOT_ROD_HEADER) ) & 4  ) << std::endl;
    //std::cout << "\t\t L1Calo FPGA: " <<  ((word0 >> (jBits::L1CALO_STREAM_ID_ROD_HEADER + jBits::L1CALO_STREAM_ID_FPGA_ROD_HEADER) ) & 2  ) << std::endl;
    //std::cout << "\t\t L1Calo INFO: " <<  ((word0 >> (jBits::L1CALO_STREAM_ID_ROD_HEADER + jBits::L1CALO_STREAM_ID_INFO_ROD_HEADER) ) & 2  ) << std::endl;
    //std::cout << "\tWord0 BCN   : " <<  ((word0 >> jBits::BCN_ROD_HEADER              ) & jBits::ROD_HEADER_12b ) << std::endl;   
    //std::cout << "\tWord0 CRC   : " <<  ((word0 >> jBits::CRC_ROD_HEADER              ) & jBits::ROD_HEADER_9b  ) << std::endl;
    //std::cout << "\tWord0 Vers  : " <<  ((word0 >> jBits::VERS_ROD_HEADER             ) & jBits::ROD_HEADER_3b  ) << std::endl;          

    //std::cout << "\tWord1 L1ID  : " <<  ((word1 >> jBits::L1ID_ROD_HEADER  ) & jBits::ROD_HEADER_24b ) << std::endl;
    //std::cout << "\tWord1 ECRID : " <<  ((word1 >> jBits::ECRID_ROD_HEADER ) & jBits::ROD_HEADER_8b  ) << std::endl;   


   
}


StatusCode jFexRoiByteStreamTool::ReadfromFile(const std::string & fileName){
    
    std::string myline;
    
    //openning file with ifstream
    std::ifstream myfile(fileName);
    
    if ( !myfile.is_open() ){
        ATH_MSG_ERROR("Could not open file:" << fileName);
        return StatusCode::FAILURE;
    }
    
    //loading the mapping information
    while ( std::getline (myfile, myline) ) {

        //removing the header of the file (it is just information!)
        if(myline[0] == '#') continue;
        
        //Splitting myline in different substrings
        std::stringstream oneLine(myline);
        
        //reading elements
        std::vector<float> elements;
        std::string element;
        while(std::getline(oneLine, element, ' '))
        {
            elements.push_back(std::stof(element));
        }
        
        // It should have 6 elements
        // ordered as:  jfex fpga localEta localPhi eta phi
        if(elements.size() != 6){
            ATH_MSG_ERROR("Unexpected number of elemennts (6 expected) in file: "<< fileName);
            myfile.close();
            return StatusCode::FAILURE;
        }
        // building array of  < eta, phi >
        std::array<float,2> aux_arr{ {elements.at(4),elements.at(5)} };
        
        //filling the map with the hash given by mapIndex()
        m_TobPos_map[ mapIndex(elements.at(0),elements.at(1),elements.at(2),elements.at(3)) ] = aux_arr;
        
    }
    myfile.close();

    return StatusCode::SUCCESS;
}

constexpr unsigned int jFexRoiByteStreamTool::mapIndex(unsigned int jfex, unsigned int fpga, unsigned int iEta, unsigned int iPhi) {
  // values from hardware: jfex=[0,5] 4 bits, fpga=[0,3] 4 bits, ieta=[0,24] 8 bits, iphi=[0,15] 4 bits
  return (jfex << 16) | (fpga << 12) | (iEta << 4) | iPhi;
}

std::array<uint8_t,2> jFexRoiByteStreamTool::unpackLocalCoords  (uint32_t word) const{
    
    uint8_t iEta = (word >> s_etaBit) & s_etaMask;
    uint8_t iPhi = (word >> s_phiBit) & s_phiMask;
    
    return {iEta, iPhi};
    
}

std::array<float,2> jFexRoiByteStreamTool::getEtaPhi  (unsigned int jfex, unsigned int fpga, uint32_t word, const char* obj = "") const{
    
    const auto [iEta, iPhi] = unpackLocalCoords(word);
    unsigned int index = mapIndex(jfex, fpga, iEta, iPhi);
    
    auto it = m_TobPos_map.find(index);
    if(it == m_TobPos_map.end()){
        //This should be a WARNING.. masked until jFEX firmware bug is fixed
        ATH_MSG_DEBUG("No index found for "<< obj <<" word, jFex:"<<jfex << " Fpga:"<<fpga<< " iEta:"<<+iEta<< " iPhi:"<<+iPhi<< " for 32-bit word: 0x"<<std::hex<<word<<std::dec);
        return {-99, -99};
    }
    
    //returns an std::array<float,2> {eta, phi}
    return it->second;
    
}

/// xAOD->BS conversion
StatusCode jFexRoiByteStreamTool::convertToBS(std::vector<WROBF*>& vrobf,
                                              const xAOD::TrigCompositeContainer* tc,
                                              const EventContext& eventContext) {

    FragmentMap blocks;
    if (tc && !tc->empty()) {
        const xAOD::TrigComposite& l1 = *tc->at(0);
        bucketContainer<xAOD::jFexSRJetRoI >(l1, blocks);
        bucketContainer<xAOD::jFexLRJetRoI >(l1, blocks);
        bucketContainer<xAOD::jFexTauRoI   >(l1, blocks);
        bucketContainer<xAOD::jFexFwdElRoI >(l1, blocks);
        bucketContainer<xAOD::jFexMETRoI   >(l1, blocks);
        bucketContainer<xAOD::jFexSumETRoI >(l1, blocks);
    }

    std::vector<uint32_t> words;
    for (const auto& [key, blk] : blocks) {
        const uint32_t jfex = key.first;
        const uint32_t fpga = key.second;

        const uint32_t n_jJ    = blk.jJ.size();
        const uint32_t n_jLJ   = blk.jLJ.size();
        const uint32_t n_jTau  = blk.jTau.size();
        const uint32_t n_jEM   = blk.jEM.size();
        const uint32_t n_jTE   = blk.jTE.size();
        const uint32_t n_jXE   = blk.jXE.size();
        const uint32_t n_xjJ   = blk.xjJ.size();
        const uint32_t n_xjLJ  = blk.xjLJ.size();
        const uint32_t n_xjTau = blk.xjTau.size();
        const uint32_t n_xjEM  = blk.xjEM.size();

        const uint32_t total_tobs = n_jJ + n_jLJ + n_jTau + n_jEM + n_jTE + n_jXE
                                  + n_xjJ + n_xjLJ + n_xjTau + n_xjEM;
        const uint32_t paddingWord = (total_tobs % 2 == 0) ? 0 : 1;
        const uint32_t payload     = total_tobs + jBits::TOB_TRAILERS + paddingWord;

        // Layout (low → high addr), reversing the decoder walk:
        //   jJ, jLJ, jTau, jEM, jTE, jXE, xjJ, xjLJ, xjTau, xjEM, [pad],
        //   TOB-counter, xTOB-counter, jFEX-to-ROD w0, jFEX-to-ROD w1
        // Decoder (TOB mode) does `tobIndex -= n_xtobs` first, then reads jXE → jJ
        // going downwards, so xTOBs sit at high addresses, just below the trailers.
        for (uint32_t w : blk.jJ)    words.push_back(w);
        for (uint32_t w : blk.jLJ)   words.push_back(w);
        for (uint32_t w : blk.jTau)  words.push_back(w);
        for (uint32_t w : blk.jEM)   words.push_back(w);
        for (uint32_t w : blk.jTE)   words.push_back(w);
        for (uint32_t w : blk.jXE)   words.push_back(w);
        for (uint32_t w : blk.xjJ)   words.push_back(w);
        for (uint32_t w : blk.xjLJ)  words.push_back(w);
        for (uint32_t w : blk.xjTau) words.push_back(w);
        for (uint32_t w : blk.xjEM)  words.push_back(w);
        if (paddingWord) words.push_back(0);

        words.push_back(buildTOBCounterTrailer (n_jJ, n_jLJ, n_jTau, n_jEM,
                                                n_jTE > 0, n_jXE > 0));
        words.push_back(buildxTOBCounterTrailer(n_xjJ, n_xjLJ, n_xjTau, n_xjEM));
        const auto rod = buildjFEXtoRODTrailer(payload, fpga, jfex);
        words.push_back(rod[0]);
        words.push_back(rod[1]);
    }

    // ROD trailer (decoder reads only 7-bit error in word1 — write 0)
    words.push_back(0);
    words.push_back(0);

    clearCache(eventContext);
    uint32_t* data = newRodData(eventContext, words.size());
    std::copy(words.begin(), words.end(), data);

    // jFex packs all 24 FPGA blocks (6 modules × 4 FPGAs) into a single ROB.
    eformat::helper::SourceIdentifier sid(eformat::TDAQ_CALO_FEAT_EXTRACT_ROI, 0x2000);
    vrobf.push_back(newRobFragment(eventContext, sid.code(), words.size(), data, 0));

    return StatusCode::SUCCESS;
}


void  jFexRoiByteStreamTool::printError(const std::string& location, const std::string& title, MSG::Level type, const std::string& detail) const{
    
    if(m_UseMonitoring){
        Monitored::Group(m_monTool,
                     Monitored::Scalar("jfexDecoderErrorLocation",location.empty() ? std::string("UNKNOWN") : location),
                     Monitored::Scalar("jfexDecoderErrorTitle"   ,title.empty()    ? std::string("UNKNOWN") : title)
                     );
    }
    else {
        msg() << type << detail << endmsg;
    }
}
