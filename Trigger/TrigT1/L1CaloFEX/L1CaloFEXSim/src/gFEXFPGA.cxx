/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//    gFEXFPGA - Defines FPGA tools
//                              -------------------
//     begin                : 01 04 2021
//     email                : cecilia.tosciri@cern.ch
//***************************************************************************

#include "L1CaloFEXSim/gFEXFPGA.h"
#include "L1CaloFEXSim/gTowerContainer.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"
#include <algorithm>


namespace LVL1 {

   // default constructor for persistency

  gFEXFPGA::gFEXFPGA(const std::string& type,const std::string& name,const IInterface* parent):
     AthAlgTool(type,name,parent)
  {
     declareInterface<IgFEXFPGA>(this);
  }

     /** Destructor */
     gFEXFPGA::~gFEXFPGA()
     {
     }

  //---------------- Initialisation -------------------------------------------------

  StatusCode gFEXFPGA::initialize()
  {

     ATH_CHECK(m_gFEXFPGA_gTowerContainerKey.initialize());
     ATH_CHECK(m_gFEXFPGA_gTower50ContainerKey.initialize());

     return StatusCode::SUCCESS;
  }


  StatusCode gFEXFPGA::init(int id)
  {
     m_fpgaId = id;

     return StatusCode::SUCCESS;
  }

  void gFEXFPGA::reset(){

     m_fpgaId = -1;
  }


   void gFEXFPGA::FillgTowerEDMCentral(SG::WriteHandle<xAOD::gFexTowerContainer> & gTowersContainer, // output
                                       gTowersCentral & gTowersIDs_central,                          // input, IDs
                                       gTowersType & output_gTower_energies,                         // output, 200 MeV
                                       gTowersType & output_gTower50_energies,                       // output, 50 MeV
                                       gTowersType & output_saturation) {                            // output, saturation                          

      float Eta = 99;
      float Phi = 99;                                
      int TowerEt = -99;
      int Fpga = m_fpgaId;
      char IsSaturated = 0;

      SG::ReadHandle<gTowerContainer> gFEXFPGA_gTowerContainer(m_gFEXFPGA_gTowerContainerKey/*,ctx*/);     // 200 MeV
      SG::ReadHandle<gTowerContainer> gFEXFPGA_gTower50Container(m_gFEXFPGA_gTower50ContainerKey/*,ctx*/); // 50 MeV

      bool is_mc = false;
      if (!gFEXFPGA_gTower50Container.isValid()) {
         is_mc = true;
      }

      int rows = gTowersIDs_central.size();
      int cols = gTowersIDs_central[0].size();

      for (int myrow = 0; myrow<rows; myrow++){
         for (int mycol = 0; mycol<cols; mycol++){

            output_gTower_energies[myrow][mycol] = 0;
            output_gTower50_energies[myrow][mycol] = 0;
            output_saturation[myrow][mycol] = 0;


            int towerID = gTowersIDs_central[myrow][mycol];
            if(towerID == 0) continue;

            const LVL1::gTower * tmpTower = gFEXFPGA_gTowerContainer->findTower(towerID);
            const LVL1::gTower * tmpTower50 = tmpTower;
            if (!is_mc) {
               tmpTower50 = gFEXFPGA_gTower50Container->findTower(towerID);
            }

            if (tmpTower == nullptr) continue;

            TowerEt = tmpTower->getET();
            Eta = tmpTower->eta();
            Phi = tmpTower->phi();
            int iPhiFW, iEtaFW;
            uint32_t gFEXtowerID = tmpTower->getFWID(iPhiFW, iEtaFW);
            IsSaturated = tmpTower->isSaturated();
            std::unique_ptr<xAOD::gFexTower> gTowerEDM (new xAOD::gFexTower());
            gTowersContainer->push_back(std::move(gTowerEDM));
            gTowersContainer->back()->initialize(iEtaFW, iPhiFW, Eta, Phi, TowerEt, Fpga, IsSaturated, gFEXtowerID);
            
            output_gTower_energies[myrow][mycol] = tmpTower->getET();
            output_gTower50_energies[myrow][mycol] = is_mc ? tmpTower50->getET() * 4. : tmpTower50->getET();
            output_saturation[myrow][mycol] = tmpTower->isSaturated();
         }
      }
   }

   void gFEXFPGA::FillgTowerEDMForward(SG::WriteHandle<xAOD::gFexTowerContainer> & gTowersContainer,
                                       gTowersForward & gTowersIDs_forward_n,
                                       gTowersForward & gTowersIDs_forward_p,
                                       gTowersType & output_gTower_energies,
                                       gTowersType & output_gTower50_energies,
                                       gTowersType & output_saturation) {

      char IsSaturated = 0;

      SG::ReadHandle<gTowerContainer> gFEXFPGA_gTowerContainer(m_gFEXFPGA_gTowerContainerKey/*,ctx*/);
      SG::ReadHandle<gTowerContainer> gFEXFPGA_gTower50Container(m_gFEXFPGA_gTower50ContainerKey/*,ctx*/);

      bool is_mc = false;
      if (!gFEXFPGA_gTower50Container.isValid()) {
         is_mc = true;
      }
      

      //
      // C-N
      //
      int rows = gTowersIDs_forward_n.size();
      int cols = gTowersIDs_forward_n[0].size();

      for (int myrow = 0; myrow<rows; myrow++){
         for (int mycol = 0; mycol<cols; mycol++){

            int towerID = gTowersIDs_forward_n[myrow][mycol];
            if(towerID == 0) continue;

            const LVL1::gTower * tmpTower = gFEXFPGA_gTowerContainer->findTower(towerID);
            const LVL1::gTower * tmpTower50 = tmpTower;
            if (!is_mc) {
               tmpTower50 = gFEXFPGA_gTower50Container->findTower(towerID);
            }

            if (tmpTower == nullptr) continue;
  
            int TowerEt = tmpTower->getET();
            float Eta = tmpTower->eta();
            float Phi = tmpTower->phi();
            int Fpga = m_fpgaId;
            int iPhiFW, iEtaFW;
            uint32_t gFEXtowerID = tmpTower->getFWID(iPhiFW, iEtaFW);
            IsSaturated = tmpTower->isSaturated();
            std::unique_ptr<xAOD::gFexTower> gTowerEDM (new xAOD::gFexTower());
            gTowersContainer->push_back(std::move(gTowerEDM));
            gTowersContainer->back()->initialize(iEtaFW, iPhiFW, Eta, Phi, TowerEt, Fpga, IsSaturated, gFEXtowerID);

            output_gTower_energies[iPhiFW][iEtaFW - 2] = tmpTower->getET();
            output_gTower50_energies[iPhiFW][iEtaFW - 2] = is_mc ? tmpTower50->getET() * 4. : tmpTower50->getET();
            output_saturation[iPhiFW][iEtaFW - 2] = tmpTower->isSaturated();
         }
      }

      //
      // C-P
      //
      rows = gTowersIDs_forward_p.size();
      cols = gTowersIDs_forward_p[0].size();

      for (int myrow = 0; myrow<rows; myrow++){
         for (int mycol = 0; mycol<cols; mycol++){

            int towerID = gTowersIDs_forward_p[myrow][mycol];
            if(towerID == 0) continue;

            const LVL1::gTower * tmpTower = gFEXFPGA_gTowerContainer->findTower(towerID);
            const LVL1::gTower * tmpTower50 = tmpTower;
            if (!is_mc) {
               tmpTower50 = gFEXFPGA_gTower50Container->findTower(towerID);
            }

            if (tmpTower == nullptr) continue;
  
            int TowerEt = tmpTower->getET();
            float Eta = tmpTower->eta();
            float Phi = tmpTower->phi();
            int Fpga = m_fpgaId;
            int iPhiFW, iEtaFW;
            uint32_t gFEXtowerID = tmpTower->getFWID(iPhiFW, iEtaFW);
            IsSaturated = tmpTower->isSaturated();
            std::unique_ptr<xAOD::gFexTower> gTowerEDM (new xAOD::gFexTower());
            gTowersContainer->push_back(std::move(gTowerEDM));
            gTowersContainer->back()->initialize(iEtaFW, iPhiFW, Eta, Phi, TowerEt, Fpga, IsSaturated, gFEXtowerID);

            output_gTower_energies[iPhiFW][iEtaFW - 32 + 6] = tmpTower->getET();
            output_gTower50_energies[iPhiFW][iEtaFW - 32 + 6] = is_mc ? tmpTower50->getET() * 4. : tmpTower50->getET();
            output_saturation[iPhiFW][iEtaFW - 32 + 6] = tmpTower->isSaturated();
         }
      }
   }


} // end of namespace bracket
