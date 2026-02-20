/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration  
*/
//***************************************************************************  
//		jFEXsumETAlgo - Algorithm for Sum ET Algorithm in jFEX
//                              -------------------
//     begin                : 12 05 2021
//     email                : Sergi.Rodriguez@cern.ch
//***************************************************************************
#include <iostream>
#include <vector>
#include <stdio.h>
#include <math.h>
#include "jFEXsumETAlgo.h"
#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"

namespace LVL1{

//Default Constructor
LVL1::jFEXsumETAlgo::jFEXsumETAlgo(const std::string& type, const std::string& name, const IInterface* parent): AthAlgTool(type, name, parent) {
    declareInterface<IjFEXsumETAlgo>(this);
}

/** Destructor */
LVL1::jFEXsumETAlgo::~jFEXsumETAlgo() {
}

StatusCode LVL1::jFEXsumETAlgo::initialize() {
    ATH_CHECK(m_jTowerContainerKey.initialize());
    return StatusCode::SUCCESS;
}

//calls container for TT
StatusCode LVL1::jFEXsumETAlgo::safetyTest() {

    m_jTowerContainer = SG::ReadHandle<jTowerContainer>(m_jTowerContainerKey);
    if(! m_jTowerContainer.isValid()) {
        ATH_MSG_ERROR("Could not retrieve  jTowerContainer " << m_jTowerContainerKey.key());
        return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
}

StatusCode LVL1::jFEXsumETAlgo::reset() {
    m_FPGA.clear();
    m_FPGA_phi02.clear();
    m_FPGA_fcal.clear();
    return StatusCode::SUCCESS;
}

//
void LVL1::jFEXsumETAlgo::setup(int FPGA[FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_thin_algoSpace_width]) {

    ATH_MSG_DEBUG("---------------- jFEXsumETAlgo::setup ----------------");
    m_FPGA.resize(FEXAlgoSpaceDefs::jFEX_algoSpace_height);
    for(int iphi=0;iphi<FEXAlgoSpaceDefs::jFEX_algoSpace_height;iphi++){
        for(int ieta=8;ieta<16;ieta++){
            m_FPGA[iphi].push_back(FPGA[iphi][ieta]);
        }
    }
}

void LVL1::jFEXsumETAlgo::setup(int FPGA[FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width]) {

    ATH_MSG_DEBUG("---------------- jFEXsumETAlgo::setup ----------------");
    m_FPGA.resize(FEXAlgoSpaceDefs::jFEX_algoSpace_height);
    for(int iphi=0;iphi<FEXAlgoSpaceDefs::jFEX_algoSpace_height;iphi++){
        for(int ieta=8;ieta<17;ieta++){
            m_FPGA[iphi].push_back(FPGA[iphi][ieta]);
        }
    }
    m_FPGA_phi02.resize(16);
    for(int iphi=0;iphi<16;iphi++){
        for(int ieta=17;ieta<21;ieta++){
            m_FPGA_phi02[iphi].push_back(FPGA[iphi][ieta]);
        }
    }    
    m_FPGA_fcal.resize(8);
    for(int iphi=0;iphi<8;iphi++){
        for(int ieta=21;ieta<FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width;ieta++){
            m_FPGA_fcal[iphi].push_back(FPGA[iphi][ieta]);
        }
    }    
    
}

//this function calculates SumET for the barrel region
void LVL1::jFEXsumETAlgo::buildBarrelSumET()
{
    ATH_MSG_DEBUG("---------------- jFEXsumETAlgo::buildBarrelSumET ----------------");
    
    m_SumET.clear();
    m_SumET.resize(8,0);
    m_SumETSat.clear();
    m_SumETSat.resize(8,0);
    
    for(uint iphi=0;iphi<m_FPGA.size();iphi++){
        for(uint ieta=0;ieta<m_FPGA[iphi].size();ieta++){
            m_SumET[ieta]+=getTTowerET(m_FPGA[iphi][ieta]);
            m_SumETSat[ieta] = m_SumETSat[ieta] || getTTowerSat(m_FPGA[iphi][ieta]);
        }
    }

}

//this function calculates SumET for the forward region
void LVL1::jFEXsumETAlgo::buildFWDSumET()
{
    ATH_MSG_DEBUG("---------------- jFEXsumETAlgo::buildFWDSumET ----------------");
    
    m_SumET.clear();
    m_SumET.resize(14,0);
    m_SumETSat.clear();
    m_SumETSat.resize(14,0);
    
    // ECal with resolution of 0.1x0.1
    for(uint iphi=0;iphi<m_FPGA.size();iphi++){
        for(uint ieta=0;ieta<m_FPGA[iphi].size();ieta++){
            m_SumET[ieta]+=getTTowerET(m_FPGA[iphi][ieta]);
            m_SumETSat.at(ieta) = m_SumETSat.at(ieta) || getTTowerSat(m_FPGA[iphi][ieta]);
        }
    }

    // ECal with resolution of 0.2x0.2
    for(uint iphi=0;iphi<m_FPGA_phi02.size();iphi++){
        for(uint ieta=0;ieta<m_FPGA_phi02[iphi].size();ieta++){
            m_SumET[ieta+9]+=getTTowerET(m_FPGA_phi02[iphi][ieta]);
            m_SumETSat.at(ieta+9) = m_SumETSat.at(ieta+9) || getTTowerSat(m_FPGA_phi02[iphi][ieta]);
        }
    }    
    
    // Fcal
    for(uint iphi=0;iphi<m_FPGA_fcal.size();iphi++){
        for(uint ieta=0;ieta<m_FPGA_fcal[iphi].size();ieta++){
            m_SumET[13]+=getTTowerET(m_FPGA_fcal[iphi][ieta]); //All FCAL is considered one slice due to the non-aligment of the FCal layers
            m_SumETSat.at(13) = m_SumETSat.at(13) || getTTowerSat(m_FPGA_fcal[iphi][ieta]);
        }
    }       

}

std::tuple<int, bool> LVL1::jFEXsumETAlgo::getETlowerEta(uint bin)
{
    uint max = m_SumET.size() > (bin + 1) ? bin : m_SumET.size();
    m_SumlowEta = 0;
    m_SumlowEtaSat = 0;
    for(uint ieta=0;ieta<max;ieta++){
        m_SumlowEta+=m_SumET.at(ieta);
        m_SumlowEtaSat = m_SumlowEtaSat || m_SumETSat.at(ieta);
    }
    return {m_SumlowEta,m_SumlowEtaSat};
}



std::tuple<int, bool> LVL1::jFEXsumETAlgo::getETupperEta(uint bin)
{
    uint min = m_SumET.size() > (bin + 1) ? bin : m_SumET.size();
    m_SumhighEta = 0;
    m_SumhighEtaSat = 0;
    for(uint ieta=min;ieta<m_SumET.size();ieta++){
        m_SumhighEta+=m_SumET.at(ieta);
        m_SumhighEtaSat = m_SumhighEtaSat || m_SumETSat.at(ieta);
    }
    return {m_SumhighEta,m_SumhighEtaSat};
}


//getter for tower saturation
bool LVL1::jFEXsumETAlgo::getTTowerSat(unsigned int TTID ) {
    if(TTID == 0) {
        return false;
    } 
    
    const LVL1::jTower * tmpTower = m_jTowerContainer->findTower(TTID);
    return tmpTower->getTowerSat();
}

//Gets the ET for the TT. This ET is EM + HAD
int LVL1::jFEXsumETAlgo::getTTowerET(unsigned int TTID ) {
    if(TTID == 0) {
        return 0;
    } 
    
    auto itr = m_map_Etvalues.find(TTID);
    if( itr == m_map_Etvalues.end()) {
        return 0;
    }
    
    return (itr->second).at(1);
}


void LVL1::jFEXsumETAlgo::setFPGAEnergy(std::unordered_map<int,std::vector<int> > et_map){
    m_map_Etvalues=std::move(et_map);
}


}// end of namespace LVL1
