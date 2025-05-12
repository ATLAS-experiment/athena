/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration  
*/
//***************************************************************************  
//		jFEXtauAlgo - Algorithm for Tau Algorithm in jFEX
//                              -------------------
//     begin                : 18 02 2021
//     email                : Sergi.Rodriguez@cern.ch
//***************************************************************************
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <math.h>
#include "L1CaloFEXSim/jFEXtauAlgo.h"
#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include "PathResolver/PathResolver.h"

namespace LVL1{

//Default Constructor
LVL1::jFEXtauAlgo::jFEXtauAlgo(const std::string& type, const std::string& name, const IInterface* parent): AthAlgTool(type, name, parent) {
    declareInterface<IjFEXtauAlgo>(this);
}

/** Destructor */
LVL1::jFEXtauAlgo::~jFEXtauAlgo() {
}

StatusCode LVL1::jFEXtauAlgo::initialize() {
    ATH_CHECK(m_jTowerContainerKey.initialize());
    
    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_IsoRingStr)  , m_IsoRingMap  ));
    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_SearchGStr)  , m_SearchGMap  ));
    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_SearchGeStr) , m_SearchGeMap )); 
    
    return StatusCode::SUCCESS;
}

//calls container for TT
StatusCode LVL1::jFEXtauAlgo::safetyTest() {

    m_jTowerContainer = SG::ReadHandle<jTowerContainer>(m_jTowerContainerKey);
    if(! m_jTowerContainer.isValid()) {
        ATH_MSG_ERROR("Could not retrieve  jTowerContainer " << m_jTowerContainerKey.key());
        return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
}

void LVL1::jFEXtauAlgo::setup(int seed[3][3]) {

    ATH_MSG_DEBUG(m_color.BLUE<<"---------------- jFEXtauAlgo::setup ----------------"<<m_color.END);
    
    m_TauSaturation = false;
    for(int phi=0; phi<3; phi++) {
        for (int eta=0; eta<3; eta++) {
            m_TTwindow[phi][eta] = seed[2-phi][eta]; // We need to flip the matrix for an increasing phi order: [0, 2Phi]
            m_TauSaturation = m_TauSaturation || getTTowerSat(seed[2-phi][eta]);
        }
    }
    
}

//check if central TT is a local maxima
bool LVL1::jFEXtauAlgo::isSeedLocalMaxima(){
    
    m_ClusterEt = 0;
    int central_seed = getTTowerET(m_TTwindow[1][1]);
    
    for (int iphi = 0; iphi < 3; iphi++) {
        for (int ieta = 0; ieta < 3; ieta++) {
            
            int ttEt = getTTowerET(m_TTwindow[iphi][ieta]);
            m_ClusterEt += ttEt;
            //avoid comparing central seed to itself
            if ((iphi == 1) && (ieta == 1)) {
                continue;
            }
            else if( (iphi > ieta) || (iphi==0 && ieta==0) ) { //less than or equal to central
                if(central_seed<ttEt) {
                    return false;
                }
            }
            else if( (iphi < ieta) || (iphi == 2 && ieta == 2)) { //strictly less than central
                if(central_seed<=ttEt) {
                    return false;
                }
            }
        }
    }
    
    ATH_MSG_DEBUG("Tau Local Maxima found. with ClusterET = "<<m_ClusterEt);
    return true;
}

bool LVL1::jFEXtauAlgo::isSeedLocalMaxima_fwd(unsigned int TTID){
    
    m_TauSaturation = false;
    
    int centreEt = getTTowerET(TTID);
    m_ClusterEt = centreEt;
    m_TauSaturation = m_TauSaturation || getTTowerSat(TTID);
    //centreEt greater than ?
    auto it_map = m_SearchGMap.find(TTID);
    if(it_map == m_SearchGMap.end()) {
         ATH_MSG_ERROR("Could not find TT" << TTID << " in the (greater than) file for Taus.");
    }
    
    for(const auto& lTT : it_map->second){
        int seachTTET = getTTowerET(lTT);
        if(centreEt <= seachTTET ){
            return false;
        }
        m_ClusterEt += seachTTET;
        m_TauSaturation = m_TauSaturation || getTTowerSat(lTT);
    }     
    
    //centreEt greater or equal than ?
    it_map = m_SearchGeMap.find(TTID);
    if(it_map == m_SearchGeMap.end()) {
        ATH_MSG_ERROR("Could not find TT" << TTID << " in the (greater or equal than) file for Taus.");
    }
    
    for(const auto& lTT : it_map->second){
        int seachTTET = getTTowerET(lTT);
        if(centreEt < seachTTET ){
            return false;
        }
        m_ClusterEt += seachTTET;
        m_TauSaturation = m_TauSaturation || getTTowerSat(lTT);
    }     
    
    // If we never returned false above.. we have a local maxima!
    //Calculating now all the Tau iso                
    
    m_TauIsolation = 0;
    it_map = m_IsoRingMap.find(TTID);
    if(it_map == m_IsoRingMap.end()) {
        ATH_MSG_ERROR("Could not find TT" << TTID << " in the isolation file for Taus.");
    }
    
    for(const auto& lTT : it_map->second){
        m_TauIsolation += getTTowerET(lTT);
        m_TauSaturation = m_TauSaturation || getTTowerSat(lTT);
    }   
                         
    return true;
}

//getter for tower saturation
bool LVL1::jFEXtauAlgo::getTTowerSat(unsigned int TTID ) {
    if(TTID == 0) {
        return false;
    } 
    
    const LVL1::jTower * tmpTower = m_jTowerContainer->findTower(TTID);
    return tmpTower->getTowerSat();
}

//Gets the ET for the TT. This ET is EM + HAD
int LVL1::jFEXtauAlgo::getTTowerET(unsigned int TTID ) const {
    if(TTID == 0) {
        return 0;
    } 
    
    auto itr = m_map_Etvalues.find(TTID);
    if( itr == m_map_Etvalues.end()) {
        return 0;
    }
    return (itr->second).at(0);
}

//Gets the seed total ET
int LVL1::jFEXtauAlgo::getClusterEt() const {
    return m_ClusterEt;
}

//Gets the Isolation/FirstEtRing of jFEX Tau

void LVL1::jFEXtauAlgo::setFirstEtRing(int First_ETring[36]) {

    ATH_MSG_DEBUG("Calculating the jFEXTau ISO");
    
    m_TauIsolation=0;
    for(int i=0; i<36; i++) {
        m_TauIsolation += getTTowerET(First_ETring[i]);
        m_TauSaturation = m_TauSaturation || getTTowerSat(First_ETring[i]);
    }
}

int LVL1::jFEXtauAlgo::getFirstEtRing() const {
    return m_TauIsolation;
}

bool LVL1::jFEXtauAlgo::getTauSat() const {
    return m_TauSaturation;
}

void LVL1::jFEXtauAlgo::setFPGAEnergy(const std::unordered_map<int,std::vector<int> >& et_map){
    m_map_Etvalues=et_map;
}


StatusCode LVL1::jFEXtauAlgo::ReadfromFile(const std::string & fileName, std::unordered_map<unsigned int, std::vector<unsigned int> >& fillingMap) const {
    
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
        std::vector<unsigned int> elements;
        std::string element;
        while(std::getline(oneLine, element, ' '))
        {
            elements.push_back(std::stoi(element));
        }
        
        // We should have at least two elements! Central TT and (at least) itself
        if(elements.size() < 1){
            ATH_MSG_ERROR("Unexpected number of elemennts (<1 expected) in file: "<< fileName);
            return StatusCode::FAILURE;
        }
        
        //Central TiggerTower
        unsigned int TTID = elements.at(0);
        
        // rest of TTs that need to be check 
        elements.erase(elements.begin());
        
        fillingMap[TTID] = elements;
        
    }
    myfile.close();

    return StatusCode::SUCCESS;
}



}// end of namespace LVL1
