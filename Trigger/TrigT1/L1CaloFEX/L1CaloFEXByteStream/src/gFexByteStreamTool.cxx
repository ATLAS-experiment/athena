/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           gFexByteStreamTool  -  description
//                              -------------------
//     begin                : 20 07 2022
//     email                : cecilia.tosciri@cern.ch
//  ***************************************************************************/

#include "gFexByteStreamTool.h"
#include "L1CaloFEXByteStream/gFexPos.h"
#include "CxxUtils/span.h"
#include "eformat/SourceIdentifier.h"
#include "eformat/Status.h"

using ROBF = OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;
using WROBF = OFFLINE_FRAGMENTS_NAMESPACE_WRITE::ROBFragment;

namespace gPos = LVL1::gFEXPos;

gFexByteStreamTool::gFexByteStreamTool(const std::string& type,
        const std::string& name,
        const IInterface* parent)
    : base_class(type, name, parent) {}

StatusCode gFexByteStreamTool::initialize() {
    // Conversion mode for gRho TOBs
    ATH_MSG_DEBUG(" ROB IDs: " << MSG::hex << m_robIds.value() << MSG::dec);

    ConversionMode gRhomode = getConversionMode(m_gFexRhoReadKey, m_gFexRhoWriteKey, msg());
    ATH_CHECK(gRhomode!=ConversionMode::Undefined);
    ATH_CHECK(m_gFexRhoWriteKey.initialize(gRhomode==ConversionMode::Decoding));
    ATH_CHECK(m_gFexRhoReadKey.initialize(gRhomode==ConversionMode::Encoding));
    ATH_MSG_DEBUG((gRhomode==ConversionMode::Encoding ? "Encoding" : "Decoding") << " gRho ");

    // Conversion mode for gSJ TOBs
    ConversionMode gSJmode = getConversionMode(m_gFexBlockReadKey, m_gFexBlockWriteKey, msg());
    ATH_CHECK(gSJmode!=ConversionMode::Undefined);
    ATH_CHECK(m_gFexBlockWriteKey.initialize(gSJmode==ConversionMode::Decoding));
    ATH_CHECK(m_gFexBlockReadKey.initialize(gSJmode==ConversionMode::Encoding));
    ATH_MSG_DEBUG((gSJmode==ConversionMode::Encoding ? "Encoding" : "Decoding") << " gSJ ");

    // Conversion mode for gLJ TOBs
    ConversionMode gLJmode = getConversionMode(m_gFexJetReadKey, m_gFexJetWriteKey, msg());
    ATH_CHECK(gLJmode!=ConversionMode::Undefined);
    ATH_CHECK(m_gFexJetWriteKey.initialize(gLJmode==ConversionMode::Decoding));
    ATH_CHECK(m_gFexJetReadKey.initialize(gLJmode==ConversionMode::Encoding));
    ATH_MSG_DEBUG((gLJmode==ConversionMode::Encoding ? "Encoding" : "Decoding") << " gLJ ");

    // Conversion mode for gScalarEJwoj TOBs
    ConversionMode gScalarEJwojmode = getConversionMode(m_gScalarEJwojReadKey, m_gScalarEJwojWriteKey, msg());
    ATH_CHECK(gScalarEJwojmode!=ConversionMode::Undefined);
    ATH_CHECK(m_gScalarEJwojWriteKey.initialize(gScalarEJwojmode==ConversionMode::Decoding));
    ATH_CHECK(m_gScalarEJwojReadKey.initialize(gScalarEJwojmode==ConversionMode::Encoding));
    ATH_MSG_DEBUG((gScalarEJwojmode==ConversionMode::Encoding ? "Encoding" : "Decoding") << " gScalarEJwoj ");

    // Conversion mode for gMETComponentsJwoj TOBs
    ConversionMode gMETComponentsJwojmode = getConversionMode(m_gMETComponentsJwojReadKey, m_gMETComponentsJwojWriteKey, msg());
    ATH_CHECK(gMETComponentsJwojmode!=ConversionMode::Undefined);
    ATH_CHECK(m_gMETComponentsJwojWriteKey.initialize(gMETComponentsJwojmode==ConversionMode::Decoding));
    ATH_CHECK(m_gMETComponentsJwojReadKey.initialize(gMETComponentsJwojmode==ConversionMode::Encoding));
    ATH_MSG_DEBUG((gMETComponentsJwojmode==ConversionMode::Encoding ? "Encoding" : "Decoding") << " gMETComponentsJwoj ");

    // Conversion mode for gMHTComponentsJwoj TOBs
    ConversionMode gMHTComponentsJwojmode = getConversionMode(m_gMHTComponentsJwojReadKey, m_gMHTComponentsJwojWriteKey, msg());
    ATH_CHECK(gMHTComponentsJwojmode!=ConversionMode::Undefined);
    ATH_CHECK(m_gMHTComponentsJwojWriteKey.initialize(gMHTComponentsJwojmode==ConversionMode::Decoding));
    ATH_CHECK(m_gMHTComponentsJwojReadKey.initialize(gMHTComponentsJwojmode==ConversionMode::Encoding));
    ATH_MSG_DEBUG((gMHTComponentsJwojmode==ConversionMode::Encoding ? "Encoding" : "Decoding") << " gMHTComponentsJwoj ");

   // Conversion mode for gMSTComponentsJwoj TOBs
    ConversionMode gMSTComponentsJwojmode = getConversionMode(m_gMSTComponentsJwojReadKey, m_gMSTComponentsJwojWriteKey, msg());
    ATH_CHECK(gMSTComponentsJwojmode!=ConversionMode::Undefined);
    ATH_CHECK(m_gMSTComponentsJwojWriteKey.initialize(gMSTComponentsJwojmode==ConversionMode::Decoding));
    ATH_CHECK(m_gMSTComponentsJwojReadKey.initialize(gMSTComponentsJwojmode==ConversionMode::Encoding));
    ATH_MSG_DEBUG((gMSTComponentsJwojmode==ConversionMode::Encoding ? "Encoding" : "Decoding") << " gMSTComponentsJwoj ");

    // Conversion mode for gMETComponentsNoiseCut TOBs
    ConversionMode gMETComponentsNoiseCutmode = getConversionMode(m_gMETComponentsNoiseCutReadKey, m_gMETComponentsNoiseCutWriteKey, msg());
    ATH_CHECK(gMETComponentsNoiseCutmode!=ConversionMode::Undefined);
    ATH_CHECK(m_gMETComponentsNoiseCutWriteKey.initialize(gMETComponentsNoiseCutmode==ConversionMode::Decoding));
    ATH_CHECK(m_gMETComponentsNoiseCutReadKey.initialize(gMETComponentsNoiseCutmode==ConversionMode::Encoding));
    ATH_MSG_DEBUG((gMETComponentsNoiseCutmode==ConversionMode::Encoding ? "Encoding" : "Decoding") << " gMETComponentsNoiseCut ");

    // Conversion mode for gMETRms TOBs
    ConversionMode gMETComponentsRmsmode = getConversionMode(m_gMETComponentsRmsReadKey, m_gMETComponentsRmsWriteKey, msg());
    ATH_CHECK(gMETComponentsRmsmode!=ConversionMode::Undefined);
    ATH_CHECK(m_gMETComponentsRmsWriteKey.initialize(gMETComponentsRmsmode==ConversionMode::Decoding));
    ATH_CHECK(m_gMETComponentsRmsReadKey.initialize(gMETComponentsRmsmode==ConversionMode::Encoding));
    ATH_MSG_DEBUG((gMETComponentsRmsmode==ConversionMode::Encoding ? "Encoding" : "Decoding") << " gMETComponentsRms ");
 
    // Conversion mode for gScalarENoiseCut TOBs
    ConversionMode gScalarENoiseCutmode = getConversionMode(m_gScalarENoiseCutReadKey, m_gScalarENoiseCutWriteKey, msg());
    ATH_CHECK(gScalarENoiseCutmode!=ConversionMode::Undefined);
    ATH_CHECK(m_gScalarENoiseCutWriteKey.initialize(gScalarENoiseCutmode==ConversionMode::Decoding));
    ATH_CHECK(m_gScalarENoiseCutReadKey.initialize(gScalarENoiseCutmode==ConversionMode::Encoding));
    ATH_MSG_DEBUG((gScalarENoiseCutmode==ConversionMode::Encoding ? "Encoding" : "Decoding") << " gScalarENoiseCut ");

    // Conversion mode for gScalarERms TOBs
    ConversionMode gScalarERmsmode = getConversionMode(m_gScalarERmsReadKey, m_gScalarERmsWriteKey, msg());
    ATH_CHECK(gScalarERmsmode!=ConversionMode::Undefined);
    ATH_CHECK(m_gScalarERmsWriteKey.initialize(gScalarERmsmode==ConversionMode::Decoding));
    ATH_CHECK(m_gScalarERmsReadKey.initialize(gScalarERmsmode==ConversionMode::Encoding));
    ATH_MSG_DEBUG((gScalarERmsmode==ConversionMode::Encoding ? "Encoding" : "Decoding") << " gScalarERms ");

    //checking all Conversion modes.. avoid misconfigurations
    const std::array<ConversionMode,2> modes{gSJmode,gLJmode};
    if (std::any_of(modes.begin(),modes.end(),[&gRhomode](ConversionMode m) { return m!=gRhomode;  } )) {
        ATH_MSG_ERROR("Inconsistent conversion modes");
        return StatusCode::FAILURE;
    }
    
    ATH_CHECK(m_l1MenuKey.initialize());
    
    if (!m_monTool.empty()) {
        ATH_CHECK(m_monTool.retrieve());
        ATH_MSG_INFO("Logging errors to " << m_monTool.name() << " monitoring tool");
        m_UseMonitoring = true;
    }
    
    
    return StatusCode::SUCCESS;
}

StatusCode gFexByteStreamTool::start() {
    // Retrieve the L1 menu configuration
    SG::ReadHandle<TrigConf::L1Menu> l1Menu (m_l1MenuKey);
    ATH_CHECK(l1Menu.isValid());

    try {
        const auto & l1Menu_gJ = l1Menu->thrExtraInfo().gJ();
        const auto & l1Menu_gLJ = l1Menu->thrExtraInfo().gLJ();
        const auto & l1Menu_gXE = l1Menu->thrExtraInfo().gXE();
        const auto & l1Menu_gTE = l1Menu->thrExtraInfo().gTE();

        ATH_CHECK(l1Menu_gJ.isValid());
        ATH_CHECK(l1Menu_gLJ.isValid());
        ATH_CHECK(l1Menu_gXE.isValid());
        ATH_CHECK(l1Menu_gTE.isValid());

        m_gJ_scale = l1Menu_gJ.resolutionMeV(); 
        m_gLJ_scale = l1Menu_gLJ.resolutionMeV(); 
        m_gXE_scale = l1Menu_gXE.resolutionMeV(); 
        m_gTE_scale = l1Menu_gTE.resolutionMeV();
    } catch (const std::exception& e) {
        ATH_MSG_ERROR("Exception reading L1Menu: " << e.what());
        return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;

}

// BS->xAOD conversion
StatusCode gFexByteStreamTool::convertFromBS(const std::vector<const ROBF*>& vrobf, const EventContext& ctx) const {
        
    //WriteHandle for gFEX EDMs
    
    //---Rho Container
    SG::WriteHandle<xAOD::gFexJetRoIContainer> gRhoContainer(m_gFexRhoWriteKey, ctx);
    ATH_CHECK(gRhoContainer.record(std::make_unique<xAOD::gFexJetRoIContainer>(), std::make_unique<xAOD::gFexJetRoIAuxContainer>()));
    ATH_MSG_DEBUG("Recorded gFexJetRoIContainer with key " << gRhoContainer.key());
    
    //---Small Jets Container
    SG::WriteHandle<xAOD::gFexJetRoIContainer> gSJContainer(m_gFexBlockWriteKey, ctx);
    ATH_CHECK(gSJContainer.record(std::make_unique<xAOD::gFexJetRoIContainer>(), std::make_unique<xAOD::gFexJetRoIAuxContainer>()));
    ATH_MSG_DEBUG("Recorded gFexJetRoIContainer with key " << gSJContainer.key());

    //---Large Jets Container
    SG::WriteHandle<xAOD::gFexJetRoIContainer> gLJContainer(m_gFexJetWriteKey, ctx);
    ATH_CHECK(gLJContainer.record(std::make_unique<xAOD::gFexJetRoIContainer>(), std::make_unique<xAOD::gFexJetRoIAuxContainer>()));
    ATH_MSG_DEBUG("Recorded gFexJetRoIContainer with key " << gLJContainer.key());

    //---Scalar MET and SumET JwoJ Container
    SG::WriteHandle<xAOD::gFexGlobalRoIContainer> gScalarEJwojContainer(m_gScalarEJwojWriteKey, ctx);
    ATH_CHECK(gScalarEJwojContainer.record(std::make_unique<xAOD::gFexGlobalRoIContainer>(), std::make_unique<xAOD::gFexGlobalRoIAuxContainer>()));
    ATH_MSG_DEBUG("Recorded gFexJetGlobalContainer with key " << gScalarEJwojContainer.key());

    //---MET Components JwoJ Container
    SG::WriteHandle<xAOD::gFexGlobalRoIContainer> gMETComponentsJwojContainer(m_gMETComponentsJwojWriteKey, ctx);
    ATH_CHECK(gMETComponentsJwojContainer.record(std::make_unique<xAOD::gFexGlobalRoIContainer>(), std::make_unique<xAOD::gFexGlobalRoIAuxContainer>()));
    ATH_MSG_DEBUG("Recorded gFexJetGlobalContainer with key " << gMETComponentsJwojContainer.key());

    //---MHT Components JwoJ Container
    SG::WriteHandle<xAOD::gFexGlobalRoIContainer> gMHTComponentsJwojContainer(m_gMHTComponentsJwojWriteKey, ctx);
    ATH_CHECK(gMHTComponentsJwojContainer.record(std::make_unique<xAOD::gFexGlobalRoIContainer>(), std::make_unique<xAOD::gFexGlobalRoIAuxContainer>()));
    ATH_MSG_DEBUG("Recorded gFexJetGlobalContainer with key " << gMHTComponentsJwojContainer.key());

    //---MST Components JwoJ Container
    SG::WriteHandle<xAOD::gFexGlobalRoIContainer> gMSTComponentsJwojContainer(m_gMSTComponentsJwojWriteKey, ctx);
    ATH_CHECK(gMSTComponentsJwojContainer.record(std::make_unique<xAOD::gFexGlobalRoIContainer>(), std::make_unique<xAOD::gFexGlobalRoIAuxContainer>()));
    ATH_MSG_DEBUG("Recorded gFexJetGlobalContainer with key " << gMSTComponentsJwojContainer.key());

    //---MET Components NoiseCut Container
    SG::WriteHandle<xAOD::gFexGlobalRoIContainer> gMETComponentsNoiseCutContainer(m_gMETComponentsNoiseCutWriteKey, ctx);
    ATH_CHECK(gMETComponentsNoiseCutContainer.record(std::make_unique<xAOD::gFexGlobalRoIContainer>(), std::make_unique<xAOD::gFexGlobalRoIAuxContainer>()));
    ATH_MSG_DEBUG("Recorded gFexJetGlobalContainer with key " << gMETComponentsNoiseCutContainer.key());

    //---MET Components Rms Container
    SG::WriteHandle<xAOD::gFexGlobalRoIContainer> gMETComponentsRmsContainer(m_gMETComponentsRmsWriteKey, ctx);
    ATH_CHECK(gMETComponentsRmsContainer.record(std::make_unique<xAOD::gFexGlobalRoIContainer>(), std::make_unique<xAOD::gFexGlobalRoIAuxContainer>()));
    ATH_MSG_DEBUG("Recorded gFexJetGlobalContainer with key " << gMETComponentsRmsContainer.key());

    //---Scalar MET and SumET NoiseCut Container
    SG::WriteHandle<xAOD::gFexGlobalRoIContainer> gScalarENoiseCutContainer(m_gScalarENoiseCutWriteKey, ctx);
    ATH_CHECK(gScalarENoiseCutContainer.record(std::make_unique<xAOD::gFexGlobalRoIContainer>(), std::make_unique<xAOD::gFexGlobalRoIAuxContainer>()));
    ATH_MSG_DEBUG("Recorded gFexJetGlobalContainer with key " << gScalarENoiseCutContainer.key());

    //---Scalar MET and SumET Rms Container
    SG::WriteHandle<xAOD::gFexGlobalRoIContainer> gScalarERmsContainer(m_gScalarERmsWriteKey, ctx);
    ATH_CHECK(gScalarERmsContainer.record(std::make_unique<xAOD::gFexGlobalRoIContainer>(), std::make_unique<xAOD::gFexGlobalRoIAuxContainer>()));
    ATH_MSG_DEBUG("Recorded gFexJetGlobalContainer with key " << gScalarERmsContainer.key());


    // Iterate over ROBFragments to decode
    for (const ROBF* rob : vrobf) {
        // Iterate over ROD words and decode
        
        
        ATH_MSG_DEBUG("Starting to decode " << rob->rod_ndata() << " ROD words from ROB 0x" << std::hex << rob->rob_source_id());
        
        //There is no data to decode.. not even the ROD trailers
        if(rob->rod_ndata() <= 0){
            continue;
        }
        
        const auto dataArray = CxxUtils::span{rob->rod_data(), rob->rod_ndata()};

        
        // Starting to loop over the gFEX words

        unsigned int n_words = rob->rod_ndata();

        //saving Jet TOBs into the EDM container
        for(unsigned int iWord=0; iWord<n_words; iWord++) {
            ATH_MSG_DEBUG("Raw word  0x" << std::hex << dataArray[iWord]  << "    " << std::bitset<32> (dataArray[iWord]));
        }
        
        // Vectors to temporarily store global tob before their are summed together
        int global_counter = 0;
        std::vector<uint32_t> JWOJ_MHT(3, 0);
        std::vector<uint32_t> JWOJ_MST(3, 0);
        std::vector<uint32_t> JWOJ_MET(3, 0);
        std::vector<uint32_t> JWOJ_SCALAR(3, 0);
        std::vector<uint32_t> NC_MET(3, 0);
        std::vector<uint32_t> NC_SCALAR(3, 0);
        std::vector<uint32_t> RMS_MET(3, 0);
        std::vector<uint32_t> RMS_SCALAR(3, 0);

        size_t index = 0;
        while ( index < n_words ) {
            const uint32_t headerWord = dataArray[index];//Identify the header words. The first is a header word.
            const uint32_t blockType  = (headerWord >> gPos::BLOCK_TYPE_BIT)  &    gPos::BLOCK_TYPE_MASK;
            const uint32_t headerSize = (headerWord >> gPos::HEADER_SIZE_BIT) &    gPos::HEADER_SIZE_MASK;
            const uint32_t errorFlags = (headerWord >> gPos::ERROR_FLAG_BIT)  &    gPos::ERROR_FLAG_MASK;
            const uint32_t dataSize   =  headerWord        &  gPos::DATA_SIZE_MASK;
      
            ATH_MSG_DEBUG( "index        "<< index );
            ATH_MSG_DEBUG( "word         "<< std::bitset<32> (dataArray[index]) );
            ATH_MSG_DEBUG( "headerWord   "<< std::bitset<32> (headerWord) );
            ATH_MSG_DEBUG( "blockType    "<< std::bitset<4> (blockType) );
            ATH_MSG_DEBUG( "headerSize   "<< std::bitset<2> (headerSize) );
            ATH_MSG_DEBUG( "errorFlags   "<< std::bitset<1> (errorFlags) );
            ATH_MSG_DEBUG( "dataSize     "<< std::bitset<12> (dataSize) );
            
            const uint32_t blockSize  = headerSize + dataSize;
            if ( (index + blockSize) > n_words ) {
                
                std::stringstream sdetail;
                sdetail  << "Remaining block size " << (n_words - index) << " is too small for subblock of type " << blockType << " with headerSize " << headerSize << " and dataSize " << dataSize ;
                std::stringstream slocation;
                slocation  << "0x"<< std::hex << rob->rob_source_id() << std::dec << " type:"<<blockType;
                std::stringstream stitle;
                stitle  << "Small subblock size " ;
                printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());  
                
            }

            index += headerSize;
      
            const uint32_t numSlices = dataSize / gPos::WORDS_PER_SLICE;
            ATH_MSG_DEBUG( "numSlices   "  <<  numSlices );

            if ( numSlices * gPos::WORDS_PER_SLICE != dataSize ) {

                std::stringstream sdetail;
                sdetail  << "L1CaloBsDecoderRun3::decodeGfexTobs: subblock type " << blockType << " with dataSize " << dataSize << " is not a multiple of " << gPos::WORDS_PER_SLICE << " words" ;
                std::stringstream slocation;
                slocation  << "0x"<< std::hex << rob->rob_source_id()<< std::dec << " type:"<<blockType;
                std::stringstream stitle;
                stitle  << "Wrong dataSize" ;
                printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());   
                     
                //skip decode of this fragment
                index+=dataSize;
                continue;
                
            }

            // The subblock type is 0xA,B,C for jet TOBs from FPGA A,B,C
            // and 0x1,2,3 for global (MET) TOBs.
            bool isMet = (blockType >= 0x1 && blockType <= 0x3);
            bool isJet = (blockType >= 0xA && blockType <= 0xC);

            
            for (uint32_t sliceNumber = 0; sliceNumber < numSlices; sliceNumber++) {
                if (sliceNumber == 0){
                    if ( !isJet && !isMet ) {
                        
                        std::stringstream sdetail;
                        sdetail  << "gFexByteStreamTool::decodeGfexTobSlice: Invalid block type " << blockType ;
                        std::stringstream slocation;
                        slocation  << "0x"<< std::hex << rob->rob_source_id();
                        std::stringstream stitle;
                        stitle  << "Invalid block type" ;
                        printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());                         
                        
                    }

                    for(unsigned int iWord=0; iWord<gPos::WORDS_PER_SLICE; iWord++) {

                        if (isJet) {
                            //Skipping the unused words
                            if (std::find(gPos::JET_UNUSED_POSITION.begin(),gPos::JET_UNUSED_POSITION.end(),iWord)!=gPos::JET_UNUSED_POSITION.end()){
                                continue;
                            }
                            //Skipping the trailer words
                            if (std::find(gPos::TRAILER_POSITION.begin(),gPos::TRAILER_POSITION.end(),iWord)!=gPos::TRAILER_POSITION.end()){
                                continue;
                            }
                            //Saving gRho TOBs into the EDM container
                            if (iWord == gPos::GRHO_POSITION){
                                std::unique_ptr<xAOD::gFexJetRoI> myEDM (new xAOD::gFexJetRoI());
                                gRhoContainer->push_back(std::move(myEDM));
                                gRhoContainer->back()->initialize(dataArray[index+iWord], m_gJ_scale);
                            }
                            //Saving gBlock TOBs into the EDM container
                            if (std::find(gPos::GBLOCK_POSITION.begin(),gPos::GBLOCK_POSITION.end(),iWord)!=gPos::GBLOCK_POSITION.end()){
                                std::unique_ptr<xAOD::gFexJetRoI> myEDM (new xAOD::gFexJetRoI());
                                gSJContainer->push_back(std::move(myEDM));
                                gSJContainer->back()->initialize(dataArray[index+iWord], m_gJ_scale);
                            }
                            //Saving gJet TOBs into the EDM container
                            if (std::find(gPos::GJET_POSITION.begin(),gPos::GJET_POSITION.end(),iWord)!=gPos::GJET_POSITION.end()){
                                std::unique_ptr<xAOD::gFexJetRoI> myEDM (new xAOD::gFexJetRoI());
                                gLJContainer->push_back(std::move(myEDM));
                                gLJContainer->back()->initialize(dataArray[index+iWord], m_gLJ_scale);
                            }

                        }

                        if (isMet){
                            //Skipping the unused words
                            if (std::find(gPos::GLOBAL_UNUSED_POSITION.begin(),gPos::GLOBAL_UNUSED_POSITION.end(),iWord)!=gPos::GLOBAL_UNUSED_POSITION.end()){
                                continue;
                            }
                            //Skipping the trailer words
                            if (std::find(gPos::TRAILER_POSITION.begin(),gPos::TRAILER_POSITION.end(),iWord)!=gPos::TRAILER_POSITION.end()){
                                continue;
                            }
                            //Saving jwoj MHT TOBs into the EDM container
                            if (iWord == gPos::JWOJ_MHT_POSITION){
                                global_counter ++;
                                if (blockType == 0x1) {JWOJ_MHT[0] = dataArray[index+iWord];}
                                if (blockType == 0x2) {JWOJ_MHT[1] = dataArray[index+iWord];}
                                if (blockType == 0x3) {JWOJ_MHT[2] = dataArray[index+iWord];}
                            }
                            //Saving jwoj MST TOBs into the EDM container
                            if (iWord == gPos::JWOJ_MST_POSITION){
                                if (blockType == 0x1) {JWOJ_MST[0] = dataArray[index+iWord];}
                                if (blockType == 0x2) {JWOJ_MST[1] = dataArray[index+iWord];}
                                if (blockType == 0x3) {JWOJ_MST[2] = dataArray[index+iWord];}
                            }
                            //Saving jwoj MET TOBs into the EDM container
                            if (iWord == gPos::JWOJ_MET_POSITION){
                                if (blockType == 0x1) {JWOJ_MET[0] = dataArray[index+iWord];}
                                if (blockType == 0x2) {JWOJ_MET[1] = dataArray[index+iWord];}
                                if (blockType == 0x3) {JWOJ_MET[2] = dataArray[index+iWord];}
                            }
                            //Saving jwoj Scalar TOBs into the EDM container
                            if (iWord == gPos::JWOJ_SCALAR_POSITION){
                                if (blockType == 0x1) {JWOJ_SCALAR[0] = dataArray[index+iWord];}
                                if (blockType == 0x2) {JWOJ_SCALAR[1] = dataArray[index+iWord];}
                                if (blockType == 0x3) {JWOJ_SCALAR[2] = dataArray[index+iWord];}
                            }
                            //Saving Noise Cut MET TOBs into the EDM container
                            if (iWord == gPos::NC_MET_POSITION){
                                if (blockType == 0x1) {NC_MET[0] = dataArray[index+iWord];}
                                if (blockType == 0x2) {NC_MET[1] = dataArray[index+iWord];}
                                if (blockType == 0x3) {NC_MET[2] = dataArray[index+iWord];}
                            }
                            //Saving Noise Cut Scalar TOBs into the EDM container
                            if (iWord == gPos::NC_SCALAR_POSITION){
                                if (blockType == 0x1) {NC_SCALAR[0] = dataArray[index+iWord];}
                                if (blockType == 0x2) {NC_SCALAR[1] = dataArray[index+iWord];}
                                if (blockType == 0x3) {NC_SCALAR[2] = dataArray[index+iWord];}
                            }
                            //Saving Rho+RMS MET TOBs into the EDM container
                            if (iWord == gPos::RMS_MET_POSITION){
                                if (blockType == 0x1) {RMS_MET[0] = dataArray[index+iWord];}
                                if (blockType == 0x2) {RMS_MET[1] = dataArray[index+iWord];}
                                if (blockType == 0x3) {RMS_MET[2] = dataArray[index+iWord];}
                            }
                            //Saving Rho+RMS Scalar TOBs into the EDM container
                            if (iWord == gPos::RMS_SCALAR_POSITION){
                                if (blockType == 0x1) {RMS_SCALAR[0] = dataArray[index+iWord];}
                                if (blockType == 0x2) {RMS_SCALAR[1] = dataArray[index+iWord];}
                                if (blockType == 0x3) {RMS_SCALAR[2] = dataArray[index+iWord];}
                            }

                        }

                    }

                }
                index += gPos::WORDS_PER_SLICE;
            }

            ATH_MSG_DEBUG("global_counter is " << global_counter);
            if (global_counter == 3) {

                fillGlobal(JWOJ_MHT, 3, gMHTComponentsJwojContainer);
                fillGlobal(JWOJ_MST, 4, gMSTComponentsJwojContainer);
                int16_t scalar = fillGlobal(JWOJ_MET, 2, gMETComponentsJwojContainer);
                fillGlobal(JWOJ_SCALAR, 1, gScalarEJwojContainer, scalar);
                                
                scalar = fillGlobal(NC_MET, 2, gMETComponentsNoiseCutContainer);
                fillGlobal(NC_SCALAR, 1, gScalarENoiseCutContainer, scalar);

                scalar = fillGlobal(RMS_MET, 2, gMETComponentsRmsContainer);
                fillGlobal(RMS_SCALAR, 1, gScalarERmsContainer, scalar);

                global_counter = 0;
            }
            
        }
    }
    return StatusCode::SUCCESS;
} 

// For MHT, MST, and MET, it sums the x and y components across FPGAs, and also returnes
// the sum in quadrature (which is actually only used for MET, and discared for MHT and MST)
// This function also accepts "scalar" as optional variable, which is used to fill the X
// component of the SCALAR tob.
int16_t gFexByteStreamTool::fillGlobal(const std::vector<uint32_t> &tob, const int type,
                                       SG::WriteHandle<xAOD::gFexGlobalRoIContainer> &container,
                                       int16_t scalar/* = -1*/) const {
    
    ATH_MSG_DEBUG("fillGlobal with type " << type);

    int16_t sum_x = 0;
    int16_t sum_y = 0;

    // Extract the x and y components and sum them for the three FPGAs
    for (size_t fpga = 0; fpga < 3; fpga++) {
        int16_t x = tob[fpga] >> gPos::GLOBAL_X_BIT & gPos::GLOBAL_X_MASK;
        int16_t y = tob[fpga] >> gPos::GLOBAL_Y_BIT & gPos::GLOBAL_Y_MASK;
        if (x & 0x00080000) { x  = 0xFFFF0000 | x;  }
        if (y & 0x00080000) { y  = 0xFFFF0000 | y;  }
        sum_x += x;
        sum_y += y;
    }

    if (type == 1) {//we are considering the scalar case (sum_x = MET and sum_y = SumEt) 
        ATH_MSG_DEBUG("  scalar tob, saving " << scalar << " in X component");
        sum_x = scalar; //Total MET
        if( sum_y > 0x000FFF) sum_y = 0x000FFF; //Overflow control for SumEt
        if( sum_y < 0) sum_y = 0;

    } else {
        if (sum_x < -0x000800) sum_x = -0x000800; //-2048
        if (sum_x > 0x0007FF) sum_x  = 0x0007FF; //2047

        if (sum_y < -0x000800) sum_y = -0x000800; //-2048
        if (sum_y > 0x0007FF) sum_y  = 0x0007FF; //2047
    }

    ATH_MSG_DEBUG("  fillGlobal type " << type << std::dec << " sum_x " << sum_x << " sum_y " << sum_y);

    uint32_t METword = 0;

    METword = (sum_y &  0x00000FFF) << 0; //set the Quantity2 to the corresponding slot (LSB)
    METword = METword | (sum_x  &  0x00000FFF) << 12;//Quantity 1 (in bit number 12)
    if (sum_y != 0) METword = METword | 0x00000001 << 24;//Status bit for Quantity 2 (0 if quantity is null)
    if (sum_x != 0) METword = METword | 0x00000001 << 25;//Status bit for Quantity 1 (0 if quantity is null)
    METword = METword | (type  &  0x0000001F) << 26;//TOB ID (5 bits starting at 26)

    // Save to the EDM
    std::unique_ptr<xAOD::gFexGlobalRoI> myEDM (new xAOD::gFexGlobalRoI());
    container->push_back(std::move(myEDM));
    container->back()->setWord(METword);
    container->back()->setQuantityOne(sum_x);
    container->back()->setQuantityTwo(sum_y);
    container->back()->setScaleOne(m_gXE_scale);
    container->back()->setScaleTwo(m_gTE_scale);
    container->back()->setStatusOne(1);
    container->back()->setStatusTwo(1);
    container->back()->setSaturated(0);
    container->back()->setGlobalType(type);
    

    int MET2 = sum_x * sum_x + sum_y * sum_y;
    int16_t MET = std::sqrt(MET2);
    if (MET > 0x000FFF) MET = 0x000FFF;

    return MET;

}


/// xAOD->BS conversion
StatusCode gFexByteStreamTool::convertToBS(std::vector<WROBF*>& /*vrobf*/, const EventContext& /*eventContext*/) {

    return StatusCode::SUCCESS;
    
}


void  gFexByteStreamTool::printError(const std::string& location, const std::string& title, MSG::Level type, const std::string& detail) const{
    
    if(m_UseMonitoring){
        Monitored::Group(m_monTool,
                     Monitored::Scalar("gfexDecoderErrorLocation",location.empty() ? std::string("UNKNOWN") : location),
                     Monitored::Scalar("gfexDecoderErrorTitle"   ,title.empty()    ? std::string("UNKNOWN") : title)
                     );
    }
    else {
        msg() << type << detail << endmsg;
    }
}
