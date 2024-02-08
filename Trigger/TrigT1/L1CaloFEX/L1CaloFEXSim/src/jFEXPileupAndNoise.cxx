/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration  
*/
//***************************************************************************  
//		jFEXPileupAndNoise - Algorithm for Pileup and Noise in jFEX
//                              -------------------
//     begin                : 24 05 2021
//     email                : Sergi.Rodriguez@cern.ch
//***************************************************************************
#include <iostream>
#include <vector>
#include <stdio.h>
#include <math.h>
#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/StoreGateSvc.h"
#include "L1CaloFEXSim/jFEXPileupAndNoise.h"
#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"

namespace LVL1{

//Default Constructor
LVL1::jFEXPileupAndNoise::jFEXPileupAndNoise(const std::string& type, const std::string& name, const IInterface* parent): AthAlgTool(type, name, parent) {
    declareInterface<IjFEXPileupAndNoise>(this);
}

/** Destructor */
LVL1::jFEXPileupAndNoise::~jFEXPileupAndNoise() {
}

StatusCode LVL1::jFEXPileupAndNoise::initialize() {
    
    ATH_CHECK(m_jTowerContainerKey.initialize());
    ATH_CHECK( m_BDToolKey.initialize() );
    
    
    return StatusCode::SUCCESS;
}

//calls container for TT
StatusCode LVL1::jFEXPileupAndNoise::safetyTest() {
    
    m_jTowerContainer = SG::ReadHandle<jTowerContainer>(m_jTowerContainerKey);
    if(! m_jTowerContainer.isValid()) {
        ATH_MSG_ERROR("Could not retrieve jTowerContainer " << m_jTowerContainerKey.key());
        return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
}

StatusCode LVL1::jFEXPileupAndNoise::reset() {

    m_is_FWD=0;
    m_apply_pileup2jets=0; 
    m_apply_pileup2met=0;
    m_apply_noise2jets=0;
    m_apply_noise2met=0;

    reset_conters();
    
    return StatusCode::SUCCESS;
}

void LVL1::jFEXPileupAndNoise::reset_conters() {
   
    m_rho_EM   = 0;
    m_rho_HAD1 = 0;
    m_rho_HAD2 = 0;
    m_rho_HAD3 = 0;
    m_rho_FCAL = 0;
    
    m_count_rho_EM   = 0;
    m_count_rho_HAD1 = 0;
    m_count_rho_HAD2 = 0;
    m_count_rho_HAD3 = 0;
    m_count_rho_FCAL = 0;    
    
}


//Setup for the central region, duplicate fuction 
void LVL1::jFEXPileupAndNoise::setup(int FPGA[FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_thin_algoSpace_width]) {

    ATH_MSG_DEBUG("---------------- jFEXPileupAndNoise::setup ----------------");
    m_is_FWD = 0; //central region
    m_etaMAX = FEXAlgoSpaceDefs::jFEX_thin_algoSpace_width;
    std::copy(&FPGA[0][0], &FPGA[0][0] + FEXAlgoSpaceDefs::jFEX_algoSpace_height*FEXAlgoSpaceDefs::jFEX_thin_algoSpace_width, &m_FPGA_central[0][0]);
}


//Setup for the forward region, duplicate fuction 
void LVL1::jFEXPileupAndNoise::setup(int FPGA[FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width]) {
    
    ATH_MSG_DEBUG("---------------- jFEXPileupAndNoise::setup ----------------");
    m_is_FWD = 1; //forward region
    m_etaMAX = FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width;
    std::copy(&FPGA[0][0], &FPGA[0][0] + FEXAlgoSpaceDefs::jFEX_algoSpace_height*FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width, &m_FPGA_forward[0][0]);
}


std::vector<int> LVL1::jFEXPileupAndNoise::CalculatePileup(){
    
    reset_conters();
    
    SG::ReadCondHandle<jFEXDBCondData> myDBTool = SG::ReadCondHandle<jFEXDBCondData>(m_BDToolKey/*, ctx*/);
    if(! myDBTool.isValid()) {
        ATH_MSG_ERROR("Could not retrieve DB tool " << m_BDToolKey);
    }    
    
    for(int iphi=0;iphi<FEXAlgoSpaceDefs::jFEX_algoSpace_height;iphi++){
        for(int ieta=0;ieta<m_etaMAX;ieta++){
            
            int TTID = 0;
            
            if(m_is_FWD){
                TTID = m_FPGA_forward[iphi][ieta];
            }
            else{
                TTID = m_FPGA_central[iphi][ieta];
            }
            
            if(TTID == 0){
                continue; //skipping TTID iqual to 0
            } 
            
            const LVL1::jTower *tmpTower = m_jTowerContainer->findTower(TTID);
            
            int tmp_eta = getTTowerEta(tmpTower);
            // energies used below are in MeV, but to reproduce firmware behavior we need to truncate at the same position,
            // i.e., temporarily go back to the granularity used in firmware!
            int em_granularity  = 25;
            int had_granularity = tmp_eta < 15 ? 500 : 25; //For Tile inputs granularity is 500MeV, for LAr 25MeV
            //storing the energies
            int tmp_energy_EM  = getET_EM(tmpTower)/em_granularity;
            int tmp_energy_HAD = getET_HAD(tmpTower)/had_granularity;
            int tmp_EM_AreaINV  = getTTAreaINV_EM(tmpTower);
            int tmp_HD_AreaINV  = getTTAreaINV_HAD(tmpTower);
            m_FPGA_ET_EM[TTID]  = getET_EM(tmpTower); 
            m_FPGA_ET_HAD[TTID] = getET_HAD(tmpTower);
            
            tmp_energy_EM = (tmp_energy_EM * tmp_EM_AreaINV)/(1<<FEXAlgoSpaceDefs::pu_AreaINV);
            tmp_energy_HAD= (tmp_energy_HAD* tmp_HD_AreaINV)/(1<<FEXAlgoSpaceDefs::pu_AreaINV);
            
            //calculating rho's
            
            // EM layer ( not EM FCAL!! )
            
            if(tmp_eta < 32 ){
                if(tmp_energy_EM > myDBTool->get_PUThrLowEm() and tmp_energy_EM < myDBTool->get_PUThrHighEm()) {
                    m_rho_EM += tmp_energy_EM; 
                    m_count_rho_EM++;
                }
            }
            
            // Tile layer
            if(tmp_eta < 15){
                if(tmp_energy_HAD > myDBTool->get_PUThrLowHadTrex() and tmp_energy_HAD < myDBTool->get_PUThrHighHadTrex()){
                    m_rho_HAD1 += tmp_energy_HAD; 
                    m_count_rho_HAD1++;
                }
            }
            // HEC Overlap layer!
            else if(tmp_eta < 16 ){
                if(tmp_energy_HAD > myDBTool->get_PUThrLowHadHecOverlap() and tmp_energy_HAD < myDBTool->get_PUThrHighHadHecOverlap()){
                    m_rho_HAD2 += tmp_energy_HAD; 
                    m_count_rho_HAD2++;
                }
            }
            // Rest of HEC without overlap
            else if(tmp_eta < 32 ){
                if(tmp_energy_HAD > myDBTool->get_PUThrLowHadLar() and tmp_energy_HAD < myDBTool->get_PUThrHighHadLar()){        
                    m_rho_HAD3 += tmp_energy_HAD;
                    m_count_rho_HAD3++;
                }
            }            
            // FCAL is treated here!
            else if(tmp_eta >= 32){
                
                // Contributes the HAD layer (FCAL2 and FCAL3)
                if( TTID >= FEXAlgoSpaceDefs::jFEX_FCAL2_start){
                    if(tmp_energy_HAD > myDBTool->get_PUThrLowFcal() and tmp_energy_HAD < myDBTool->get_PUThrHighFcal()){
                        m_rho_FCAL += tmp_energy_HAD; 
                        m_count_rho_FCAL++;
                    }
                }
                // FCAL1 is EM layer so the energy is suposed to be in the EM layer
                else{
                    if(tmp_energy_EM > myDBTool->get_PUThrLowFcal() and tmp_energy_EM < myDBTool->get_PUThrHighFcal()){
                        m_rho_FCAL += tmp_energy_EM; 
                        m_count_rho_FCAL++;
                    }                    
                }
            }
        }
    }//end of iphi loop
        
    //calculating rho values for each region
    m_rho_EM    = m_count_rho_EM   > 0 ? ((m_rho_EM*rhoDivLUT(m_count_rho_EM)    )/(1<<FEXAlgoSpaceDefs::pu_rhoLUT)) : 0;
    m_rho_HAD1  = m_count_rho_HAD1 > 0 ? ((m_rho_HAD1*rhoDivLUT(m_count_rho_HAD1))/(1<<FEXAlgoSpaceDefs::pu_rhoLUT)) : 0;
    m_rho_HAD2  = m_count_rho_HAD2 > 0 ? ((m_rho_HAD2*rhoDivLUT(m_count_rho_HAD2))/(1<<FEXAlgoSpaceDefs::pu_rhoLUT)) : 0;
    m_rho_HAD3  = m_count_rho_HAD3 > 0 ? ((m_rho_HAD3*rhoDivLUT(m_count_rho_HAD3))/(1<<FEXAlgoSpaceDefs::pu_rhoLUT)) : 0;
    m_rho_FCAL  = m_count_rho_FCAL > 0 ? ((m_rho_FCAL*rhoDivLUT(m_count_rho_FCAL))/(1<<FEXAlgoSpaceDefs::pu_rhoLUT)) : 0;
    
    //return values in MeV -> m_rho_HAD1 corresponds to TileCal, so 500MeV/count, rest is 25MeV/count
    //note: do not convert the internal values yet, this needs to happen after the PU correction is applied
    //      to fully reproduce FW behavior!
    std::vector<int> rho_values {m_rho_EM*25,m_rho_HAD1*500,m_rho_HAD2*25,m_rho_HAD3*25,m_rho_FCAL*25};
    
    SubtractPileup();
    
    return rho_values;
}

int  LVL1::jFEXPileupAndNoise::rhoDivLUT(int ntowers){
    
    //This is to save one bit in the firmware (19 bit will set be set to 1 instead of the 20th bit and rest are 0) 
    if(ntowers == 1) return ((1<<FEXAlgoSpaceDefs::pu_rhoLUT) - 1);
    return static_cast<int>((1.0/ntowers)*(1<<FEXAlgoSpaceDefs::pu_rhoLUT) );
}

void LVL1::jFEXPileupAndNoise::SubtractPileup(){
    
    for(int iphi=0; iphi<FEXAlgoSpaceDefs::jFEX_algoSpace_height; iphi++) {
        for(int ieta=0; ieta<m_etaMAX; ieta++) {

            int TTID = 0;

            if(m_is_FWD) {
                TTID = m_FPGA_forward[iphi][ieta];
            }
            else {
                TTID = m_FPGA_central[iphi][ieta];
            }

            if(TTID == 0) continue; //skipping TTID iqual to 0

            const LVL1::jTower *tmpTower = m_jTowerContainer->findTower(TTID);
            int tmp_eta = getTTowerEta(tmpTower);
            int tmp_EM_Area = getTTArea_EM(tmpTower);
            int tmp_HD_Area = getTTArea_HAD(tmpTower);

            if(tmp_eta < 32) {
                m_FPGA_ET_EM[TTID] =m_FPGA_ET_EM[TTID] - ((m_rho_EM * tmp_EM_Area)/(1<<FEXAlgoSpaceDefs::pu_Area)) * 25; //factor 25: jFEX internal counts -> MeV (LAr)
            }

            if(tmp_eta < 15) {
                m_FPGA_ET_HAD[TTID]=m_FPGA_ET_HAD[TTID] - ((m_rho_HAD1 * tmp_HD_Area)/(1<<FEXAlgoSpaceDefs::pu_Area)) * 500; //factor 500: jFEX internal counts -> MeV (Tile)
            }
            else if(tmp_eta < 16 ) {
                m_FPGA_ET_HAD[TTID]=m_FPGA_ET_HAD[TTID] - ((m_rho_HAD2 * tmp_HD_Area)/(1<<FEXAlgoSpaceDefs::pu_Area)) * 25; //factor 25: jFEX internal counts -> MeV (LAr)
            }
            else if(tmp_eta < 32 ) {
                m_FPGA_ET_HAD[TTID]=m_FPGA_ET_HAD[TTID] - ((m_rho_HAD3 * tmp_HD_Area)/(1<<FEXAlgoSpaceDefs::pu_Area)) * 25; //factor 25: jFEX internal counts -> MeV (LAr)
            }
            else if(tmp_eta >= 32) {
                // Contributes the HAD layer (FCAL2 and FCAL3)
                if( TTID >= FEXAlgoSpaceDefs::jFEX_FCAL2_start){
                    m_FPGA_ET_HAD[TTID] = m_FPGA_ET_HAD[TTID] - ((m_rho_FCAL * (tmp_HD_Area))/(1<<FEXAlgoSpaceDefs::pu_Area)) * 25; //factor 25: jFEX internal counts -> MeV (LAr)
                }
                // FCAL1 is EM layer so the energy is suposed to be in the EM layer
                else{
                    m_FPGA_ET_EM[TTID]  = m_FPGA_ET_EM[TTID] - ((m_rho_FCAL * (tmp_EM_Area))/(1<<FEXAlgoSpaceDefs::pu_Area)) * 25; //factor 25: jFEX internal counts -> MeV (LAr)
                }
            }            
        }
    }
}


//Flags that allow to apply the pileup/noise to the objets
StatusCode LVL1::jFEXPileupAndNoise::ApplyPileup  (){
    SG::ReadCondHandle<jFEXDBCondData>   myDBTool = SG::ReadCondHandle<jFEXDBCondData>(m_BDToolKey);
    if(! myDBTool.isValid()) {
        ATH_MSG_ERROR("Could not retrieve DB tool " << m_BDToolKey);
        return StatusCode::FAILURE;
    }
    
    m_apply_pileup2jets = myDBTool->get_doPileUpJet();
    m_apply_pileup2met = myDBTool->get_doPileUpMet();
    
    return StatusCode::SUCCESS;
}

void LVL1::jFEXPileupAndNoise::ApplyNoise2Jets  (bool b){
    m_apply_noise2jets = b;
}

void LVL1::jFEXPileupAndNoise::ApplyNoise2Met  (bool b){
    m_apply_noise2met = b;
}



std::unordered_map<int,std::vector<int> > LVL1::jFEXPileupAndNoise::Get_EM_Et_values(){
    
    // map for energies sent to the FPGA
    m_map_Etvalues_EM.clear();

    
    for(int iphi=0; iphi<FEXAlgoSpaceDefs::jFEX_algoSpace_height; iphi++) {
        for(int ieta=0; ieta<m_etaMAX; ieta++) {

            int TTID = 0;

            if(m_is_FWD) {
                TTID = m_FPGA_forward[iphi][ieta];
            }
            else {
                TTID = m_FPGA_central[iphi][ieta];
            }

            if(TTID == 0) continue; //skipping TTID iqual to 0    
            const LVL1::jTower *tmpTower = m_jTowerContainer->findTower(TTID);
            
            // tmp variable to fill the map
            std::vector<int> v_energies;
            v_energies.clear();
            v_energies.resize(2,0);
            
            //saving the SG energy
            int tmp_TotalEt_jet=getET_EM(tmpTower);
            int tmp_TotalEt_met=getET_EM(tmpTower);
            
            // if true changing the raw energy to the pileup subtracted energy for jets
            if(m_apply_pileup2jets){
                tmp_TotalEt_jet = m_FPGA_ET_EM[TTID];
            }
            
            // if true changing the raw energy to the pileup subtracted energy for met
            if(m_apply_pileup2met){
                tmp_TotalEt_met = m_FPGA_ET_EM[TTID];
            }
            
            v_energies[0]=tmp_TotalEt_jet;
            v_energies[1]=tmp_TotalEt_met;

            m_map_Etvalues_EM.insert(std::make_pair(TTID, v_energies));    
        }
    }

    if(m_apply_noise2jets || m_apply_noise2met) ApplyNoiseCuts(m_map_Etvalues_EM,0);
    
    return m_map_Etvalues_EM;
}

std::unordered_map<int,std::vector<int> > LVL1::jFEXPileupAndNoise::Get_HAD_Et_values(){
    
    // map for energies sent to the FPGA
    m_map_Etvalues_HAD.clear();

    
    for(int iphi=0; iphi<FEXAlgoSpaceDefs::jFEX_algoSpace_height; iphi++) {
        for(int ieta=0; ieta<m_etaMAX; ieta++) {

            int TTID = 0;

            if(m_is_FWD) {
                TTID = m_FPGA_forward[iphi][ieta];
            }
            else {
                TTID = m_FPGA_central[iphi][ieta];
            }

            if(TTID == 0) continue; //skipping TTID iqual to 0    
            const LVL1::jTower *tmpTower = m_jTowerContainer->findTower(TTID);
            
            // tmp variable to fill the map
            std::vector<int> v_energies;
            v_energies.clear();
            v_energies.resize(2,0);
            
            //saving the SG energy
            int tmp_TotalEt_jet=getET_HAD(tmpTower);
            int tmp_TotalEt_met=getET_HAD(tmpTower);
            
            // if true changing the raw energy to the pileup subtracted energy for jets
            if(m_apply_pileup2jets){
                tmp_TotalEt_jet = m_FPGA_ET_HAD[TTID];
            }
            
            // if true changing the raw energy to the pileup subtracted energy for met
            if(m_apply_pileup2met){
                tmp_TotalEt_met = m_FPGA_ET_HAD[TTID];
            }
            
            v_energies[0]=tmp_TotalEt_jet;
            v_energies[1]=tmp_TotalEt_met;

            m_map_Etvalues_HAD.insert(std::make_pair(TTID, v_energies));
        }
    }

    if(m_apply_noise2jets || m_apply_noise2met) ApplyNoiseCuts(m_map_Etvalues_HAD,1);
    
    return m_map_Etvalues_HAD;
}

void LVL1::jFEXPileupAndNoise::ApplyNoiseCuts(std::unordered_map<int,std::vector<int> > & map_Etvalues,int layer ){
    
    const LVL1::jTower *tmpTower;
    
    for(auto [key,vec] : map_Etvalues){
        
        tmpTower = m_jTowerContainer->findTower(key);
        int Jet_NoiseCut = tmpTower->getNoiseForJet(layer);
        int Met_NoiseCut = tmpTower->getNoiseForMet(layer);
        
        if(m_apply_noise2jets && map_Etvalues[key][0]<=Jet_NoiseCut){ // Et for jets
            map_Etvalues[key][0]=0.;
        }        
        if(m_apply_noise2met && map_Etvalues[key][1]<=Met_NoiseCut){ // Et for Met
            map_Etvalues[key][1]=0.;
        }            

    }
    
}

std::unordered_map<int,std::vector<int> > LVL1::jFEXPileupAndNoise::GetEt_values(){
    
    // map for energies sent to the FPGA
    std::unordered_map<int,std::vector<int> > map_Etvalues;
    map_Etvalues.clear();
    
    
    /* 
     *  The vector Et_energy has size 2
     *  Et_energy[0] is used un the Jet algos
     *  Et_energy[1] is used un the Met/SumEt algos
     */
    std::vector<int> Et_energy;
    
    for(auto [key,vec] : m_map_Etvalues_EM){
        
        Et_energy.clear();
        Et_energy.resize(2,0);
        
        Et_energy[0]=m_map_Etvalues_EM[key][0]+m_map_Etvalues_HAD[key][0];
        Et_energy[1]=m_map_Etvalues_EM[key][1]+m_map_Etvalues_HAD[key][1];
        map_Etvalues[key] = Et_energy;
    }
    return map_Etvalues;
}


//Gets Eta of the TT
int LVL1::jFEXPileupAndNoise::getTTowerEta(const LVL1::jTower *tmpTower) {
    return tmpTower->iEta() < 0 ? std::abs(tmpTower->iEta()+1) : tmpTower->iEta() ;
}
//Gets ET of the TT
int LVL1::jFEXPileupAndNoise::getTTowerET(const LVL1::jTower *tmpTower) {
    return tmpTower->getTotalET();
}
//Gets EM ET of the TT
int LVL1::jFEXPileupAndNoise::getET_EM(const LVL1::jTower *tmpTower) {
    return tmpTower->getET_EM();
}
//Gets HAD ET of the TT
int LVL1::jFEXPileupAndNoise::getET_HAD(const LVL1::jTower *tmpTower) {
    return tmpTower->getET_HAD();
}

//Get Area of a EM TT
int LVL1::jFEXPileupAndNoise::getTTArea_EM(const LVL1::jTower *tmpTower) {
    return tmpTower->getTTowerArea(0);
}

//Get Area of a HAD TT
int LVL1::jFEXPileupAndNoise::getTTArea_HAD(const LVL1::jTower *tmpTower) {
    return tmpTower->getTTowerArea(1);
}

//Get Area of a EM TT
int LVL1::jFEXPileupAndNoise::getTTAreaINV_EM(const LVL1::jTower *tmpTower) {
    return tmpTower->getTTowerAreaInv(0);
}

//Get Area of a HAD TT
int LVL1::jFEXPileupAndNoise::getTTAreaINV_HAD(const LVL1::jTower *tmpTower) {
    return tmpTower->getTTowerAreaInv(1);
}





}// end of namespace LVL1
