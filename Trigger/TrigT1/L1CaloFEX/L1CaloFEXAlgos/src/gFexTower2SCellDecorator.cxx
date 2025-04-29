/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           gFexTower2SCellDecorator  -  description
//                              -------------------
//      This reentrant algorithm is meant to decorate the gFEX Towers (input data and simulation) with the corresponding matching set of SuperCell from LAr
//      Information about SCellContainer objetcs are in:
//          - https://gitlab.cern.ch/atlas/athena/-/blob/22.0/Calorimeter/CaloEvent/CaloEvent/CaloCell.h
//      
//     begin                : 27 01 2023
//     email                : cecilia.tosciri@cern.ch
//***************************************************************************/


#include "gFexTower2SCellDecorator.h"
#include "L1CaloFEXSim/gFEXCompression.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdio.h> 

namespace LVL1 {

gFexTower2SCellDecorator::gFexTower2SCellDecorator(const std::string& name, ISvcLocator* svc) : AthReentrantAlgorithm(name, svc){}

StatusCode gFexTower2SCellDecorator::initialize() {
    ATH_MSG_INFO( "L1CaloFEXTools/gFexTower2SCellDecorator::initialize()");
    ATH_CHECK( m_SCellKey.initialize() );
    ATH_CHECK( m_triggerTowerKey.initialize() );
    ATH_CHECK( m_gTowersReadKey.initialize() );
    ATH_CHECK( m_gSCellEtdecorKey.initialize() );
    ATH_CHECK( m_gSCellEtadecorKey.initialize() );
    ATH_CHECK( m_gSCellPhidecorKey.initialize() );
    ATH_CHECK( m_gSCellIDdecorKey.initialize() );
    ATH_CHECK( m_gSCellSampledecorKey.initialize() );
    ATH_CHECK( m_gtowerEtMeVdecorKey.initialize() );
    ATH_CHECK( m_gTowerEtdecorKey.initialize() );
    ATH_CHECK( m_gTileEtMeVdecorKey.initialize() );
    ATH_CHECK( m_gTileEtadecorKey.initialize() );
    ATH_CHECK( m_gTilePhidecorKey.initialize() );
    ATH_CHECK( m_gTileIDdecorKey.initialize() );
    
    
    //Reading from CVMFS Trigger Tower and their corresponding SCell ID
    ATH_CHECK(ReadSCfromFile(PathResolver::find_calib_file(m_gFEX2Scellmapping)));
    ATH_CHECK(ReadTilefromFile(PathResolver::find_calib_file(m_gFEX2Tilemapping)));
    
    return StatusCode::SUCCESS;
}

StatusCode gFexTower2SCellDecorator::execute(const EventContext& ctx) const {
    
    //Reading the Scell container
    SG::ReadHandle<CaloCellContainer> ScellContainer(m_SCellKey, ctx);
    if(!ScellContainer.isValid()) {
        ATH_MSG_FATAL("Could not retrieve collection " << ScellContainer.key() );
        return StatusCode::FAILURE;
    }
    
    // Reading the TriggerTower container
    SG::ReadHandle<xAOD::TriggerTowerContainer> triggerTowerContainer(m_triggerTowerKey, ctx);
    if(!triggerTowerContainer.isValid()) {
        ATH_MSG_FATAL("Could not retrieve collection " << triggerTowerContainer.key() );
        return StatusCode::FAILURE;
    }

    //Reading the gTower container
    SG::ReadHandle<xAOD::gFexTowerContainer> gTowerContainer(m_gTowersReadKey, ctx);
    if(!gTowerContainer.isValid()) {
        ATH_MSG_FATAL("Could not retrieve collection " << gTowerContainer.key() );
        return StatusCode::FAILURE;
    }  
     
    if(ScellContainer->empty() || triggerTowerContainer->empty() || gTowerContainer->empty() ){
        ATH_MSG_WARNING("Nothing to decorate here, at least one container is empty. ScellContainer.size="<<ScellContainer->size() << " or gTowerContainer.size=" << gTowerContainer->size() << " or triggerTowerContainer.size=" << triggerTowerContainer->size() );
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
        
        // keeping just 
        if(std::abs(tower->eta())>1.5 || tower->sampling()!=1) continue;
        map_TileID2ptr[tower->coolId()]=tower;
    }    
        
    //Setup Decorator Handlers
    SG::WriteDecorHandle<xAOD::gFexTowerContainer, std::vector<float> > gTowerSCellEt    (m_gSCellEtdecorKey     , ctx);
    SG::WriteDecorHandle<xAOD::gFexTowerContainer, std::vector<float> > gTowerSCellEta   (m_gSCellEtadecorKey    , ctx);
    SG::WriteDecorHandle<xAOD::gFexTowerContainer, std::vector<float> > gTowerSCellPhi   (m_gSCellPhidecorKey    , ctx);
    SG::WriteDecorHandle<xAOD::gFexTowerContainer, std::vector<int> >   gTowerSCellID    (m_gSCellIDdecorKey     , ctx);
    SG::WriteDecorHandle<xAOD::gFexTowerContainer, std::vector<int> >   gTowerSCellSample(m_gSCellSampledecorKey     , ctx);
    SG::WriteDecorHandle<xAOD::gFexTowerContainer, int >                gTowerEtMeV      (m_gtowerEtMeVdecorKey  , ctx);
    SG::WriteDecorHandle<xAOD::gFexTowerContainer, int >                gTowerSCEtEncoded(m_gTowerEtdecorKey     , ctx);
    SG::WriteDecorHandle<xAOD::gFexTowerContainer, std::vector<int> >   gTowerTileEt     (m_gTileEtMeVdecorKey   , ctx);
    SG::WriteDecorHandle<xAOD::gFexTowerContainer, std::vector<int> >   gTowerTileID     (m_gTileEtMeVdecorKey   , ctx);
    SG::WriteDecorHandle<xAOD::gFexTowerContainer, std::vector<float> > gTowerTileEta    (m_gTileEtadecorKey     , ctx);
    SG::WriteDecorHandle<xAOD::gFexTowerContainer, std::vector<float> > gTowerTilePhi    (m_gTilePhidecorKey     , ctx);

    //looping over the gTowers to decorate them
    for(const xAOD::gFexTower* gTower : *gTowerContainer){
        
        uint32_t gFexID = gTower->gFEXtowerID();
        uint16_t gFexEt = gTower->towerEt();
        uint16_t scSumEtEncoded = 0;

        std::vector<float> scEt;
        std::vector<float> scEta;
        std::vector<float> scPhi;
        std::vector<int>   scID;
        std::vector<int>   scSample;

        std::vector<int>   TileEt;
        std::vector<float> TileEta;
        std::vector<float> TilePhi;
        std::vector<int>   TileID;
              
        //check that the gFEX Tower ID exists in the map
        auto it_TTower2SCells = (m_map_TTower2SCells).find(gFexID);
        if(it_TTower2SCells == (m_map_TTower2SCells).end()) {
            ATH_MSG_ERROR("ID: "<<gFexID<< " not found on map m_map_TTower2SCells");
            return StatusCode::FAILURE;
        }
            
        for (auto const& SCellID : it_TTower2SCells->second ) {
            
            //check that the SCell Identifier exists in the map
            auto it_ScellID2ptr = map_ScellID2ptr.find(SCellID);
            if(it_ScellID2ptr == map_ScellID2ptr.end()) {
                ATH_MSG_WARNING("Scell ID: 0x"<<std::hex<<(SCellID >> 32)<<std::dec<< " not found on map map_ScellID2ptr");
                
                scEt.push_back(-9999);
                scEta.push_back(-99);
                scPhi.push_back(-99);
                // bit shifting to get only a 32 bit number
                scID.push_back( SCellID >> 32 );  
                scSample.push_back(-99);
   
            }

            else{

                const CaloCell* myCell = it_ScellID2ptr->second;                
                float et = myCell->et();
                const CaloSampling::CaloSample sample = (myCell)->caloDDE()->getSampling();
                    

                // The following is to check if any SuperCells from data are permanently masked, and if so the masking is applied 
                int prov = (myCell)->provenance();
                int SCprov = prov&0xFFF;
                bool isMasked = (SCprov&0x80)==0x80;//prov looks like 0000 0000 1000 0000 if the cell is masked
                if (isMasked) et=0;

                scEt.push_back(et);
                scEta.push_back(myCell->eta());
                scPhi.push_back(myCell->phi());
                // bit shifting to get only a 32 bit number
                scID.push_back( SCellID >> 32 ); 
                scSample.push_back(sample);

            }   
        }

        //emulated encoded Et
        float tmpSCellEt = 0;
        for(const auto& tmpet : scEt){
            tmpSCellEt += tmpet;
        }

        scSumEtEncoded = gFEXCompression::compress( tmpSCellEt );

        // Decorating the tower with the corresponding information
        gTowerSCellEt       (*gTower) = std::move(scEt);
        gTowerSCellEta      (*gTower) = std::move(scEta);
        gTowerSCellPhi      (*gTower) = std::move(scPhi);
        gTowerSCellID       (*gTower) = std::move(scID);
        gTowerSCellSample   (*gTower) = std::move(scSample);
        gTowerEtMeV         (*gTower) = gFexEt * 200;
        gTowerSCEtEncoded   (*gTower) = scSumEtEncoded;


        auto it_TTower2Tile = (m_map_TTower2Tile).find(gFexID);
        //check that the gFEX Tower ID exists in the map
        if(it_TTower2Tile == (m_map_TTower2Tile).end()) {
            continue;
        }
        
        for (auto const& TileTowerID : it_TTower2Tile->second ) {

            //check that the Tile Identifier exists in the map
            auto it_TileID2ptr = map_TileID2ptr.find(TileTowerID);
            if(it_TileID2ptr == map_TileID2ptr.end()) {

                ATH_MSG_WARNING("Tile ID: "<<TileID<<std::dec<< " not found on map map_TileID2ptr");

                TileEt.push_back(-9999);
                TileEta.push_back(-99);
                TilePhi.push_back(-99); 
                TileID.push_back(-99); 
            }
            else{

                const xAOD::TriggerTower* tileTower = it_TileID2ptr->second;                
                TileEt.push_back(tileTower->jepET()*1000); //1000 is the Tile energy resolution
                TileEta.push_back(tileTower->eta());
                float phi = tileTower->phi() < M_PI ? tileTower->phi() : tileTower->phi()-2*M_PI;
                TilePhi.push_back(phi);                
                TileID.push_back(TileTowerID); 

            }  


        }         
        
        
        // Decorating the tower with the corresponding information
        gTowerTileEt        (*gTower) = std::move(TileEt);
        gTowerTileID        (*gTower) = std::move(TileID);
        gTowerTileEta       (*gTower) = std::move(TileEta);
        gTowerTilePhi       (*gTower) = std::move(TilePhi);
    }

    // Return gracefully
    return StatusCode::SUCCESS;
}


StatusCode  gFexTower2SCellDecorator::ReadSCfromFile(const std::string& fileName){
    
    std::string myline;   
    //open file with ifstream
    std::ifstream myfile(fileName);
    
    if ( !myfile.is_open() ){
        ATH_MSG_FATAL("Could not open file:" << fileName);
        return StatusCode::FAILURE;
    }
    
    //loading the mapping information into an unordered_map <Fex Tower ID, vector of SCell IDs>
    while ( std::getline (myfile, myline) ) {
        std::vector<uint64_t> SCellvector;
        SCellvector.clear();

        //removing the header of the file (it is just information!)
        myline.erase(myline.begin(), std::find_if(myline.begin(), myline.end(), [](int ch) {
            return !std::isspace(ch);
        })); 
        if(myline[0] == '#') continue;
        
        //Splitting myline in different substrings
        std::stringstream oneSCellID(myline);
        
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
                
                SCellvector.push_back(scid_uint64);
            }
        }        
        
        m_map_TTower2SCells[TTID] = std::move(SCellvector);
        
    }
    myfile.close();

    return StatusCode::SUCCESS;
}

bool gFexTower2SCellDecorator::isBadSCellID(const std::string& ID) const{
    
    // does it start with "0x"?, if so then is a GOOD SCell ID!
    if (ID.find("0x") == std::string::npos) {
        ATH_MSG_ERROR("Invalid SuperCell ID " << ID << ". Expecting hexadecimal number on the mapping file");
        return true;
    }
    return false;
}


StatusCode  gFexTower2SCellDecorator::ReadTilefromFile(const std::string& fileName){
   
    std::string myline;
    
    //openning file with ifstream
    std::ifstream myfile(fileName);
    
    if ( !myfile.is_open() ){
        ATH_MSG_FATAL("Could not open file:" << fileName);
        return StatusCode::FAILURE;
    }
    
    //loading the mapping information into an unordered_map <Fex Tower ID, vector of SCell IDs>
    while ( std::getline (myfile, myline) ) {

        std::vector<uint32_t> Tilevector;
        Tilevector.clear();
        //removing the header of the file 
        myline.erase(myline.begin(), std::find_if(myline.begin(), myline.end(), [](int ch) {
            return !std::isspace(ch);
        }));        
        if(myline[0] == '#') continue;
        
        //Splitting myline in different substrings
        std::stringstream oneTileID(myline);
        
        //reading elements
        std::string substr = "";
        int gTowerID = 0;
        int elem = 0;
        
        while(std::getline(oneTileID, substr, ' ')){
            ++elem;
            if(elem == 1){
               gTowerID =  std::stoi(substr);
            }
            else{         
                uint32_t tileid_uint32 = std::strtoul(substr.c_str(), nullptr, 0);
                Tilevector.push_back(tileid_uint32);
            }
        }
        
        m_map_TTower2Tile[gTowerID] = std::move(Tilevector);
        
    }
    myfile.close();
    
    return StatusCode::SUCCESS;
}





}
