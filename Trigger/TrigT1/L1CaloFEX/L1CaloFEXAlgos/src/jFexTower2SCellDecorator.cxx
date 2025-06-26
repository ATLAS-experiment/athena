/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           jFexTower2SCellDecorator  -  description
//                              -------------------
//      This reentrant algorithm is meant to decorate the FEX Towers (input data and simulation) with the corresponding matching set of SuperCell from LAr
//      Information about SCellContainer objetcs are in:
//          - https://gitlab.cern.ch/atlas/athena/-/blob/22.0/Calorimeter/CaloEvent/CaloEvent/CaloCell.h
//
//     begin                : 01 09 2022
//     email                : sergi.rodriguez@cern.ch
//***************************************************************************/


#include "jFexTower2SCellDecorator.h"
#include "L1CaloFEXSim/jFEXCompression.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <optional>
#include <stdio.h>

namespace {

template <class T>
std::optional<SG::WriteDecorHandle<xAOD::jFexTowerContainer, T> >
makeOptionalKey (const SG::WriteDecorHandleKey<xAOD::jFexTowerContainer>& key,
                 const EventContext& ctx,
                 bool flag)
{
  std::optional<SG::WriteDecorHandle<xAOD::jFexTowerContainer, T> > h;
  if (flag)
    h.emplace (key, ctx);
  return h;
}


}

namespace LVL1 {

jFexTower2SCellDecorator::jFexTower2SCellDecorator(const std::string& name, ISvcLocator* svc) : AthReentrantAlgorithm(name, svc){}

StatusCode jFexTower2SCellDecorator::initialize() {

    ATH_MSG_INFO( "Initializing L1CaloFEXTools/jFexTower2SCellDecorator algorithm with name: "<< name());
    ATH_MSG_INFO( "Decorating SG key: "<< m_jTowersReadKey);

    ATH_CHECK( m_SCellKey.initialize() );
    ATH_CHECK( m_triggerTowerKey.initialize() );
    ATH_CHECK( m_jTowersReadKey.initialize() );

    // This will avoid extra variables when the Key is different than DataTowers
    if( (m_jTowersReadKey.key()).compare("L1_jFexDataTowers") != 0 ){
        m_save_emulated_var = false;
    }

    //Decarator keys
    ATH_CHECK( m_jtowerEtMeVdecorKey.initialize() );
    ATH_CHECK( m_SCellEtMeVdecorKey.initialize() );
    ATH_CHECK( m_TileEtMeVdecorKey.initialize() );
    ATH_CHECK( m_jTowerEtdecorKey.initialize(m_save_emulated_var) );

    ATH_CHECK( m_SCellEtdecorKey.initialize(m_save_extras) );
    ATH_CHECK( m_SCellEtadecorKey.initialize(m_save_extras) );
    ATH_CHECK( m_SCellPhidecorKey.initialize(m_save_extras) );
    ATH_CHECK( m_SCellIDdecorKey.initialize(m_save_extras) );
    ATH_CHECK( m_SCellMaskdecorKey.initialize(m_save_extras) );
    ATH_CHECK( m_TileEtdecorKey.initialize(m_save_extras) );
    ATH_CHECK( m_TileEtadecorKey.initialize(m_save_extras) );
    ATH_CHECK( m_TilePhidecorKey.initialize(m_save_extras) );



    //Reading from CVMFS Trigger Tower and their corresponding SCell ID
    ATH_CHECK(ReadSCfromFile(m_jFEX2Scellmapping));
    ATH_CHECK(ReadTilefromFile(m_jFEX2Tilemapping));

    return StatusCode::SUCCESS;
}

StatusCode jFexTower2SCellDecorator::execute(const EventContext& ctx) const {

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

    //Reading the jTower container
    SG::ReadHandle<xAOD::jFexTowerContainer> jTowerContainer(m_jTowersReadKey, ctx);
    if(!jTowerContainer.isValid()) {
        ATH_MSG_ERROR("Could not retrieve collection " << jTowerContainer.key() );
        return StatusCode::FAILURE;
    }

    if(ScellContainer->empty() || triggerTowerContainer->empty() || jTowerContainer->empty() ){
        ATH_MSG_DEBUG("Nothing to decorate here, at least one container is empty. ScellContainer.size="<<ScellContainer->size() << " or jTowerContainer.size=" << jTowerContainer->size() << " or triggerTowerContainer.size=" << triggerTowerContainer->size() );
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

    // FIXME: In C++20, we could do this more nicely with a templated lambda.
    auto jTowerSCellEt   = makeOptionalKey<std::vector<float> > (m_SCellEtdecorKey, ctx, m_save_extras);
    auto jTowerSCellEta  = makeOptionalKey<std::vector<float> > (m_SCellEtadecorKey, ctx, m_save_extras);
    auto jTowerSCellPhi  = makeOptionalKey<std::vector<float> > (m_SCellPhidecorKey, ctx, m_save_extras);
    auto jTowerSCellID   = makeOptionalKey<std::vector<int> >   (m_SCellIDdecorKey, ctx, m_save_extras);
    auto jTowerSCellMask = makeOptionalKey<std::vector<bool> >  (m_SCellMaskdecorKey, ctx, m_save_extras);
    auto jTowerTileEt    = makeOptionalKey<int>                 (m_TileEtdecorKey, ctx, m_save_extras);
    auto jTowerTileEta   = makeOptionalKey<float>               (m_TileEtadecorKey, ctx, m_save_extras);
    auto jTowerTilePhi   = makeOptionalKey<float>               (m_TilePhidecorKey, ctx, m_save_extras);
    auto jTowerEtencoded = makeOptionalKey<int>                 (m_jTowerEtdecorKey, ctx, m_save_emulated_var);

    SG::WriteDecorHandle<xAOD::jFexTowerContainer, int >   jTowerEtMeV     (m_jtowerEtMeVdecorKey , ctx);
    SG::WriteDecorHandle<xAOD::jFexTowerContainer, float > SCellEtMeV      (m_SCellEtMeVdecorKey  , ctx);
    SG::WriteDecorHandle<xAOD::jFexTowerContainer, float > TileEtMeV       (m_TileEtMeVdecorKey   , ctx);

    //looping over the jTowers to decorate them!
    for(const xAOD::jFexTower* jTower : *jTowerContainer){

        uint32_t jFexID = jTower->jFEXtowerID();
        uint8_t  source = jTower->Calosource();
        int jFexEt = 0;
        uint16_t jFexEtencoded = 0;

        if(source >=7){
            ATH_MSG_WARNING("Undefined source element: "<<source);
        }

        std::vector<float> scEt;
        std::vector<float> scEta;
        std::vector<float> scPhi;
        std::vector<int>   scID;
        std::vector<bool>  scMask;
        float SCellEt = 0.0;
        int   TileEt  = 0;
        float TileEta = -99.0;
        float TilePhi = -99.0;

        if(source != 1){ // Source == 1 is belong to Tile Calorimeter, and of course the is not SCell information!

            const std::unordered_map< uint32_t, std::vector<uint64_t> > * ptr_TTower2Cells;

            //HAD layer for HEC, FCAL2 and FCAL3
            if(source == 3 or source > 4){
                ptr_TTower2Cells = &m_map_TTower2SCellsHAD;
            }
            else{
                ptr_TTower2Cells = &m_map_TTower2SCellsEM;
            }

            //check that the jFEX Tower ID exists in the map
            auto it_TTower2SCells = (*ptr_TTower2Cells).find(jFexID);
            if(it_TTower2SCells == (*ptr_TTower2Cells).end()) {
                ATH_MSG_ERROR("ID: "<<jFexID<< " not found on map m_map_TTower2SCellsEM/HAD");
                return StatusCode::FAILURE;
            }

            bool invalid=true;
            for (auto const& SCellID : it_TTower2SCells->second ) {

                //check that the SCell Identifier exists in the map
                auto it_ScellID2ptr = map_ScellID2ptr.find(SCellID);
                if(it_ScellID2ptr == map_ScellID2ptr.end()) {
                    ATH_MSG_DEBUG("Scell ID: 0x"<<std::hex<<(SCellID >> 32)<<std::dec<< " not found on map map_ScellID2ptr");

                    scEt.push_back(0);
                    scEta.push_back(-99);
                    scPhi.push_back(-99);
                    // bit shifting to get only a 32 bit number
                    scID.push_back( SCellID >> 32 );
                    scMask.push_back(1); // treat missing cells as masked, so that if all missing will get a 0 not an invalid code

                }
                else{
                    const CaloCell* myCell = it_ScellID2ptr->second;

                    int val =  std::round(myCell->energy()/(12.5*std::cosh(myCell->eta()))); // 12.5 is b.c. energy is in units of 12.5MeV per count
                    bool isMasked = m_apply_masking ? ((myCell)->provenance()&0x80) : false;
                    bool isInvalid = m_apply_masking ? ((myCell)->provenance()&0x40) : false;

                    invalid &= isInvalid;
                    //masked &= isMasked;

                    if( isMasked || isInvalid ) {
                        val = 0;
                    }

                    scEt.push_back(val);
                    scEta.push_back(myCell->eta());
                    scPhi.push_back(myCell->phi());
                    // bit shifting to get only a 32 bit number
                    scID.push_back( SCellID >> 32 );
                    scMask.push_back( isMasked );
                }
            }

            //emulated encoded Et
            float tmpSCellEt = 0;
            for(const auto& tmpet : scEt){
                tmpSCellEt += tmpet;
            }

            //How many SCell are masked? if all then send invalid code
            unsigned int count_scMask =0;
            for(const auto& masked : scMask){
                if(masked) count_scMask++;
            }

            SCellEt = tmpSCellEt*12.5;
            // now must convert Total_Et int value into fex value: multi-level encoding
            if(count_scMask==scMask.size()) {
                jFexEtencoded = 0; // no data
            } else if(invalid) {
                jFexEtencoded = 4095; // invalid
            } else {
                jFexEtencoded = jFEXCompression::Compress( tmpSCellEt*12.5, false );
            }

            jFexEt        = jFEXCompression::Expand( jTower->jTowerEt() );
        }
        else if(source == 1){

            //check that the jFEX Tower ID exists in the map
            auto it_TTower2Tile = m_map_TTower2Tile.find(jFexID);
            if(it_TTower2Tile == m_map_TTower2Tile.end()) {
                ATH_MSG_ERROR("ID: "<<jFexID<< " not found on map m_map_TTower2Tile");
                return StatusCode::FAILURE;
            }

            uint32_t TileID = std::get<0>( it_TTower2Tile->second );

            //check that the Tile Identifier exists in the map
            auto it_TileID2ptr = map_TileID2ptr.find(TileID);
            if(it_TileID2ptr == map_TileID2ptr.end()) {
                ATH_MSG_WARNING("Scell ID: 0x"<<std::hex<<TileID<<std::dec<< " not found on map map_TileID2ptr");

                jFexEtencoded = 0;
                TileEt        = 0;
                TileEta       = -99;
                TilePhi       = -99;
            }
            else{
                jFexEtencoded = (it_TileID2ptr->second)->cpET();
                TileEt        = jFexEtencoded*500; // cf 500 since it is cpET
                TileEta       = (it_TileID2ptr->second)->eta();
                float phi     = (it_TileID2ptr->second)->phi() < M_PI ? (it_TileID2ptr->second)->phi() : (it_TileID2ptr->second)->phi()-2*M_PI;
                TilePhi       = phi;

            }

            jFexEt = jTower->jTowerEt()*500; // cf 500 since it is cpET

        }

        // Decorating the tower with the corresponding information
        //Setup Decorator Handlers
        if(m_save_extras) {
            (*jTowerSCellEt)   (*jTower) = std::move(scEt);
            (*jTowerSCellEta)  (*jTower) = std::move(scEta);
            (*jTowerSCellPhi)  (*jTower) = std::move(scPhi);
            (*jTowerSCellID)   (*jTower) = std::move(scID);
            (*jTowerSCellMask) (*jTower) = std::move(scMask);
            (*jTowerTileEt)    (*jTower) = static_cast<int>( TileEt );
            (*jTowerTileEta)   (*jTower) = TileEta;
            (*jTowerTilePhi)   (*jTower) = TilePhi;
        }

        jTowerEtMeV     (*jTower) = jFexEt;
        SCellEtMeV      (*jTower) = SCellEt;
        TileEtMeV       (*jTower) = TileEt;

        if(m_save_emulated_var){
          (*jTowerEtencoded) (*jTower) = jFexEtencoded;
        }


    }

    // Return gracefully
    return StatusCode::SUCCESS;
}


StatusCode  jFexTower2SCellDecorator::ReadSCfromFile(const std::string& fileName){

    std::string myline;

    //openning file with ifstream
    std::ifstream myfile(fileName);

    if ( !myfile.is_open() ){
        ATH_MSG_ERROR("Could not open file:" << fileName);
        return StatusCode::FAILURE;
    }

    //loading the mapping information into an unordered_map <Fex Tower ID, vector of SCell IDs>
    while ( std::getline (myfile, myline) ) {
        std::vector<uint64_t> SCellvectorEM;
        SCellvectorEM.clear();
        std::vector<uint64_t> SCellvectorHAD;
        SCellvectorHAD.clear();

        //removing the header of the file (it is just information!)
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

                //from element from 2 to 13 are EM SCells, element 14 is a HAD SCell
                if(elem<14) SCellvectorEM.push_back(scid_uint64);
                else        SCellvectorHAD.push_back(scid_uint64);
            }
        }

        m_map_TTower2SCellsEM[TTID] = std::move(SCellvectorEM);
        m_map_TTower2SCellsHAD[TTID] = std::move(SCellvectorHAD);

    }
    myfile.close();

    return StatusCode::SUCCESS;
}

bool jFexTower2SCellDecorator::isBadSCellID(const std::string& ID) const{

    // does it start with "0x"?, if so then is a GOOD SCell ID!
    if (ID.find("0x") == std::string::npos) {
        ATH_MSG_ERROR("Invalid SuperCell ID " << ID << ". Expecting hexadecimal number on the mapping file");
        return true;
    }
    return false;
}




StatusCode  jFexTower2SCellDecorator::ReadTilefromFile(const std::string& fileName){

    std::string myline;

    //openning file with ifstream
    std::ifstream myfile(fileName);

    if ( !myfile.is_open() ){
        ATH_MSG_ERROR("Could not open file:" << fileName);
        return StatusCode::FAILURE;
    }

    //loading the mapping information into an unordered_map <Fex Tower ID, vector of SCell IDs>
    while ( std::getline (myfile, myline) ) {

        //removing the header of the file (it is just information!)
        if(myline[0] == '#') continue;

        //Splitting myline in different substrings
        std::stringstream oneLine(myline);

        std::vector<std::string> elements;
        std::string element = "";

        while(std::getline(oneLine, element, ' ')){
            elements.push_back(element);
        }

        if(elements.size() != 4){
            ATH_MSG_ERROR("Invalid number of element in " << myline << ". Expecting 4 elements {jFexID, TileID, eta, phi}");
            return StatusCode::FAILURE;
        }

        uint32_t jFexID = std::stoi( elements.at(0) );
        uint32_t TileID = std::stoi( elements.at(1) );
        float eta       = std::stof( elements.at(2) );
        float phi       = std::stof( elements.at(3) );

        m_map_TTower2Tile[jFexID] = {TileID,eta,phi};

    }
    myfile.close();

    return StatusCode::SUCCESS;
}





}
