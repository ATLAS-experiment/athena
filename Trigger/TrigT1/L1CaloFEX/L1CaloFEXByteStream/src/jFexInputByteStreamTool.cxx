/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           jFexInputByteStreamTool  -  This tool decodes Run3 jFEX input data!
//                              -------------------
//     begin                : 01 07 2022
//     email                : Sergi.Rodriguez@cern.ch
//  ***************************************************************************/

#include "jFexInputByteStreamTool.h"
#include "jFexBits.h"
#include "eformat/SourceIdentifier.h"
#include "eformat/Status.h"

#include <span>
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

jFexInputByteStreamTool::jFexInputByteStreamTool(const std::string& type,
        const std::string& name,
        const IInterface* parent)
    : base_class(type, name, parent) {}

StatusCode jFexInputByteStreamTool::initialize() {

    ATH_MSG_INFO( "Initializing L1CaloFEXByteStream/jFexInputByteStreamTool algorithm with name: "<< name());
    ATH_MSG_INFO( "Reading into SG key: "<< m_jTowersReadKey);
    ATH_MSG_INFO( "Writting into SG key: "<< m_jTowersWriteKey);
    ATH_MSG_INFO( "Thinnig towers: "<< m_doThinning);    
    
    
    // Conversion mode for jTowers
    ConversionMode jTowersmode = getConversionMode(m_jTowersReadKey, m_jTowersWriteKey, msg());
    ATH_CHECK(jTowersmode!=ConversionMode::Undefined);
    ATH_CHECK(m_jTowersWriteKey.initialize(jTowersmode==ConversionMode::Decoding));
    ATH_CHECK(m_jTowersReadKey.initialize(jTowersmode==ConversionMode::Encoding));
    ATH_MSG_DEBUG((jTowersmode==ConversionMode::Encoding ? "Encoding" : "Decoding") << " jTowers ROB IDs: " << MSG::hex << m_robIds.value() << MSG::dec);
    
    
    
    //Since the mapping is constant in everyentry, better to be read in the initialize function
    //Reading from CVMFS Fiber mapping
    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_FiberMapping)));
    
    
    // Initialize monitoring tool if not empty
    if (!m_monTool.empty()) {
        ATH_CHECK(m_monTool.retrieve());
        ATH_MSG_INFO("Logging errors to " << m_monTool.name() << " monitoring tool");
        m_UseMonitoring = true;
    }


    return StatusCode::SUCCESS;
}

// BS->xAOD conversion
StatusCode jFexInputByteStreamTool::convertFromBS(const std::vector<const ROBF*>& vrobf, const EventContext& ctx) const {
    
    //WriteHandle for jFEX EDMs
    
    //---jTower EDM
    SG::WriteHandle<xAOD::jFexTowerContainer> jTowersContainer(m_jTowersWriteKey, ctx);
    ATH_CHECK(jTowersContainer.record(std::make_unique<xAOD::jFexTowerContainer>(), std::make_unique<xAOD::jFexTowerAuxContainer>()));
    ATH_MSG_DEBUG("Recorded jFexTowerContainer with key " << jTowersContainer.key());
    
    

    // Iterate over ROBFragments to decode
    for (const ROBF* rob : vrobf) {
        // Iterate over ROD words and decode
        
        
        ATH_MSG_DEBUG("Starting to decode " << rob->rod_ndata() << " ROD words from ROB 0x" << std::hex << rob->rob_source_id() << std::dec);
        
        //There is no data to decode.. not even the ROD trailers
        if(rob->rod_ndata() <= 0){
            ATH_MSG_DEBUG(C.B_RED<<"  No ROD words to decode: " << rob->rod_ndata() <<" in ROB 0x"<< std::hex << rob->rob_source_id()<< std::dec <<". Skipping"<<C.END);
            continue;
        }
        
        const auto dataArray = std::span{rob->rod_data(), rob->rod_ndata()};
        std::vector<uint32_t> vec_words(dataArray.begin(),dataArray.end());
        
        // jFEX to ROD trailer position
        unsigned int trailers_pos = rob->rod_ndata();
        
        // Starting to loop over the different jFEX blocks
        bool READ_WORDS = true;
        while(READ_WORDS){
            
            if( trailers_pos < jBits::jFEX2ROD_WORDS ){
                std::stringstream sdetail;
                sdetail  << "There are not enough words ("<< trailers_pos <<") for the jFEX to ROD trailer decoder. Expected at least " << jBits::jFEX2ROD_WORDS<<". Skipping this FPGA(?)" ;
                std::stringstream slocation;
                slocation  << "0x"<< std::hex << rob->rob_source_id();
                std::stringstream stitle;
                stitle  << "Invalid trailer words" ;
                printError(slocation.str(),stitle.str(),MSG::WARNING,sdetail.str());                


                READ_WORDS = false;
                continue;
            }
            
            const auto [payload, jfex, fpga, error] = jFEXtoRODTrailer  ( vec_words.at(trailers_pos-2), vec_words.at(trailers_pos-1) );
            
            if(error != 0 ){
                
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
                    printError(slocation.str(),stitle.str(),MSG::WARNING,sdetail.str());
                    break;
                }   
                if( safemode ){
                    std::stringstream stitle;
                    stitle  << "Safe Mode" ;
                    printError(slocation.str(),stitle.str(),MSG::WARNING,sdetail.str()); 
                    break;                  
                }   
                if( proterror ){
                    std::stringstream stitle;
                    stitle  << "Protocol error" ;
                    printError(slocation.str(),stitle.str(),MSG::WARNING,sdetail.str());  
                    break;                  
                }   
                if( lenerror ){
                    std::stringstream stitle;
                    stitle  << "Length mismatch" ;
                    printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str()); 
                    break;                   
                }   
                if( headmismatch ){
                    std::stringstream stitle;
                    stitle  << "Header mismatch" ;
                    printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());
                    break;                    
                }   
                if( processerror ){
                    std::stringstream stitle;
                    stitle  << "Processor Timeout" ;
                    printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str()); 
                    break;                   
                }   
                break;
            }            
            
            if(payload % jBits::DATA_BLOCKS != 0){
                ATH_MSG_DEBUG("  Not full readout activated (" << payload << "). Data blocks/channels expected (" << jBits::DATA_BLOCKS <<")"<<C.END);
            }
            
            if(payload % jBits::DATA_WORDS_PER_BLOCK != 0) {
                std::stringstream sdetail;
                sdetail  << "  Payload number (" << payload << ") not a multiple of data words per channel. Expected: " << jBits::DATA_WORDS_PER_BLOCK ;
                std::stringstream slocation;
                slocation  << "jFEX "<< jfex << " FPGA "<< fpga << " in 0x"<< std::hex << rob->rob_source_id();
                std::stringstream stitle;
                stitle  << "Invalid Data Blocks" ;
                printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());

                READ_WORDS = false;
                continue;
            }
            
            //Position index, removing jFEX to ROD, TOB and xTOB Trailers from trailers_pos (4 positions), possible padding word added on the data to get even number of 32bit words
            unsigned int wordIndex = trailers_pos - (jBits::jFEX2ROD_WORDS);      
            
            // Number of iterations that must be done. It is divisible otherwise it throws out an error (Line 108)
            unsigned int Max_iter = payload/jBits::DATA_WORDS_PER_BLOCK;
            
            if(Max_iter>trailers_pos) {
                std::stringstream sdetail;
                sdetail  << "Block size error in fragment 0x"<< std::hex << rob->rob_source_id() << std::dec<<". Words available: " << trailers_pos << ". Number of words wanted to decode: " << Max_iter ;
                std::stringstream slocation;
                slocation  << "jFEX "<< jfex << " FPGA "<< fpga << " in 0x"<< std::hex << rob->rob_source_id();
                std::stringstream stitle;
                stitle  << "Block Size Error" ;
                
                //Currently under investigation... 
                printError(slocation.str(),stitle.str(),MSG::DEBUG,sdetail.str());
                return StatusCode::SUCCESS;
                
                //DO NOT REMOVE, once fixed it should return and error and FAILURE
                //printError(slocation.str(),stitle.str(),m_ERROR,sdetail.str());
                //return StatusCode::FAILURE;
            }
            
            for (unsigned int iblock = 0; iblock < Max_iter; iblock++){
                const auto [channel, saturation] = BulkStreamTrailer(vec_words.at(wordIndex-1),vec_words.at(wordIndex-2), jfex);
                                
                const auto [DATA13_low          , DATA15, DATA14] = Dataformat1(vec_words.at(wordIndex-3));
                const auto [DATA13_up,DATA10_up , DATA12, DATA11] = Dataformat2(vec_words.at(wordIndex-4));
                const auto [DATA10_low          , DATA9 , DATA8 ] = Dataformat1(vec_words.at(wordIndex-5));
                const auto [DATA5_low           , DATA7 , DATA6 ] = Dataformat1(vec_words.at(wordIndex-6));
                const auto [DATA5_up ,DATA2_up  , DATA4 , DATA3 ] = Dataformat2(vec_words.at(wordIndex-7));
                const auto [DATA2_low           , DATA1 , DATA0 ] = Dataformat1(vec_words.at(wordIndex-8));
                
                //uncomment for mergeing splitted Et
                uint16_t DATA2  = ( DATA2_up  << jBits::BS_MERGE_DATA ) + DATA2_low;
                uint16_t DATA5  = ( DATA5_up  << jBits::BS_MERGE_DATA ) + DATA5_low;
                uint16_t DATA10 = ( DATA10_up << jBits::BS_MERGE_DATA ) + DATA10_low;
                uint16_t DATA13 = ( DATA13_up << jBits::BS_MERGE_DATA ) + DATA13_low;
                
                std::array<uint16_t,16> allDATA = {DATA0, DATA1, DATA2, DATA3, DATA4, DATA5, DATA6, DATA7, DATA8, DATA9, DATA10, DATA11, DATA12, DATA13, DATA14, DATA15 };
                //std::array<uint16_t,16> allsat  = {0};
                
                for(uint idata = 0; idata < allDATA.size(); idata++){
                    
                    char et_saturation = ((saturation >> idata) & jBits::BS_TRAILER_1b);
                    
                    //allsat[idata] = et_saturation;
                    
                    // read ID, eta and phi from map
                    unsigned int intID = mapIndex(jfex, fpga, channel, idata);

                    // Exists the jTower in the mapping?
                    auto it_Firm2Tower_map = m_Firm2Tower_map.find(intID);
                    if(it_Firm2Tower_map == m_Firm2Tower_map.end()){

                        // jTower not found in the mapping.. skipping. 
                        // If we are here means that the jTower is not actually used in the firmware
                        continue;
                    }

                    const auto [IDsim, eta, phi, source, iEta, iPhi] = it_Firm2Tower_map->second;
                    
                    std::vector<uint16_t> vtower_ET;
                    vtower_ET.clear();
                    vtower_ET.push_back(allDATA[idata]);
                    
                    std::vector<char> vtower_SAT;
                    vtower_SAT.clear();
                    vtower_SAT.push_back(et_saturation);
                    
                    //initilize the jTower EDM
                    jTowersContainer->push_back( std::make_unique<xAOD::jFexTower>() );
                    jTowersContainer->back()->initialize(eta, phi, iEta, iPhi, IDsim, source, vtower_ET, jfex, fpga, channel, idata, vtower_SAT );

                    if( m_doThinning && !jTowersContainer->back()->isCore() ){
                        jTowersContainer->pop_back(); 
                    }                    
                }
                
                // keeping this for future x-checks
                //if(m_verbose ){
                    
                    //printf("DATA00 :%5x   DATA01 :%5x   DATA02_lo :%5x\n"                  , DATA0  ,DATA1  ,DATA2_low);
                    //printf("DATA03 :%5x   DATA04 :%5x   DATA02_up :%5x   DATA05_up :%5x\n" , DATA3  ,DATA4  ,DATA2_up  ,DATA5_up);
                    //printf("DATA06 :%5x   DATA07 :%5x   DATA05_lo :%5x\n"                  , DATA6  ,DATA7  ,DATA5_low);
                    //printf("DATA08 :%5x   DATA09 :%5x   DATA10_lo :%5x\n"                  , DATA8  ,DATA9  ,DATA10_low);
                    //printf("DATA11 :%5x   DATA12 :%5x   DATA10_up :%5x   DATA13_up :%5x\n" , DATA11 ,DATA12 ,DATA10_up  ,DATA13_up  );
                    //printf("DATA14 :%5x   DATA15 :%5x   DATA13_lo :%5x\n"                  , DATA14 ,DATA15 ,DATA13_low);
                    //printf("*merged* DATA02 :%5x   DATA05 :%5x   DATA10 :%5x   DATA13 :%5x\n" , DATA2 ,DATA5 ,DATA10  ,DATA13  );
                    
                    //printf("DATA0 :%5d   DATA1 :%5d   DATA2 :%5d   DATA3 :%5d\n",allsat[0]  ,allsat[1]  ,allsat[2]  ,allsat[3]  );
                    //printf("DATA4 :%5d   DATA5 :%5d   DATA6 :%5d   DATA7 :%5d\n",allsat[4]  ,allsat[5]  ,allsat[6]  ,allsat[7]  );
                    //printf("DATA8 :%5d   DATA8 :%5d   DATA10:%5d   DATA11:%5d\n",allsat[8]  ,allsat[9]  ,allsat[10] ,allsat[11] );
                    //printf("DATA12:%5d   DATA13:%5d   DATA14:%5d   DATA15:%5d\n",allsat[12] ,allsat[13] ,allsat[14] ,allsat[15] );
                //}

                
                wordIndex -= jBits::DATA_WORDS_PER_BLOCK;
            }
            
            //moving trailer position index to the next jFEX data block
            trailers_pos -= (payload + jBits::jFEX2ROD_WORDS);
            
            if(trailers_pos == 0){
                READ_WORDS = false;
            }
        }
    }
        
    return StatusCode::SUCCESS;
}


// Unpack jFEX to ROD Trailer
std::array<uint32_t,4> jFexInputByteStreamTool::jFEXtoRODTrailer (uint32_t word0, uint32_t word1) const {
    
    uint32_t payload    = ((word0 >> jBits::PAYLOAD_ROD_TRAILER ) & jBits::ROD_TRAILER_16b);
    uint32_t jfex       = ((word0 >> jBits::jFEX_ROD_TRAILER    ) & jBits::ROD_TRAILER_4b );
    uint32_t fpga       = ((word0 >> jBits::FPGA_ROD_TRAILER    ) & jBits::ROD_TRAILER_2b );
    
    uint32_t error      = ((word1 >> jBits::ERROR_ROD_TRAILER   ) & jBits::ROD_TRAILER_6b );
    
    return {payload,jfex,fpga,error};
   
}

// Unpack Bulk stream trailer
std::array<uint16_t,2> jFexInputByteStreamTool::BulkStreamTrailer (uint32_t word0, uint32_t word1, uint32_t jfex) const {
    
    uint16_t Satur_high = ((word1 >> jBits::BS_SATUR_1_TRAILER ) & jBits::BS_TRAILER_8b );
    uint16_t Satur_down = ((word1 >> jBits::BS_SATUR_0_TRAILER ) & jBits::BS_TRAILER_8b );
    uint16_t Channel    = ((word0 >> jBits::BS_CHANNEL_TRAILER ) & jBits::BS_TRAILER_8b );
    
    //Tile fibre are decoded differently, no saturation bits are set for tile  
    bool isTileFibre = false;
    for(unsigned int i=0; i< jBits::tile_channels.at(jfex).size(); i++){
        if(jBits::tile_channels.at(jfex).at(i) == Channel){
            isTileFibre=true;
            break;
        }
    }
    
    if(isTileFibre){
        //If it is tile then return just the channel number and sat = 0
        return {Channel, 0};
    }
    
    uint16_t Satur = ( Satur_high << jBits::BS_SATUR_1_TRAILER ) + Satur_down;
    
    return {Channel, Satur};
   
}


// Unpack Bulk stream data format 1 : DATA4 [7:0], DATA1 [12:0], DATA0 [12:0]
std::array<uint16_t,3>  jFexInputByteStreamTool::Dataformat1 (uint32_t word0) const {
    
    uint16_t data_low  = ((word0 >> jBits::BS_ET_DATA_0 ) & jBits::BS_TRAILER_12b);
    uint16_t data_mid  = ((word0 >> jBits::BS_ET_DATA_1 ) & jBits::BS_TRAILER_12b );
    uint16_t data_up   = ((word0 >> jBits::BS_ET_DATA_4 ) & jBits::BS_TRAILER_8b );
    
    return {data_up,data_mid,data_low};
}


// Unpack Bulk stream data format 2 : DATA7 [11:8], DATA4 [11:8], DATA1 [12:0], DATA0 [12:0]
std::array<uint16_t,4>  jFexInputByteStreamTool::Dataformat2 (uint32_t word0) const {
    
    uint16_t data_low  = ((word0 >> jBits::BS_ET_DATA_0 ) & jBits::BS_TRAILER_12b);
    uint16_t data_mid  = ((word0 >> jBits::BS_ET_DATA_1 ) & jBits::BS_TRAILER_12b );
    uint16_t data_up_1 = ((word0 >> jBits::BS_ET_DATA_4 ) & jBits::BS_TRAILER_4b );
    uint16_t data_up_2 = ((word0 >> jBits::BS_ET_DATA_7 ) & jBits::BS_TRAILER_4b );

    return {data_up_2,data_up_1,data_mid,data_low};
}



/// xAOD->BS conversion
StatusCode jFexInputByteStreamTool::convertToBS(std::vector<WROBF*>& /*vrobf*/,
                                                const xAOD::TrigCompositeContainer* /*tc*/,
                                                const EventContext& /*eventContext*/) {
    
/*
    // Retrieve the RoI container
    auto muonRoIs = SG::makeHandle(m_roiReadKey, eventContext);
    ATH_CHECK(muonRoIs.isValid());

    // Clear BS data cache
    clearCache(eventContext);

    // Create raw ROD data words
    ATH_MSG_DEBUG("Converting " << muonRoIs->size() << " L1 Muon RoIs to ByteStream");
    uint32_t* data = newRodData(eventContext, muonRoIs->size());
    for (size_t i=0; i<muonRoIs->size(); ++i) {
        data[i] = muonRoIs->at(i)->roiWord();
    }

    // Create ROBFragment containing the ROD words
    const eformat::helper::SourceIdentifier sid(eformat::TDAQ_MUON_CTP_INTERFACE, m_muCTPIModuleID.value());
    vrobf.push_back(newRobFragment(
                        eventContext,
                        sid.code(),
                        muonRoIs->size(),
                        data,
                        eformat::STATUS_BACK // status_position is system-specific
                    ));
*/
    return StatusCode::SUCCESS;
}

StatusCode jFexInputByteStreamTool::ReadfromFile(const std::string & fileName){
    
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
        
        // It should have 10 elements
        // ordered as:  jfex fpga channel towerNr source globalEtaIndex globalPhiIndex IDSimulation eta phi
        if(elements.size() != 10){
            ATH_MSG_ERROR("Unexpected number of elemennts (10 expected) in file: "<< fileName);
            return StatusCode::FAILURE;
        }
        // building array of  <IDSimulation, eta, phi, source, iEta, iPhi>
        std::array<float,6> aux_arr{ {elements.at(7),elements.at(8),elements.at(9),elements.at(4),elements.at(5),elements.at(6)} };
        
        //filling the map with the hash given by mapIndex()
        m_Firm2Tower_map[ mapIndex(elements.at(0),elements.at(1),elements.at(2),elements.at(3)) ] = aux_arr;
        
    }
    myfile.close();

    return StatusCode::SUCCESS;
}


constexpr unsigned int jFexInputByteStreamTool::mapIndex(unsigned int jfex, unsigned int fpga, unsigned int channel, unsigned int tower) {
  // values from hardware: jfex=[0,5] 4 bits, fpga=[0,3] 4 bits, channel=[0,59] 8 bits, tower=[0,15] 4 bits
  return (jfex << 16) | (fpga << 12) | (channel << 4) | tower;
}

void  jFexInputByteStreamTool::printError(const std::string& location, const std::string& title, MSG::Level type, const std::string& detail) const{
    
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
