/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           jFexEmulatedTowers  -  description
//                              -------------------
//      This reentrant algorithm is meant build jFEX Towers from LAr and Tile
//      Information about SCellContainer objetcs are in:
//          - https://gitlab.cern.ch/atlas/athena/-/blob/22.0/Calorimeter/CaloEvent/CaloEvent/CaloCell.h
//      
//     begin                : 01 11 2022
//     email                : sergi.rodriguez@cern.ch
//***************************************************************************/


#include "jFexEmulatedTowers.h"
#include "L1CaloFEXSim/jFEXCompression.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <stdio.h> 

namespace LVL1 {

jFexEmulatedTowers::jFexEmulatedTowers(const std::string& name, ISvcLocator* svc) : AthReentrantAlgorithm(name, svc){}

StatusCode jFexEmulatedTowers::initialize() {
    
    ATH_MSG_INFO( "Initializing L1CaloFEXAlgos/jFexEmulatedTowers algorithm with name: "<< name());
    ATH_MSG_INFO( "Writting into SG key: "<< m_jTowersWriteKey);
    ATH_MSG_INFO( "SCell masking: "<< m_apply_masking);
    ATH_MSG_INFO( "Thinnig towers: "<< m_doThinning);
    
    ATH_CHECK( m_SCellKey.initialize() );
    ATH_CHECK( m_triggerTowerKey.initialize() );
    ATH_CHECK( m_jTowersWriteKey.initialize() );
    

    //Reading from CVMFS Fiber mapping
    ATH_CHECK(ReadFibersfromFile(m_FiberMapping));    
    
    //Reading from CVMFS Trigger Tower and their corresponding SCell ID
    ATH_CHECK(ReadSCfromFile(m_jFEX2Scellmapping));
    ATH_CHECK(ReadTilefromFile(m_jFEX2Tilemapping));
    
    return StatusCode::SUCCESS;
}

StatusCode jFexEmulatedTowers::execute(const EventContext& ctx) const {
    
    //Reading the Scell container
    SG::ReadHandle<CaloCellContainer> ScellContainer(m_SCellKey, ctx);
    if(!ScellContainer.isValid()) {
        ATH_MSG_ERROR("Could not retrieve collection " << ScellContainer.key() );
        return StatusCode::FAILURE;
    }
    
    //Reading the TriggerTower container
    SG::ReadHandle<xAOD::TriggerTowerContainer> triggerTowerContainer(m_triggerTowerKey, ctx);
    if(!triggerTowerContainer.isValid()) {
        ATH_MSG_ERROR("Could not retrieve collection " << triggerTowerContainer.key() );
        return StatusCode::FAILURE;
    }

    //WriteHandle for jFEX EDMs
    //---jTower EDM
    SG::WriteHandle<xAOD::jFexTowerContainer> jTowersContainer(m_jTowersWriteKey, ctx);
    ATH_CHECK(jTowersContainer.record(std::make_unique<xAOD::jFexTowerContainer>(), std::make_unique<xAOD::jFexTowerAuxContainer>()));
    ATH_MSG_DEBUG("Recorded jFexEmulatedTower container with key " << jTowersContainer.key());
     
    if(ScellContainer->empty() || triggerTowerContainer->empty() ){
        ATH_MSG_WARNING("Cannot fill jTowers here, at least one container is empty. ScellContainer.size="<<ScellContainer->size() << " or triggerTowerContainer.size=" << triggerTowerContainer->size() );
        return StatusCode::SUCCESS;
    }
    
    // building Scell ID pointers
    std::unordered_map< uint64_t, const CaloCell*> map_ScellID2ptr;
    
    for(const CaloCell* scell : *ScellContainer){
        const uint64_t ID = scell->ID().get_compact();
        map_ScellID2ptr[ID] = scell;
    }
    
    // building Tile ID pointers
    std::unordered_map< uint32_t, const xAOD::TriggerTower*> map_TileID2ptr;
    
    for(const xAOD::TriggerTower* tower : *triggerTowerContainer){
        
        // keeping just tile information
        if(std::abs(tower->eta())>1.5 || tower->sampling()!=1) continue;
        
        map_TileID2ptr[tower->coolId()]=tower;
    }
    
    
    for( const auto& [key, element] : m_Firm2Tower_map){
        
        unsigned int jfex    = (key >> 16) & 0xf;
        unsigned int fpga    = (key >> 12) & 0xf;
        unsigned int channel = (key >> 4 ) & 0xff;
        unsigned int tower   = (key >> 0 ) & 0xf; 
               
        const auto [f_IDSimulation, eta, phi, f_source, f_iEta, f_iPhi] = element;
        
        //elements that need to be casted
        unsigned int IDSimulation = static_cast<int>(f_IDSimulation);
        unsigned int source       = static_cast<int>(f_source);
        unsigned int iEta         = static_cast<int>(f_iEta);
        unsigned int iPhi         = static_cast<int>(f_iPhi);
        
        
        uint16_t Total_Et_encoded = 0;
        char jTower_sat = 0;
        
        if( source != 1 ){
            
            const std::unordered_map< uint32_t, std::vector<uint64_t> > * ptr_TTower2Cells;
            
            //HAD layer for HEC, FCAL2 and FCAL3
            if(source == 3 or source > 4){
                ptr_TTower2Cells = &m_map_TTower2SCellsHAD;
            } 
            else{
                ptr_TTower2Cells = &m_map_TTower2SCellsEM;
            }  
            
            //check that the jFEX Tower ID exists in the map
            auto it_TTower2SCells = (*ptr_TTower2Cells).find(IDSimulation);
            if(it_TTower2SCells == (*ptr_TTower2Cells).end()) {
                ATH_MSG_ERROR("jFEX ID: "<<IDSimulation<< " not found on map m_map_TTower2SCellsEM/HAD");
                return StatusCode::FAILURE;
            }
        
            int Total_Et = 0;
            float Total_Et_float = 0;
            bool invalid = m_apply_masking&&m_isDATA; // the isDATA is because still waiting for a decision from LAr on whether we should be masking invalid cells in mc or not
            bool masked = m_apply_masking;
            for (auto const& SCellID : it_TTower2SCells->second ) {
                //check that the SCell Identifier exists in the map
                auto it_ScellID2ptr = map_ScellID2ptr.find(SCellID);
                if(it_ScellID2ptr == map_ScellID2ptr.end()) {
                    if(m_isDATA) ATH_MSG_DEBUG("Scell ID: 0x"<<std::hex<< (SCellID >> 32) <<std::dec<< " not found in the CaloCell Container, skipping");
                    // this is equivalent to treat the scell as a masked input, since masking takes precedence over invalidity
                    continue;
                }

                const CaloCell* myCell = it_ScellID2ptr->second;
                int val =  std::round(myCell->energy()/(12.5*std::cosh(myCell->eta()))); // 12.5 is b.c. energy is in units of 12.5MeV per count
                bool isMasked = m_apply_masking ? ((myCell)->provenance()&0x80) : false;
                bool isInvalid = m_apply_masking ? ((myCell)->provenance()&0x40) : false;
                bool isSaturated = myCell->quality();

                invalid &= isInvalid;
                masked &= isMasked;
                jTower_sat |= isSaturated;

                if( isMasked ) {
                    //if masked then Et = 0
                    val = 0;
                    //countMasked++;
                } else if( isInvalid&&m_isDATA /* remove isDATA check when answer question above about MC treatment of invalid SCs*/ ) {
                    val = 0;
                }
                
                Total_Et += val;
                if(val!=0) Total_Et_float += myCell->et();
                
            }

            // now must convert Total_Et int value into fex value: multi-level encoding
            if(masked) {
                Total_Et_encoded = 0; // no data
            } else if(invalid) {
                Total_Et_encoded = 4095; // invalid
            } else {
                Total_Et_encoded = jFEXCompression::Compress( Total_Et*12.5, false );
            }

            // leaving this commented while outstanding questions above about treatment of supercells
            // using floating point for MC until determine correct procedure for MC values re invalid/masking
            if(!m_isDATA) Total_Et_encoded = jFEXCompression::Compress( Total_Et_float, masked );
            
        }
        else{
            
            //check that the jFEX Tower ID exists in the map
            auto it_TTower2Tile = m_map_TTower2Tile.find(IDSimulation);
            if(it_TTower2Tile == m_map_TTower2Tile.end()) {
                ATH_MSG_ERROR("ID: "<<IDSimulation<< " not found on map m_map_TTower2Tile");
                return StatusCode::FAILURE;
            }
            
            uint32_t TileID = std::get<0>( it_TTower2Tile->second );
            
            //check that the Tile Identifier exists in the map
            auto it_TileID2ptr = map_TileID2ptr.find(TileID);
            if(it_TileID2ptr == map_TileID2ptr.end()) {
                if(m_isDATA) ATH_MSG_WARNING("Tile cool ID: "<<TileID<< " not found in the xAOD::TriggerTower, skipping");
                continue;
            }
            else{
                Total_Et_encoded = (it_TileID2ptr->second)->cpET();
            }           
        }
        
        std::vector<uint16_t> vtower_ET;
        vtower_ET.clear();
        vtower_ET.push_back(Total_Et_encoded);
        
        std::vector<char> vtower_SAT;
        vtower_SAT.clear();
        
        //Needs to be updated with Saturation flag from LAr CaloCell container, not ready yet!
        vtower_SAT.push_back(jTower_sat);  
        
        jTowersContainer->push_back( std::make_unique<xAOD::jFexTower>() );
        jTowersContainer->back()->initialize(eta, phi, iEta, iPhi, IDSimulation, source, vtower_ET, jfex, fpga, channel, tower, vtower_SAT ); 
        
        if( m_doThinning && !jTowersContainer->back()->isCore() ){
            jTowersContainer->pop_back(); 
        }
    }
    
    // Return gracefully
    return StatusCode::SUCCESS;
}


StatusCode jFexEmulatedTowers::ReadFibersfromFile(const std::string & fileName){
    
    
    
    //openning file with ifstream
    std::ifstream file(fileName);
    
    if ( !file.is_open() ){
        ATH_MSG_ERROR("Could not open file:" << fileName);
        return StatusCode::FAILURE;
    }
    
    std::string line;
    //loading the mapping information
    while ( std::getline (file, line) ) {

        //removing the header of the file (it is just information!)
        if(line[0] == '#') continue;
        
        //Splitting line in different substrings
        std::stringstream oneLine(line);
        
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
    file.close();

    return StatusCode::SUCCESS;
}


constexpr unsigned int jFexEmulatedTowers::mapIndex(unsigned int jfex, unsigned int fpga, unsigned int channel, unsigned int tower) {
  // values from hardware: jfex=[0,5] 4 bits, fpga=[0,3] 4 bits, channel=[0,59] 8 bits, tower=[0,15] 4 bits
  return (jfex << 16) | (fpga << 12) | (channel << 4) | tower;
}

StatusCode  jFexEmulatedTowers::ReadSCfromFile(const std::string& fileName){
    
    
    
    //openning file with ifstream
    std::ifstream file(fileName);
    
    if ( !file.is_open() ){
        ATH_MSG_ERROR("Could not open file:" << fileName);
        return StatusCode::FAILURE;
    }
    
    std::string line;
    //loading the mapping information into an unordered_map <Fex Tower ID, vector of SCell IDs>
    while ( std::getline (file, line) ) {
        std::vector<uint64_t> SCellvectorEM;
        SCellvectorEM.clear();
        std::vector<uint64_t> SCellvectorHAD;
        SCellvectorHAD.clear();

        //removing the header of the file (it is just information!)
        if(line[0] == '#') continue;
        
        //Splitting line in different substrings
        std::stringstream oneSCellID(line);
        
        //reading elements
        std::string substr = "";
        int TTID = 0;
        int elem = 0;
        
        while(std::getline(oneSCellID, substr, ' '))
        {
            ++elem;
            if(elem == 1){
               TTID =  std::stoi(substr);
            }
            else{
                //Check if it looks like a SCell Identifier
                if(isBadSCellID(substr)){
                    return StatusCode::FAILURE;
                }
                
                // converts hex number to unsigned long long int
                uint64_t scid_uint64 = std::strtoull(substr.c_str(), nullptr, 0);
                
                //empty slots are filled with 0xffffffffffffffff
                if(scid_uint64 == 0xffffffffffffffff) continue;
                
                //from element from 2 to 13 are EM SCells, element 14 is a HAD SCell
                if(elem<14) SCellvectorEM.push_back(scid_uint64);
                else        SCellvectorHAD.push_back(scid_uint64);          
            }
        }        
        
        m_map_TTower2SCellsEM[TTID] = SCellvectorEM;
        m_map_TTower2SCellsHAD[TTID] = SCellvectorHAD;
        
    }
    file.close();

    return StatusCode::SUCCESS;
}

bool jFexEmulatedTowers::isBadSCellID(const std::string& ID) const{
    
    // does it start with "0x"?, if so then is a GOOD SCell ID!
    if (ID.find("0x") == std::string::npos) {
        ATH_MSG_ERROR("Invalid SuperCell ID " << ID << ". Expecting hexadecimal number on the mapping file");
        return true;
    }
    return false;
}




StatusCode  jFexEmulatedTowers::ReadTilefromFile(const std::string& fileName){
    
    //openning file with ifstream
    std::ifstream file(fileName);
    
    if ( !file.is_open() ){
        ATH_MSG_ERROR("Could not open file:" << fileName);
        return StatusCode::FAILURE;
    }
    
    std::string line;
    //loading the mapping information into an unordered_map <Fex Tower ID, vector of SCell IDs>
    while ( std::getline (file, line) ) {

        //removing the header of the file (it is just information!)
        if(line[0] == '#') continue;
        
        //Splitting line in different substrings
        std::stringstream oneLine(line);
        
        std::vector<std::string> elements;
        std::string element = "";
        
        while(std::getline(oneLine, element, ' ')){
            elements.push_back(element);
        }
        
        if(elements.size() != 4){
            ATH_MSG_ERROR("Invalid number of element in " << line << ". Expecting 4 elements {jFexID, TileID, eta, phi}");
            return StatusCode::FAILURE;
        }
        
        uint32_t jFexID = std::stoi( elements.at(0) );
        uint32_t TileID = std::stoi( elements.at(1) );
        float eta       = std::stof( elements.at(2) );
        float phi       = std::stof( elements.at(3) );
        
        m_map_TTower2Tile[jFexID] = {TileID,eta,phi};
        
    }
    file.close();
    
    return StatusCode::SUCCESS;
}





}
