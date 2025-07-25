/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//              jFEXSmallRJetAlgo - Algorithm for small R jet Algorithm in jFEX
//                              -------------------
//     begin                : 03 11 2020
//     email                : varsiha.sothilingam@cern.ch
//***************************************************************************

#include <vector>
#include "jFEXLargeRJetAlgo.h"
#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/jTowerContainer.h"

namespace LVL1{

LVL1::jFEXLargeRJetAlgo::jFEXLargeRJetAlgo(const std::string& type, const std::string& name, const IInterface* parent):
   AthAlgTool(type, name, parent)   
  {
  declareInterface<IjFEXLargeRJetAlgo>(this);
  }

/** Destructor */
LVL1::jFEXLargeRJetAlgo::~jFEXLargeRJetAlgo()
{
}
StatusCode LVL1::jFEXLargeRJetAlgo::initialize()
{
   ATH_CHECK(m_jTowerContainerKey.initialize());

   return StatusCode::SUCCESS;

}

StatusCode LVL1::jFEXLargeRJetAlgo::safetyTest() {
    m_jTowerContainer = SG::ReadHandle<jTowerContainer>(m_jTowerContainerKey);
    if(! m_jTowerContainer.isValid()) {
        ATH_MSG_ERROR("Could not retrieve jTowerContainer " << m_jTowerContainerKey.key());

        return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
}

void LVL1::jFEXLargeRJetAlgo::setupCluster(int inputTable[15][15]){

  std::copy(&inputTable[0][0], &inputTable[0][0] + 225 , &m_largeRJetEtRing_IDs[0][0]);

}

unsigned int LVL1::jFEXLargeRJetAlgo::getRingET() {
    int RingET =0;
    for(int n =0; n <15; n++) {
        for(int m =0; m <15; m++) {
            int et = getTTowerET(m_largeRJetEtRing_IDs[n][m]);
            RingET +=et;
        }
    }
    return RingET;
}

bool LVL1::jFEXLargeRJetAlgo::getLRjetSat() {
    m_saturation = false;
    for(int n =0; n <15; n++) {
        for(int m =0; m <15; m++) {
            m_saturation = m_saturation || getTTowerSat(m_largeRJetEtRing_IDs[n][m]);
        }
    }
    return m_saturation;
}

unsigned int LVL1::jFEXLargeRJetAlgo::getLargeClusterET(unsigned int smallClusterET, unsigned int largeRingET) const {
  int largeClusterET = smallClusterET + largeRingET;
  return largeClusterET; 
}


//Gets the ET for the TT. This ET is EM + HAD
int LVL1::jFEXLargeRJetAlgo::getTTowerET(unsigned int TTID ) {
    if(TTID == 0) {
        return 0;
    } 
    
    if(m_map_Etvalues.find(TTID) != m_map_Etvalues.end()) {
        return m_map_Etvalues[TTID][0];
    }
    
    //we shouldn't arrive here
    return 0;
    
}

//getter for tower saturation
bool LVL1::jFEXLargeRJetAlgo::getTTowerSat(unsigned int TTID ) {
    if(TTID == 0) {
        return false;
    } 
    
    const LVL1::jTower * tmpTower = m_jTowerContainer->findTower(TTID);
    return tmpTower->getTowerSat();
}


void LVL1::jFEXLargeRJetAlgo::setFPGAEnergy(const std::unordered_map<int,std::vector<int> >& et_map){
    m_map_Etvalues=et_map;
}

}// end of namespace LVL1



