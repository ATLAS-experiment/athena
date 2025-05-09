/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration  
*/
//***************************************************************************  
//		jFEXForwardJetsAlgo - Algorithm for forward Jets in jFEX
//                              -------------------
//     begin                : 07 06 2021
//     email                : Sergi.Rodriguez@cern.ch
//***************************************************************************  
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "L1CaloFEXSim/jFEXForwardJetsAlgo.h"
#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include "L1CaloFEXSim/jFEXForwardJetsInfo.h"
#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"
#include "PathResolver/PathResolver.h"

#include <fstream>

namespace LVL1{

//Default Constructor
LVL1::jFEXForwardJetsAlgo::jFEXForwardJetsAlgo(const std::string& type, const std::string& name, const IInterface* parent):
   AthAlgTool(type, name, parent)   
  {
  declareInterface<IjFEXForwardJetsAlgo>(this);
  }

/** Destructor */
LVL1::jFEXForwardJetsAlgo::~jFEXForwardJetsAlgo()
{
}
StatusCode LVL1::jFEXForwardJetsAlgo::initialize(){
    
    ATH_CHECK(m_jFEXForwardJetsAlgo_jTowerContainerKey.initialize());

    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_SeedRingStr) , m_SeedRingMap ));
    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_1stRingStr)  , m_1stRingMap  ));
    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_2ndRingStr)  , m_2ndRingMap  ));
    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_CorrStr)     , m_CorrMap     ));
    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_Corr2Str)    , m_Corr2Map    ));
    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_SearchGStr)  , m_SearchGMap  ));
    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_SearchGeStr) , m_SearchGeMap ));

    return StatusCode::SUCCESS;

}

//calls container for TT
StatusCode LVL1::jFEXForwardJetsAlgo::safetyTest() {
    m_jTowerContainer = SG::ReadHandle<jTowerContainer>(m_jFEXForwardJetsAlgo_jTowerContainerKey);
    if(! m_jTowerContainer.isValid()) {
        ATH_MSG_ERROR("Could not retrieve jTowerContainer " << m_jFEXForwardJetsAlgo_jTowerContainerKey.key());

        return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
}

StatusCode LVL1::jFEXForwardJetsAlgo::reset() {
    return StatusCode::SUCCESS;
}

void LVL1::jFEXForwardJetsAlgo::setup(int inputTable[FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width], int jfex) {
    std::copy(&inputTable[0][0], &inputTable[0][0] + (FEXAlgoSpaceDefs::jFEX_algoSpace_height*FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width), &m_jFEXalgoTowerID[0][0]);
    m_jfex=jfex;
}

//Gets geometric global centre Eta and Phi coord of the TT
//Has the advantage over the individual eta, phi methods that it does only one tower search
std::array<float,2> LVL1::jFEXForwardJetsAlgo::globalEtaPhi(int TTID) {
    if(TTID == 0) {
        return {999,999};
    }

    const LVL1::jTower *tmpTower = m_jTowerContainer->findTower(TTID);
    return {tmpTower->centreEta(),tmpTower->centrephi_toPI()};
}

int LVL1::jFEXForwardJetsAlgo::getEt(unsigned int TTID) const {
    if(TTID == 0) {
        return -999;
    }
    
    auto itr = m_map_Etvalues.find(TTID);
    
    if( itr == m_map_Etvalues.end()) {
        return -999;
    }    

    return itr->second[0];
}

std::unordered_map<int, jFEXForwardJetsInfo> LVL1::jFEXForwardJetsAlgo::FcalJetsTowerIDLists(int seedThreshold) {
    
    std::unordered_map<int, jFEXForwardJetsInfo> FCALJetTowerIDLists;

    std::vector<int> lower_centre_neta;
    std::vector<int> upper_centre_neta;
    m_lowerEM_eta = 0;
    m_upperEM_eta = 0;

    //STEP 1: check if we are in module 0 or 5 and assign corrrect eta FEXAlgoSpace parameters
    if(m_jfex == 0) {
        //Module 0
        lower_centre_neta.assign({FEXAlgoSpaceDefs::jFEX_algoSpace_C_EMB_start_eta, FEXAlgoSpaceDefs::jFEX_algoSpace_C_EMIE_start_eta, FEXAlgoSpaceDefs::jFEX_algoSpace_C_FCAL_start_eta});
        upper_centre_neta.assign({FEXAlgoSpaceDefs::jFEX_algoSpace_C_EMB_end_eta, FEXAlgoSpaceDefs::jFEX_algoSpace_C_EMIE_end_eta,FEXAlgoSpaceDefs::jFEX_algoSpace_C_FCAL_end_eta });
        m_lowerEM_eta = FEXAlgoSpaceDefs::jFEX_algoSpace_C_lowerEM_eta;
        m_upperEM_eta = FEXAlgoSpaceDefs::jFEX_algoSpace_C_upperEM_eta;
    }
    else {
        //Module 5
        lower_centre_neta.assign({FEXAlgoSpaceDefs::jFEX_algoSpace_A_EMB_eta, FEXAlgoSpaceDefs::jFEX_algoSpace_A_EMIE_eta, FEXAlgoSpaceDefs::jFEX_algoSpace_A_FCAL_start_eta});
        upper_centre_neta.assign({FEXAlgoSpaceDefs::jFEX_algoSpace_A_EMIE_eta, FEXAlgoSpaceDefs::jFEX_algoSpace_A_FCAL_start_eta, FEXAlgoSpaceDefs::jFEX_algoSpace_A_FCAL_end_eta});

        m_lowerEM_eta = FEXAlgoSpaceDefs::jFEX_algoSpace_A_lowerEM_eta;
        m_upperEM_eta = FEXAlgoSpaceDefs::jFEX_algoSpace_A_upperEM_eta;

    }
    
    

    //STEP 2: define phi FEXAlgoSpace parameters
    std::vector<int> lower_centre_nphi{FEXAlgoSpaceDefs::jFEX_algoSpace_EMB_start_phi, FEXAlgoSpaceDefs::jFEX_algoSpace_EMIE_start_phi,  FEXAlgoSpaceDefs::jFEX_algoSpace_FCAL_start_phi};
    std::vector<int> upper_centre_nphi{FEXAlgoSpaceDefs::jFEX_algoSpace_EMB_end_phi, FEXAlgoSpaceDefs::jFEX_algoSpace_EMIE_end_phi,  FEXAlgoSpaceDefs::jFEX_algoSpace_FCAL_end_phi};

    //STEP 3: loop over different EM/FCAL0 eta phi core fpga regions. These are potential central trigger towers for jets
    for(int myCounter = 0; myCounter<3; myCounter++) {
        for(int centre_nphi = lower_centre_nphi[myCounter]; centre_nphi < upper_centre_nphi[myCounter]; centre_nphi++) {
            for(int centre_neta = lower_centre_neta[myCounter]; centre_neta < upper_centre_neta[myCounter]; centre_neta++) {

                //STEP 4: define TTID which will be the key for class in map
                int myTTIDKey = m_jFEXalgoTowerID[centre_nphi][centre_neta];
                
                //STEP 5: ignore when tower ID is zero. Should not happend though
                if(myTTIDKey == 0) {
                    continue;
                }
                bool iAmJet = false;
                bool bool_isLM = false;
                bool bool_condCorr2 = false;
                bool bool_isLMabove = false;
                bool bool_condCorr = false;        
                
                //Know which condition should satisfy
                unsigned int elemCorr  = elementsCorr(myTTIDKey);
                unsigned int elemCorr2 = elementsCorr2(myTTIDKey);

                if(elemCorr == 0 and elemCorr2 == 0){
                    bool_isLM = isLM(myTTIDKey);
                    iAmJet = bool_isLM;
                }
                else if(elemCorr == 0 and elemCorr2 > 0){
                    bool_isLM = isLM(myTTIDKey);
                    bool_condCorr2 = condCorr2(myTTIDKey);
                    iAmJet = bool_isLM and bool_condCorr2;
                }
                else if(elemCorr > 0 and elemCorr2 == 0){
                    bool_isLM = isLM(myTTIDKey);
                    bool_isLMabove = isLMabove(myTTIDKey);
                    bool_condCorr = condCorr(myTTIDKey);
                    iAmJet = bool_isLM or (bool_isLMabove and bool_condCorr);
                }
                else if(elemCorr > 0 and elemCorr2 > 0){
                    bool_isLM = isLM(myTTIDKey);
                    bool_condCorr2 = condCorr2(myTTIDKey);
                    bool_isLMabove = isLMabove(myTTIDKey);
                    bool_condCorr = condCorr(myTTIDKey);
                    iAmJet = (bool_isLM and bool_condCorr2) or (bool_isLMabove and bool_condCorr);                   
                }
                
                //veto seeds below seed threshold
                if (SumEtSeed(myTTIDKey) <= seedThreshold){
                  iAmJet = false;
                }
                
                if(iAmJet){
                    
                    //STEP 6: define class
                    jFEXForwardJetsInfo TriggerTowerInformation;
                    
                    TriggerTowerInformation.setCentreLocalTTPhi(centre_nphi);
                    TriggerTowerInformation.setCentreLocalTTEta(centre_neta);
                    
                    const auto [centreTT_eta,centreTT_phi] = globalEtaPhi(myTTIDKey);
                    TriggerTowerInformation.setCentreTTPhi(centreTT_phi);
                    TriggerTowerInformation.setCentreTTEta(centreTT_eta);  
                    
                    
                    //STEP 7: Filling energies   
                    int saturation = 0;
                    // Seed
                    auto it_seed_map = m_SeedRingMap.find(myTTIDKey);
                    if(it_seed_map == m_SeedRingMap.end()) {
                        ATH_MSG_ERROR("Could not find TT" << myTTIDKey << " in Jet seed file.");
                    }
                    
                    for(const auto& seedTT : it_seed_map->second){
                        TriggerTowerInformation.includeTTinSeed(seedTT);
                        saturation = saturation || getTTowerSat(seedTT);
                        TriggerTowerInformation.addToSeedET(getEt(seedTT));
                    }                    

                    // 1st Energy Ring!
                    it_seed_map = m_1stRingMap.find(myTTIDKey);
                    if(it_seed_map == m_1stRingMap.end()) {
                        ATH_MSG_ERROR("Could not find TT" << myTTIDKey << " in 1st Energy ring file.");
                    }
                    
                    for(const auto& firstER_TT : it_seed_map->second){
                        TriggerTowerInformation.addToFirstEnergyRingET(getEt(firstER_TT));
                        saturation = saturation || getTTowerSat(firstER_TT);
                        if(m_storeEnergyRingTTIDs) {
                            TriggerTowerInformation.includeTTIDinFirstER(firstER_TT);
                        }                        
                    }
                    
                    //setting saturation for jJ
                    if(saturation) TriggerTowerInformation.setSRjetSat();
                    

                    // 2nd Energy Ring!
                    it_seed_map = m_2ndRingMap.find(myTTIDKey);
                    if(it_seed_map == m_2ndRingMap.end()) {
                        ATH_MSG_ERROR("Could not find TT" << myTTIDKey << " in 2nd Energy ring file.");
                    }
                    
                    for(const auto& secondER_TT : it_seed_map->second){
                        TriggerTowerInformation.addToSecondEnergyRingET(getEt(secondER_TT));
                        saturation = saturation || getTTowerSat(secondER_TT);
                        if(m_storeEnergyRingTTIDs) {
                            TriggerTowerInformation.includeTTIDinSecondER(secondER_TT);
                        }
                    }
                    
                    //setting saturation for jLJ
                    if(saturation) TriggerTowerInformation.setLRjetSat();                    

                    // Storing all jets in the same map!
                    FCALJetTowerIDLists[myTTIDKey] = TriggerTowerInformation;                    
                }
                
            }//end of centre_neta loop
        }//end of centre_nphi loop
    }//end of myCounter loop
    
    return FCALJetTowerIDLists;
}

int LVL1::jFEXForwardJetsAlgo::SumEtSeed(unsigned int TTID) const {
    
    // Exists the jTower in the mapping?
    auto it_seed_map = m_SeedRingMap.find(TTID);
    if(it_seed_map == m_SeedRingMap.end()) {
        ATH_MSG_ERROR("Could not find TT" << TTID << " in Jet seed file.");
        return 0;
    }
    int summedEt = 0;
    for(const auto& seedTT : it_seed_map->second){
        summedEt += getEt(seedTT);  
    }

    return summedEt;
}

bool LVL1::jFEXForwardJetsAlgo::isLM(unsigned int TTID) const {
    
    int CentralSeedEt = SumEtSeed(TTID);
    
    // Exists the jTower in the seach (greater than) tower map?
    auto it_seed_map = m_SearchGMap.find(TTID);
    if(it_seed_map == m_SearchGMap.end()) {
        ATH_MSG_ERROR("Could not find TT" << TTID << " in the seach (>) local maxima for jets file.");
        return false;
    }

    bool greater = true;
    for (const auto& Gtt : it_seed_map->second ){
        //checking if the Central seed has strictly more energy than its neighbours
        int tmpEt = SumEtSeed(Gtt);
        if( !(CentralSeedEt > tmpEt) ){
            greater = false;
            break;
        }
    }

    //No need to continue.. Not a LM
    if(!greater){
        return false;
    }

    // Exists the jTower in the seach (greater or equal than) tower map?
    it_seed_map = m_SearchGeMap.find(TTID);
    if(it_seed_map == m_SearchGeMap.end()) {
        ATH_MSG_ERROR("Could not find TT" << TTID << " in the seach (>=) local maxima for jets file.");
        return false;
    }

    bool greaterEqual = true;
    for (const auto& Gtt : it_seed_map->second ){
        //checking if the Central seed has more energy or equal than its neighbours
        int tmpEt = SumEtSeed(Gtt);
        if( !(CentralSeedEt >= tmpEt) ){
            greaterEqual = false;
            break;
        }
    }
    
    //Not a LM
    if(!greaterEqual){
        return false;
    }    
    return true;
}

bool LVL1::jFEXForwardJetsAlgo::isLMabove(unsigned int TTID) const {
    
    // Exists the jTower in the correction tower map?
    auto it_seed_map = m_CorrMap.find(TTID);
    if(it_seed_map == m_CorrMap.end()) {
        ATH_MSG_ERROR("Could not find TT" << TTID << " in the correction (LM above) for jets file.");
        return false;
    }
    
    // If there is not Trigger tower to correct with, then return false
    const auto & GttVec =  it_seed_map->second;
    if (! GttVec.empty()) return isLM(GttVec.front());
    return false;
}

unsigned int LVL1::jFEXForwardJetsAlgo::elementsCorr(unsigned int TTID) const {
    auto it_seed_map = m_CorrMap.find(TTID);
    if(it_seed_map == m_CorrMap.end()) {
        ATH_MSG_ERROR("Could not find TT" << TTID << " in the condition (greater than) for jets file.");
        return 0;
    }    
    
    return (it_seed_map->second).size();
}

bool LVL1::jFEXForwardJetsAlgo::condCorr(unsigned int TTID) const {

    // Exists the jTower in the correction tower map?
    auto it_seed_map = m_CorrMap.find(TTID);
    if(it_seed_map == m_CorrMap.end()) {
        ATH_MSG_ERROR("Could not find TT" << TTID << " in the condition (greater than) for jets file.");
        return false;
    }
    
    // If there is no TT to check then the Et of central is always bigger :D
    if( (it_seed_map->second).size() == 0){
        ATH_MSG_ERROR("Elements=0 in condCorr function for element"<< TTID <<". This should never happend. REPORT IT!");
        return true;
    }
    
    int centralEt = getEt(TTID);
    int centralSeed = SumEtSeed(TTID);
    for (const auto& Gtt : it_seed_map->second ){
        //Checking if central Et is always strictly greater than the previous TT Et 
        int tmpEt = getEt(Gtt);
        int tmpSeedEt = SumEtSeed(Gtt);
        if( !(centralEt > tmpEt && centralSeed >= tmpSeedEt ) ){
            return false;
        }
    }
    
    return true;    
    
}

unsigned int LVL1::jFEXForwardJetsAlgo::elementsCorr2(unsigned int TTID) const {
    auto it_seed_map = m_Corr2Map.find(TTID);
    if(it_seed_map == m_Corr2Map.end()) {
        ATH_MSG_ERROR("Could not find TT" << TTID << " in the condition (greater than) for jets file.");
        return 0;
    }    
    
    return (it_seed_map->second).size();
}

bool LVL1::jFEXForwardJetsAlgo::condCorr2(unsigned int TTID) const {

    // Exists the jTower in the correction tower map?
    auto it_seed_map = m_Corr2Map.find(TTID);
    if(it_seed_map == m_Corr2Map.end()) {
         ATH_MSG_ERROR("Could not find TT" << TTID << " in the correction (greater or equal) file.");
        return false;
    }
    
    int centralEt = getEt(TTID);
    int centralSeed = SumEtSeed(TTID);
    
    for (const auto& Gtt : it_seed_map->second ){
        //Checking if central Et is always greater or equal than the previous TT Et 
        int tmpEt = getEt(Gtt);
        
        //(central >= 1stElem or seedcentral > seed1stElem) and ( central >= 2stElem or seedcentral > seed2stElem)
        if( !( centralEt >= tmpEt || centralSeed > SumEtSeed(Gtt) ) ){
            return false;
        }
    }
    
    return true;    
    
}

std::unordered_map<int, jFEXForwardJetsInfo> LVL1::jFEXForwardJetsAlgo::calculateJetETs(int seedThreshold) {

    std::unordered_map<int, jFEXForwardJetsInfo> localMaximas = FcalJetsTowerIDLists(seedThreshold);
    return localMaximas;
}


void LVL1::jFEXForwardJetsAlgo::setFPGAEnergy(std::unordered_map<int,std::vector<int> > et_map){
    m_map_Etvalues=et_map;
}

//getter for tower saturation
bool LVL1::jFEXForwardJetsAlgo::getTTowerSat(unsigned int TTID ) {
    if(TTID == 0) {
        return false;
    } 
    
    const LVL1::jTower * tmpTower = m_jTowerContainer->findTower(TTID);
    return tmpTower->getTowerSat();
}

StatusCode LVL1::jFEXForwardJetsAlgo::ReadfromFile(const std::string & fileName, std::unordered_map<unsigned int, std::vector<unsigned int> >& fillingMap) const {
    
    std::string myline;
    
    //opening file with ifstream
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
        
        // rest of TTs that need to be checked
        elements.erase(elements.begin());
        
        fillingMap[TTID] = elements;
        
    }
    myfile.close();

    return StatusCode::SUCCESS;
}



}// end of namespace LVL1
