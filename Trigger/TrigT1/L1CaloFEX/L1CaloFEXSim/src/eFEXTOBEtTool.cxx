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

StatusCode eFEXTOBEtTool::getTOBCellEnergies(float etaTOB, float phiTOB, std::vector<int> &ClusterCellETs){
	
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
              if (tobtable[ieta][iphi]==0){ 
                  for(size_t c=0;c<nCells;c++) {
		                ClusterCellETs.push_back(0); // no energies for TOBs are the extreme eta values 
				          }
              } else {
                  const eTower * tower = eTowerContainer->findTower(tobtable[ieta][iphi]);
                  if (tower==nullptr) {
                    ATH_MSG_ERROR("No tower with id " << tobtable[ieta][iphi]);
                    return StatusCode::FAILURE;
                  }
                  for(size_t c=0;c<nCells;c++) {
			               ClusterCellETs.push_back( tower->getET(il,c) );
                  }
                }
            }
        }
    }

  // and we're done
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
