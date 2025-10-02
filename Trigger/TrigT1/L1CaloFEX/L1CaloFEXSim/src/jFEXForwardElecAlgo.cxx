/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//		jFEXForwardElecAlgo - Algorithm for Forward Electron Algorithm in jFEX
//                              -------------------
//     begin                : 16 11 2021
//     email                : Sergi.Rodriguez@cern.ch
//     email                : ulla.blumenschein@cern.ch
//     email                : sjolin@cern.ch
//***************************************************************************

#include <iostream>
#include <vector>
#include <stdio.h>
#include <math.h>
#include <fstream>
#include "jFEXForwardElecAlgo.h"
#include "L1CaloFEXSim/jFEXForwardElecTOB.h"
#include "L1CaloFEXSim/jFEXForwardElecInfo.h"
#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include "PathResolver/PathResolver.h"

namespace LVL1 {

  //Default Constructor
  LVL1::jFEXForwardElecAlgo::jFEXForwardElecAlgo(const std::string& type, const std::string& name, const IInterface* parent): AthAlgTool(type, name, parent) {
    declareInterface<IjFEXForwardElecAlgo>(this);
  }
  
  /** Destructor */
  LVL1::jFEXForwardElecAlgo::~jFEXForwardElecAlgo() {
  }
  
  StatusCode LVL1::jFEXForwardElecAlgo::initialize() {
    ATH_CHECK(m_jTowerContainerKey.initialize());

    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_IsoMapStr), m_IsoMap ));
    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_Frac1MapStr), m_Frac1Map ));
    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_Frac2MapStr), m_Frac2Map ));
    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_SearchGTauStr), m_SearchGTauMap));
    ATH_CHECK(ReadfromFile(PathResolver::find_calib_file(m_SearchGeTauStr), m_SearchGeTauMap));

    return StatusCode::SUCCESS;
  }
  
  //calls container for TT
  StatusCode LVL1::jFEXForwardElecAlgo::safetyTest() {
    m_jTowerContainer = SG::ReadHandle<jTowerContainer>(m_jTowerContainerKey);
    if(! m_jTowerContainer.isValid()) {
      ATH_MSG_ERROR("Could not retrieve jTowerContainer " << m_jTowerContainerKey.key());
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }

  StatusCode LVL1::jFEXForwardElecAlgo::reset() {
    return StatusCode::SUCCESS;
  }
    
  void LVL1::jFEXForwardElecAlgo::setup(int inputTable[FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width], int jfex, int fpga) {
    std::copy(&inputTable[0][0], &inputTable[0][0] + (FEXAlgoSpaceDefs::jFEX_algoSpace_height*FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width), &m_jFEXalgoTowerID[0][0]);
    m_jfex=jfex;
    m_fpga=fpga;
  }

  //global centre Eta and Phi coord of the TT
  std::array<float,2> LVL1::jFEXForwardElecAlgo::getEtaPhi(uint ttID) {
    if(ttID == 0) {
      return {999,999};
    }
    const LVL1::jTower *tmpTower = m_jTowerContainer->findTower(ttID);
    return {tmpTower->centreEta(),tmpTower->centrePhi()};
  }

  std::array<int,2> LVL1::jFEXForwardElecAlgo::getEtEmHad(uint ttID) const {
    if(ttID == 0) {
      return {0,0};
    }
    int TT_EtEM = 0;
    if(m_map_Etvalues_EM.find(ttID) != m_map_Etvalues_EM.end()) {
      TT_EtEM = m_map_Etvalues_EM.at(ttID)[0];
    }
    int TT_EtHad = 0;
    if(m_map_Etvalues_HAD.find(ttID) != m_map_Etvalues_HAD.end()) {
      TT_EtHad = m_map_Etvalues_HAD.at(ttID)[0];
    }

    return {TT_EtEM, TT_EtHad};
  }

  bool LVL1::jFEXForwardElecAlgo::getEMSat(unsigned int ttID ) {
    if(ttID == 0) {
        return false;
    } 
    
    const LVL1::jTower * tmpTower = m_jTowerContainer->findTower(ttID);
    return tmpTower->getEMSat();
  }

  void LVL1::jFEXForwardElecAlgo::setFPGAEnergy(
    std::unordered_map<int,std::vector<int> > etmapEM,
    std::unordered_map<int,std::vector<int> > etmapHAD) {
    m_map_Etvalues_EM=etmapEM; 
    m_map_Etvalues_HAD=etmapHAD;
  }
  
  
  bool LVL1::jFEXForwardElecAlgo::isValidSeed(uint seedTTID) const {
    auto [centreTT_EtEM,centreTT_EtHad] = getEtEmHad(seedTTID);
    // check if seed has strictly more energy than its neighbours
    {
      auto it_seed_map = m_SearchGTauMap.find(seedTTID);
      if(it_seed_map == m_SearchGTauMap.end()) {
          ATH_MSG_ERROR("Could not find TT" << seedTTID << " in the seach (>) local maxima for tau/em file.");
          return false;
      }
      for (const auto& gtt : it_seed_map->second ){
        auto [tmp_EtEM,tmp_EtHad] = getEtEmHad(gtt);
        if(tmp_EtEM>=centreTT_EtEM) {
          return false;
        }
      }
    }

    // check if seed has equal or more energy than its neighbours
    {
      auto it_seed_map = m_SearchGeTauMap.find(seedTTID);
      if(it_seed_map == m_SearchGeTauMap.end()) {
          ATH_MSG_ERROR("Could not find TT" << seedTTID << " in the seach (>=) local maxima for tau/em file.");
          return false;
      }
      for (const auto& gtt : it_seed_map->second ){
        auto [tmp_EtEM,tmp_EtHad] = getEtEmHad(gtt);
        if( tmp_EtEM>centreTT_EtEM) {
          return false;
        }
      }
    }

    return true;  
  }
  

  void LVL1::jFEXForwardElecAlgo::findAndFillNextTT(jFEXForwardElecInfo& elCluster, int neta, int nphi) {
    //determine direction for offsets (-1 : C-side, +1: A-side)
    int direction = m_jfex < 3 ? -1 : +1;
    
    std::vector<std::pair<int,int>> neighbours;
    //eta/phi index offsets depend on the position of the seed the cluster is created from
    //differentiate the (many) different cases:
    if ( direction>0 ? neta < FEXAlgoSpaceDefs::jFEX_algoSpace_A_EMIE_eta - 1    // A-side condition
                     : neta > FEXAlgoSpaceDefs::jFEX_algoSpace_C_EMB_start_eta)  // C-side condition
    { //Barrel
      neighbours = { {neta  , nphi+1},
                     {neta  , nphi-1},
                     {neta+1, nphi  },
                     {neta-1, nphi  }
                   };
    } else if ( direction>0 ? neta == FEXAlgoSpaceDefs::jFEX_algoSpace_A_EMIE_eta - 1
                            : neta == FEXAlgoSpaceDefs::jFEX_algoSpace_C_EMB_start_eta ) 
    { //Barrel next to Endcap
      neighbours = { {neta, nphi+1},
                     {neta, nphi-1},
                     {neta-direction, nphi},
                     {neta+direction, nphi/2} //boundary crossing into endcap -> reduced phi granularity
                   };
    } else if ( direction>0 ? neta == FEXAlgoSpaceDefs::jFEX_algoSpace_A_EMIE_eta 
                            : neta == FEXAlgoSpaceDefs::jFEX_algoSpace_C_EMB_start_eta - 1 ) 
    { //Endcap next to Barrel
      neighbours = { {neta, nphi+1},
                     {neta, nphi-1},
                     {neta+direction, nphi},
                     {neta-direction, 2*nphi+0}, //crossing into barrel region, higher phi granularity
                     {neta-direction, 2*nphi+1}, // -> consider both "touching" towers 
                   };
    } else if ( direction>0 ? neta > FEXAlgoSpaceDefs::jFEX_algoSpace_A_EMIE_eta         && neta < FEXAlgoSpaceDefs::jFEX_algoSpace_A_FCAL_start_eta - 1
                            : neta < FEXAlgoSpaceDefs::jFEX_algoSpace_C_EMIE_end_eta - 1 && neta > FEXAlgoSpaceDefs::jFEX_algoSpace_C_EMIE_start_eta ) 
    { //Endcap
      neighbours = { {neta  , nphi+1},
                     {neta  , nphi-1},
                     {neta+1, nphi  },
                     {neta-1, nphi  }
                   };
    } else if ( direction>0 ? neta == FEXAlgoSpaceDefs::jFEX_algoSpace_A_FCAL_start_eta - 1
                            : neta == FEXAlgoSpaceDefs::jFEX_algoSpace_C_EMIE_start_eta ) 
    { //Endcap next to FCal
      neighbours = { {neta, nphi+1},
                     {neta, nphi-1},
                     {neta-direction, nphi},
                     {neta+2*direction, nphi/2} //boundary crossing into FCal -> reduced phi granularity and skip first FCal bin
                   };
    } else if ( direction>0 ? neta == FEXAlgoSpaceDefs::jFEX_algoSpace_A_FCAL_start_eta + 1   //first FCal bin must be skipped!
                            : neta == FEXAlgoSpaceDefs::jFEX_algoSpace_C_EMIE_start_eta - 2 ) //first FCal bin must be skipped!
    { //FCal next to Endcap
      //phi spacing in FCal is very wide, no longer consider adding towers in phi direction
      neighbours = { {neta-2*direction, 2*nphi+0}, //boundary crossing into endcap, higher phi granularity
                     {neta-2*direction, 2*nphi+1}, // -> consider both "touching" towers
                     {neta+direction, nphi}
                   };
    } else if ( direction>0 ? neta > FEXAlgoSpaceDefs::jFEX_algoSpace_A_FCAL_start_eta + 1 && neta < FEXAlgoSpaceDefs::jFEX_algoSpace_A_FCAL_end_eta - 1
                            : neta < FEXAlgoSpaceDefs::jFEX_algoSpace_C_FCAL_end_eta - 2   && neta > FEXAlgoSpaceDefs::jFEX_algoSpace_C_FCAL_start_eta ) 
    { //FCal
      neighbours = { {neta+1, nphi},
                     {neta-1, nphi}
                   };
    } else if ( direction>0 ? neta == FEXAlgoSpaceDefs::jFEX_algoSpace_A_FCAL_end_eta - 1 
                            : neta == FEXAlgoSpaceDefs::jFEX_algoSpace_C_FCAL_start_eta  ) 
    { //FCal, last eta bin
      neighbours = { {neta-direction, nphi}
                   };    
    } else {
      ATH_MSG_ERROR("Eta index " << neta << " (side: "<< (direction>0?"A":"C") << ") does not seem to belong to any valid seed region");
    }
    
    //iterate over neighbours, find most energetic one
    for (const auto& candEtaPhi: neighbours) {
      uint candID = m_jFEXalgoTowerID[candEtaPhi.second][candEtaPhi.first];
      const auto [candTT_EtEM, candTT_EtHad] = getEtEmHad(candID);
      if (candTT_EtEM > elCluster.getNextTTEtEM()) {
        elCluster.setNextTTEtEM(candTT_EtEM);
        elCluster.setNextTTID(candID);
        elCluster.setNextTTSatEM(getEMSat(candID));
      }
    }
    
  }

  
  std::unordered_map<uint, LVL1::jFEXForwardElecInfo> LVL1::jFEXForwardElecAlgo::calculateEDM(void) {
    std::unordered_map<uint, LVL1::jFEXForwardElecInfo> clusterList;
    std::vector<int> lower_centre_neta;
    std::vector<int> upper_centre_neta;
   
    //check if we are in module 0 or 5 and assign corrrect eta FEXAlgoSpace parameters
    if(m_jfex == 0) {
      //Module 0 
      lower_centre_neta.assign({FEXAlgoSpaceDefs::jFEX_algoSpace_C_EMB_start_eta, // 28
                                FEXAlgoSpaceDefs::jFEX_algoSpace_C_EMIE_start_eta, // 24
                                FEXAlgoSpaceDefs::jFEX_algoSpace_C_FCAL_start_eta}); // 12
      
      upper_centre_neta.assign({FEXAlgoSpaceDefs::jFEX_algoSpace_C_EMB_end_eta, // 37
                                FEXAlgoSpaceDefs::jFEX_algoSpace_C_EMIE_end_eta, // 28
                                FEXAlgoSpaceDefs::jFEX_algoSpace_C_FCAL_end_eta }); // 24
    }
    else {
      //Module 5
      lower_centre_neta.assign({FEXAlgoSpaceDefs::jFEX_algoSpace_A_EMB_eta, // 8
                                FEXAlgoSpaceDefs::jFEX_algoSpace_A_EMIE_eta, // 17
                                FEXAlgoSpaceDefs::jFEX_algoSpace_A_FCAL_start_eta}); // 21
                                
      upper_centre_neta.assign({FEXAlgoSpaceDefs::jFEX_algoSpace_A_EMIE_eta, // 17
                                FEXAlgoSpaceDefs::jFEX_algoSpace_A_FCAL_start_eta, // 21 
                                FEXAlgoSpaceDefs::jFEX_algoSpace_A_FCAL_end_eta}); // 33
    }

    //define phi FEXAlgoSpace parameters
    std::vector<int> lower_centre_nphi{FEXAlgoSpaceDefs::jFEX_algoSpace_EMB_start_phi, FEXAlgoSpaceDefs::jFEX_algoSpace_EMIE_start_phi,  FEXAlgoSpaceDefs::jFEX_algoSpace_FCAL_start_phi};
    std::vector<int> upper_centre_nphi{FEXAlgoSpaceDefs::jFEX_algoSpace_EMB_end_phi, FEXAlgoSpaceDefs::jFEX_algoSpace_EMIE_end_phi,  FEXAlgoSpaceDefs::jFEX_algoSpace_FCAL_end_phi};

    //loop over different EM/FCAL1 eta phi core fpga regions with different granularities. These are potential seed  towers for electron clusters 
    for(uint region = 0; region<3; region++) {
      for(int nphi = lower_centre_nphi[region]; nphi < upper_centre_nphi[region]; nphi++) {
	      for(int neta = lower_centre_neta[region]; neta < upper_centre_neta[region]; neta++) {
          
          // ignore  seeds for |eta| < 2.3 or from the first FCAL eta bin                
          if (m_jfex == 0 && neta >= FEXAlgoSpaceDefs::jFEX_algoSpace_C_FwdEl_start) continue;
          if (m_jfex == 5 && neta <= FEXAlgoSpaceDefs::jFEX_algoSpace_A_FwdEl_start) continue;
          if (m_jfex == 0 && neta == FEXAlgoSpaceDefs::jFEX_algoSpace_C_FCAL1_1st)   continue;
          if (m_jfex == 5 && neta == FEXAlgoSpaceDefs::jFEX_algoSpace_A_FCAL1_1st)   continue;
	         
          // define ttID (only FCAL1 in the third region) which will be the key for class in map, ignore tower ID = 0
          uint ttID = m_jFEXalgoTowerID[nphi][neta];
          if(ttID == 0) continue;
          //check if seed candidate is actual seed (passes local maximum criteria)
          if(!isValidSeed(ttID)) continue;
          
          //gather some first, basic information for resulting cluster/TOB
          jFEXForwardElecInfo elCluster;
          elCluster.setup(m_jfex, ttID, neta, nphi);
          const auto [centreTT_eta, centreTT_phi] = getEtaPhi(ttID);
          const auto [centreTT_EtEM, centreTT_EtHad] = getEtEmHad(ttID);
          elCluster.setCoreTTfPhi(centreTT_phi);
          elCluster.setCoreTTfEta(centreTT_eta);
          elCluster.setCoreTTEtEM(centreTT_EtEM);  
          elCluster.setCoreTTSatEM(getEMSat(ttID));                
          elCluster.setNextTTEtEM(0);
          elCluster.setNextTTID(0);
          elCluster.setTTEtEMiso(0);

          //find "NextTT", i.e., highest ET neighbour
          findAndFillNextTT(elCluster, neta, nphi);

          // sum up EM isolation using the isolation map and remove cluster ET
          {
            int sumEtEM = 0;
            auto it_iso_map = m_IsoMap.find(ttID);
            if(it_iso_map != m_IsoMap.end()) {
              for(const auto& gtt : it_iso_map->second){
                  auto [tmp_EtEM,tmp_EtHad] = getEtEmHad(gtt);
                  sumEtEM += tmp_EtEM;  
              }
              elCluster.setTTEtEMiso(sumEtEM-elCluster.getNextTTEtEM());
            } else {
                ATH_MSG_ERROR("Could not find TT" << ttID << " in jEM isolation map file.");
            }
          }
          
          if(fabs(centreTT_eta) < 3.2) {
            // for non-FCal positions only Frac1 is meaningful and has a "trivial" mapping
            elCluster.setTTEtHad1(centreTT_EtHad);
            elCluster.setTTEtHad2(0);
          } else {
            // sum up Et for hadronic fraction 1
            {
              int sumEtHad1 = 0;
              auto it_frac1_map = m_Frac1Map.find(ttID);
              if(it_frac1_map != m_Frac1Map.end()) {
                for(const auto& gtt : it_frac1_map->second){
                    auto [tmp_EtEM,tmp_EtHad] = getEtEmHad(gtt);
                    sumEtHad1 += tmp_EtHad;  
                }
                elCluster.setTTEtHad1(sumEtHad1);
              } else { 
                ATH_MSG_ERROR("Could not find TT" << ttID << " in jEM frac1 map file.");
              }
            }
            
            // sum up Et for hadronic fraction 2 (only FCal!)
            {
              int sumEtHad2 = 0;
              auto it_frac2_map = m_Frac2Map.find(ttID);
              if(it_frac2_map != m_Frac2Map.end()) {
                for(const auto& gtt : it_frac2_map->second) {
                    auto [tmp_EtEM,tmp_EtHad] = getEtEmHad(gtt);
                    sumEtHad2 += tmp_EtHad;  
                }
                elCluster.setTTEtHad2(sumEtHad2);
              } else { 
                ATH_MSG_ERROR("Could not find TT" << ttID << " in jEM frac2 map file.");
              }
            }
          }

          // save this cluster in the list
          clusterList[ttID] = elCluster;
        }//eta
      }//phi
    }// 3 regions
    return clusterList;
  } 
      
  StatusCode LVL1::jFEXForwardElecAlgo::ReadfromFile(const std::string & fileName, std::unordered_map<unsigned int, std::vector<unsigned int> >& fillingMap) const {
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
