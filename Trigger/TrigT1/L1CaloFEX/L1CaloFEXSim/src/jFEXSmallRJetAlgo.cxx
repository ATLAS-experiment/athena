/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration  
*/
//***************************************************************************  
//		jFEXSmallRJetAlgo - Algorithm for small R jet Algorithm in jFEX
//                              -------------------
//     begin                : 03 11 2020
//     email                : varsiha.sothilingam@cern.ch
//***************************************************************************  
#include <vector>
#include "L1CaloFEXSim/jFEXSmallRJetAlgo.h"
#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/jTowerContainer.h"

namespace LVL1{

//Default Constructor
LVL1::jFEXSmallRJetAlgo::jFEXSmallRJetAlgo(const std::string& type, const std::string& name, const IInterface* parent):
   AthAlgTool(type, name, parent)   
  {
  declareInterface<IjFEXSmallRJetAlgo>(this);
  }

/** Destructor */
LVL1::jFEXSmallRJetAlgo::~jFEXSmallRJetAlgo()
{
}
StatusCode LVL1::jFEXSmallRJetAlgo::initialize()
{
   ATH_CHECK(m_jTowerContainerKey.initialize());

   return StatusCode::SUCCESS;

}

//calls container for TT
StatusCode LVL1::jFEXSmallRJetAlgo::safetyTest(){

    m_jTowerContainer = SG::ReadHandle<jTowerContainer>(m_jTowerContainerKey);
    if(! m_jTowerContainer.isValid()) {
        ATH_MSG_ERROR("Could not retrieve  jTowerContainer " << m_jTowerContainerKey.key());
        return StatusCode::FAILURE;
    }
    
    return StatusCode::SUCCESS;
}

void LVL1::jFEXSmallRJetAlgo::setup(int inputTable[7][7], int inputTableDisplaced[7][7]) {
    
    for(int phi=0; phi<7; phi++) {
        for (int eta=0; eta<7; eta++) {
            m_jFEXalgoTowerID[phi][eta] = inputTable[6-phi][eta];
        }
    }

    for(int phi=0; phi<7; phi++) {
        for (int eta=0; eta<7; eta++) {
            m_jFEXalgoTowerID_displaced[phi][eta] = inputTableDisplaced[6-phi][eta];
        }
    }
}

//Gets the ET for the TT. This ET is EM + HAD
unsigned int LVL1::jFEXSmallRJetAlgo::getTTowerET(unsigned int TTID ) const {
    if(TTID == 0) {
        return 0;
    }

    if(m_map_Etvalues.find(TTID) != m_map_Etvalues.end()) {
      return m_map_Etvalues.at(TTID).at(0);
    }

    //we shouldn't arrive here
    return 0;
}

//this function calculates seed for a given TT
void LVL1::jFEXSmallRJetAlgo::buildSeeds()
{

    for(int mphi = 1; mphi < 6; mphi++) {
        for(int meta = 1; meta< 6; meta++) {
            
            int seedTotalET = 0;
            int seedTotalET_displaced = 0;
            for(int iphi = -1; iphi < 2; iphi++) {
                for(int ieta = -1; ieta < 2; ieta++) {
                    //for that TT, build the seed
                    //here we sum TT ET to calculate seed
                    seedTotalET           += getTTowerET(m_jFEXalgoTowerID          [mphi + iphi][meta + ieta]);
                    seedTotalET_displaced += getTTowerET(m_jFEXalgoTowerID_displaced[mphi + iphi][meta + ieta]);
                }
            }
            m_jFEXalgoSearchWindowSeedET[mphi -1][meta -1] = seedTotalET;
            m_jFEXalgoSearchWindowSeedET_displaced[mphi -1][meta -1] = seedTotalET_displaced;
        }
    }
}


bool LVL1::jFEXSmallRJetAlgo::CalculateLM(int mymatrix[5][5]) {

    //here put the 24 conditions to determine if the TT seed is a local maxima.
    int central_seed = mymatrix[2][2];
    for (int iphi = 0; iphi < 5; iphi++) {
        for (int ieta = 0; ieta < 5; ieta++) {
            //avoid comparing central seed to itself
            if ((ieta == 2) && (iphi == 2)) {
                continue;
            }
            //strictly less than central
            if( (iphi > ieta) || (iphi == 0 && ieta == 0) || (iphi == 1 && ieta == 1) ) {
                if(central_seed < mymatrix[iphi][ieta]) {
                    return false;
                }
            }
            //less than or equal to central
            if((iphi < ieta) || (iphi == 3 && ieta == 3) || (iphi == 4 && ieta == 4)) {
                if(central_seed <= mymatrix[iphi][ieta]) {
                    return false;
                }
            }
        }
    }

    return true;
}

//check if central TT is a local maxima
bool LVL1::jFEXSmallRJetAlgo::isSeedLocalMaxima(int seedThreshold) {
    
    bool isCentralLM   = CalculateLM(m_jFEXalgoSearchWindowSeedET) && 
    ( getTTowerET(m_jFEXalgoTowerID[3][3]) >= getTTowerET(m_jFEXalgoTowerID[4][2]) || m_jFEXalgoSearchWindowSeedET[2][2] > m_jFEXalgoSearchWindowSeedET[3][1]);

    bool isDisplacedLM = CalculateLM(m_jFEXalgoSearchWindowSeedET_displaced) && 
    ( getTTowerET(m_jFEXalgoTowerID[3][3]) > getTTowerET(m_jFEXalgoTowerID[2][4]) && m_jFEXalgoSearchWindowSeedET[2][2] == m_jFEXalgoSearchWindowSeedET[1][3]);
    
    if(isCentralLM || isDisplacedLM ){
        calcSaturation();
        return m_jFEXalgoSearchWindowSeedET[2][2] > seedThreshold;
    }
    return false;
}

unsigned int LVL1::jFEXSmallRJetAlgo::getSeedET() const {
  // only valid after calling buildSeeds() !
  return m_jFEXalgoSearchWindowSeedET[2][2];
}


//in this clustering func, the central TT in jet is the parameters
unsigned int LVL1::jFEXSmallRJetAlgo::getSmallClusterET() const {

    int SRJetClusterET = 0;
    for(int nphi = -3; nphi< 4; nphi++) {
        for(int neta = -3; neta< 4; neta++) {
            int DeltaRSquared = std::pow(nphi,2)+std::pow(neta,2);
            if(DeltaRSquared < 16) {
                SRJetClusterET += getTTowerET(m_jFEXalgoTowerID[3+nphi][3+neta]);
            }
        }
    }
    return SRJetClusterET;
}

void LVL1::jFEXSmallRJetAlgo::calcSaturation() {

    m_JetSaturation = false;
    for(int nphi = -3; nphi< 4; nphi++) {
        for(int neta = -3; neta< 4; neta++) {
            int DeltaRSquared = std::pow(nphi,2)+std::pow(neta,2);
            if(DeltaRSquared < 16) {
                m_JetSaturation = m_JetSaturation || getTTowerSat(m_jFEXalgoTowerID[3+nphi][3+neta]);
            }
        }
    }
}

bool LVL1::jFEXSmallRJetAlgo::getSRjetSat() const {
    return m_JetSaturation;
}

unsigned int LVL1::jFEXSmallRJetAlgo::getSmallETRing() const {
  int SmallETRing = getSmallClusterET() - m_jFEXalgoSearchWindowSeedET[3][3];   
  return SmallETRing;
}
         
unsigned int LVL1::jFEXSmallRJetAlgo::getTTIDcentre() const {
  return m_jFEXalgoTowerID[3][3];
}


void LVL1::jFEXSmallRJetAlgo::setFPGAEnergy(const std::unordered_map<int,std::vector<int> >& et_map){
    m_map_Etvalues=et_map;
}

//getter for tower saturation
bool LVL1::jFEXSmallRJetAlgo::getTTowerSat(unsigned int TTID ) {
    if(TTID == 0) {
        return false;
    } 
    
    const LVL1::jTower * tmpTower = m_jTowerContainer->findTower(TTID);
    return tmpTower->getTowerSat();
}

}// end of namespace LVL1
