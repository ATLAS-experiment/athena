/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFEXTOBEtTool  -  description
//                              -------------------
//     begin                : 13 12 2022
//     email                : Alan.Watson@CERN.CH
//  ***************************************************************************/
#include "L1CaloFEXSim/eFEXTOBEtTool.h"
#include "L1CaloFEXSim/eFEXegAlgo.h"
#include "L1CaloFEXSim/eFEXtauAlgo.h"
#include "L1CaloFEXSim/eTowerContainer.h"
#include <vector>

namespace LVL1 {

  // default constructor for persistency

eFEXTOBEtTool::eFEXTOBEtTool(const std::string& type,const std::string& name,const IInterface* parent):
  AthAlgTool(type,name,parent)
{
  declareInterface<IeFEXTOBEtTool>(this);
}
 
    
  /** Destructor */
  eFEXTOBEtTool::~eFEXTOBEtTool()
  {
  }

//---------------- Initialisation -------------------------------------------------
  
StatusCode eFEXTOBEtTool::initialize()
{
  
  ATH_CHECK( m_eFEXegAlgoTool.retrieve() );
  ATH_CHECK( m_eFEXtauAlgoTool.retrieve() );
  ATH_CHECK(m_eTowerContainerKey.initialize());

  return StatusCode::SUCCESS;
}
  
// The main user calls
StatusCode eFEXTOBEtTool::getegSums(float etaTOB, float phiTOB, int seed, int UnD,
                              std::vector<unsigned int> &ClusterCellETs, 
                              std::vector<unsigned int> &RetaSums,
                              std::vector<unsigned int> &RhadSums, 
                              std::vector<unsigned int> &WstotSums)
{

  /// Form grid of 3x3 tower IDs for this window
  int tobtable[3][3];

  for (int iphi = -1; iphi <= 1; ++iphi) {
    float phiTable = phiTOB + iphi*m_dphiTower;
    if (phiTable > M_PI)  phiTable -= 2*M_PI;
    if (phiTable < -M_PI) phiTable += 2*M_PI;

    for (int ieta = -1; ieta <= 1; ++ieta) {
      float etaTable = etaTOB + ieta*m_detaTower;

      // Set the tower ID if within acceptance, else 0
      if (std::abs(etaTable)<2.5) tobtable[iphi+1][ieta+1] = eTowerID(etaTable, phiTable);
      else                   tobtable[iphi+1][ieta+1] = 0;

    } // eta loop
  }  // phi loop

  /// Which eFEX & FPGA is responsible for this window?
  int eFEX, FPGA, fpgaEta;
  location(etaTOB,phiTOB, eFEX, FPGA, fpgaEta);

  // Set up e/g algorithm for this location
  ATH_CHECK( m_eFEXegAlgoTool->safetyTest() );
  m_eFEXegAlgoTool->setup(tobtable, eFEX, FPGA, fpgaEta);

  // Get ETs of cells making up the ET clusters
  m_eFEXegAlgoTool->getClusterCells(ClusterCellETs);
  
  // Get sums from algorithm
  // This will override the seed calculated by the algorithm with the one supplied here
  m_eFEXegAlgoTool->getSums(seed, UnD, RetaSums, RhadSums, WstotSums);

  // and we're done
  return StatusCode::SUCCESS;

}

StatusCode eFEXTOBEtTool::getTOBCellEnergies(float etaTOB, float phiTOB, std::vector<unsigned int> &ClusterCellETs){
	
	  /// Form grid of 3x3 tower IDs for this window
  int tobtable[3][3];

  for (int iphi = -1; iphi <= 1; ++iphi) {
    float phiTable = phiTOB + iphi*m_dphiTower;
    if (phiTable > M_PI)  phiTable -= 2*M_PI;
    if (phiTable < -M_PI) phiTable += 2*M_PI;

    for (int ieta = -1; ieta <= 1; ++ieta) {
      float etaTable = etaTOB + ieta*m_detaTower;

      // Set the tower ID if within acceptance, else 0
      if (std::abs(etaTable)<2.5) tobtable[iphi+1][ieta+1] = eTowerID(etaTable, phiTable);
      else                   tobtable[iphi+1][ieta+1] = 0;

    } // eta loop
  }  // phi loop

  SG::ReadHandle<eTowerContainer> eTowerContainer(m_eTowerContainerKey/*,ctx*/);
  ClusterCellETs.reserve(99); // will have a total of 99 supercell values (they are in counts of 25 MeV)

  for (unsigned int il = 0; il < 5; il++) { // layer loop
      size_t nCells = (il==1 || il==2) ? 4 : 1;
      for (unsigned int iphi = 0; iphi < 3; iphi++) { // tower phi loop
          for (unsigned int ieta = 0; ieta < 3; ieta++) { // tower eta loop
              if (tobtable[iphi][ieta]==0){ 
                  for(size_t c=0;c<nCells;c++) {
		                ClusterCellETs.push_back(0); // no energies for TOBs are the extreme eta values 
				          }
              } else {
                  const eTower * tower = eTowerContainer->findTower(tobtable[iphi][ieta]);
                  if (tower==nullptr) {
                    ATH_MSG_ERROR("No tower with id " << tobtable[iphi][ieta]);
                    return StatusCode::FAILURE;
                  }
                  for(size_t c=0;c<nCells;c++) {
                      // Note: energy counts can technically be negative (although noise cuts should usually prevent this)
                      // But algo developers wanted the energy count decoration to be a vector of unsigned int for historical reasons
                      // It is therefore possible for the unsigned int to be an overflow of int max
                      ClusterCellETs.push_back( tower->getET(il,c) );
                  }
                }
            }
        }
    }
    
    /*----------- SuperCells eta, phi, layer coordinates ----------------------
     0:{0, 0, 0}, 1:{1, 0, 0}, 2:{2, 0, 0},  
     3:{0, 1, 0}, 4:{1, 1, 0}, 5:{2, 1, 0},
     6:{0, 2, 0}, 7:{1, 2, 0}, 8:{2, 2, 0},  
     9:{0, 0, 1}, 10:{1, 0, 1}, 11:{2, 0, 1}, 12:{3, 0, 1}, 13:{4, 0, 1}, 14:{5, 0, 1},  15:{6, 0, 1}, 16:{7, 0, 1}, 17:{8, 0, 1}, 18:{9, 0, 1}, 19:{10, 0, 1}, 20:{11, 0, 1}, 
    21:{0, 1, 1}, 22:{1, 1, 1}, 23:{2, 1, 1}, 24:{3, 1, 1}, 25:{4, 1, 1}, 26:{5, 1, 1},  27:{6, 1, 1}, 28:{7, 1, 1}, 29:{8, 1, 1}, 30:{9, 1, 1}, 31:{10, 1, 1}, 32:{11, 1, 1}, 
    33:{0, 2, 1}, 34:{1, 2, 1}, 35:{2, 2, 1}, 36:{3, 2, 1}, 37:{4, 2, 1}, 38:{5, 2, 1},  39:{6, 2, 1}, 40:{7, 2, 1}, 41:{8, 2, 1}, 42:{9, 2, 1}, 43:{10, 2, 1}, 44:{11, 2, 1}, 
    44:{0, 0, 2}, 45:{1, 0, 2}, 46:{2, 0, 2}, 47:{3, 0, 2}, 48:{4, 0, 2},  49:{5, 0, 2},  50:{6, 0, 2}, 51:{7, 0, 2}, 52:{8, 0, 2}, 53:{9, 0, 2}, 54:{10, 0, 2}, 55:{11, 0, 2}, 
    56:{0, 1, 2}, 57:{1, 1, 2}, 58:{2, 1, 2}, 59:{3, 1, 2}, 60:{4, 1, 2},  61:{5, 1, 2},  62:{6, 1, 2}, 63:{7, 1, 2}, 64:{8, 1, 2}, 65:{9, 1, 2}, 66:{10, 1, 2}, 67:{11, 1, 2}, 
    68:{0, 2, 2}, 69:{1, 2, 2}, 70:{2, 2, 2}, 71:{3, 2, 2}, 72:{4, 2, 2},  73:{5, 2, 2},  74:{6, 2, 2}, 75:{7, 2, 2}, 76:{8, 2, 2}, 77:{9, 2, 2}, 78:{10, 2, 2}, 79:{11, 2, 2}, 
    81:{0, 0, 3}, 82:{1, 0, 3}, 83:{2, 0, 3},
    84:{0, 1, 3}, 85:{1, 1, 3}, 86:{2, 1, 3},  
    87:{0, 2, 3}, 88:{1, 2, 3}, 89:{2, 2, 3},
    90:{0, 0, 4}, 91:{1, 0, 4},  92:{2, 0, 4},  
    93:{0, 1, 4}, 94:{1, 1, 4}, 95:{2, 1, 4},
    96:{0, 2, 4}, 97:{1, 2, 4}, 98:{2, 2, 4}; */
  
  return StatusCode::SUCCESS;
}


StatusCode eFEXTOBEtTool::gettauSums(float etaTOB, float phiTOB, int seed, int UnD, 
                              std::vector<unsigned int> &RcoreSums,
                              std::vector<unsigned int> &RemSums)
{

 /// Form grid of 3x3 tower IDs for this window
  int tobtable[3][3];

  for (int iphi = -1; iphi <= 1; ++iphi) {
    float phiTable = phiTOB + iphi*m_dphiTower;
    if (phiTable > M_PI)  phiTable -= 2*M_PI;
    if (phiTable < -M_PI) phiTable += 2*M_PI;

    for (int ieta = -1; ieta <= 1; ++ieta) {
      float etaTable = etaTOB + ieta*m_detaTower;

      // Set the tower ID if within acceptance, else 0
      if (std::abs(etaTable)<2.5) tobtable[iphi+1][ieta+1] = eTowerID(etaTable, phiTable);
      else                   tobtable[iphi+1][ieta+1] = 0;

    } // eta loop
  }  // phi loop

  /// Which eFEX & FPGA is responsible for this window?
  int eFEX, FPGA, fpgaEta;
  location(etaTOB,phiTOB, eFEX, FPGA, fpgaEta);

  // Set up e/g algorithm for this location
  ATH_CHECK( m_eFEXtauAlgoTool->safetyTest() );
  m_eFEXtauAlgoTool->setup(tobtable, eFEX, FPGA, fpgaEta);

  // Get sums from algorithm
  // This will override the seed calculated by the algorithm with the one supplied here
  m_eFEXtauAlgoTool->getSums(seed, UnD, RcoreSums, RemSums);

  // and we're done
  return StatusCode::SUCCESS;

}


// Find eTower ID from a floating point coordinate pair
unsigned int eFEXTOBEtTool::eTowerID(float eta, float phi) const
{
  // Calculate ID by hand from coordinate
  int posneg = (eta >= 0 ? 1 : -1);
  int towereta = std::abs(eta)/0.1;
  if (phi < 0) phi += 2*M_PI;
  int towerphi = int(32*phi/M_PI);
  unsigned int tower_id = towerphi + 64*towereta;
  
  if (towereta < 14) {
    tower_id += (posneg > 0 ? 200000 : 100000);
  }
  else if (towereta == 14) {
    tower_id += (posneg > 0 ? 400000 : 300000);    
  }
  else {
    tower_id += (posneg > 0 ? 600000 : 500000);    
  }

  return tower_id;
}


// Find eFEX and FPGA numbers and eta index within FPGA
void eFEXTOBEtTool::location(float etaTOB, float phiTOB, int& eFEX, int& FPGA, int& etaIndex)
{
  // indices of central tower within a 0->49, 0->63 eta,phi map
  int ieta = (etaTOB + 2.5)/m_detaTower;
  float phiShifted = phiTOB + 2*m_dphiTower; // eFEX boundary does not line up with phi = 0
  int iphi = (phiShifted > 0 ? phiShifted/m_dphiTower : (phiShifted + 2*M_PI)/m_dphiTower );

  // Now we have global 0->N indices we can simply calculate which eFEX these come from
  int eFEXPhi = iphi/8;
  int eFEXeta = 0;
  if (ieta >  16) eFEXeta = 1;
  if (ieta >  32) eFEXeta = 2;

  // eFEX number in range 0 -> 23
  eFEX = eFEXeta + 3*eFEXPhi;

  // Now which FPGA within the eFEX?
  // This logic will give an index:  0 -> 16 for eFEX 0
  //                                 1 -> 16 for eFEX 1
  //                                 1 -> 17 for eFEX 2
  // which puts FPGA boundaries at 4, 8, 12 in all cases
  int eFEXIndex;
  switch(eFEXeta) {
    case 0: {
      eFEXIndex = ieta;
      break;
    }
    case 1: {
      eFEXIndex = ieta -16;
      break;
    }
    case 2: {
      eFEXIndex = ieta -32;
      break;
    }
  }

  // Finally we can calculate the FPGA number
  if (eFEXIndex <= 4)       FPGA = 0;
  else if (eFEXIndex <= 8)  FPGA = 1;
  else if (eFEXIndex <= 12) FPGA = 2;
  else                      FPGA = 3;

  // And eta index within the FPGA = 1-4 in most cases
  // except for eFEX 0 FPGA 0 => 0-4 and eFEX 2 FPGA 3 => 1-5
  etaIndex = eFEXIndex - 4*FPGA;

  // And return the results by reference
  return;
}

} // end of namespace bracket
