/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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

namespace LVL1
{

   // default constructor for persistency

   gFEXFPGA::gFEXFPGA(const std::string &type, const std::string &name, const IInterface *parent) : AthAlgTool(type, name, parent)
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
      ATH_CHECK(m_DBToolKey.initialize());

      return StatusCode::SUCCESS;
   }

   StatusCode gFEXFPGA::init(int id)
   {
      m_fpgaId = id;

      return StatusCode::SUCCESS;
   }

   void gFEXFPGA::reset()
   {

      m_fpgaId = -1;
   }

   void gFEXFPGA::FillgTowerEDMCentral(SG::WriteHandle<xAOD::gFexTowerContainer> &gTowersContainer, // output
                                       gTowersCentral &gTowersIDs_central,                          // input, IDs
                                                                                                    // int fpga,                                                     // input fpga (A=0, B=1)
                                       gTowersType &output_gTower_energies,                         // output, 200 MeV
                                       gTowersType &output_gTower50_energies,                       // output, 50 MeV
                                       gTowersType &output_saturation)
   { // output, saturation
      SG::ReadCondHandle<gFEXDBCondData> myDBTool = SG::ReadCondHandle<gFEXDBCondData>(m_DBToolKey);
      if (!myDBTool.isValid())
      {
         ATH_MSG_ERROR("Could not retrieve DB tool " << m_DBToolKey);
      }
      gFEXFPGA::calExpand(m_offsetsDefaultA, m_noiseCutsDefaultA, m_slopesDefaultA, 48, myDBTool->get_AnoiseCuts(), myDBTool->get_Aslopes());
      gFEXFPGA::calExpand(m_offsetsDefaultB, m_noiseCutsDefaultB, m_slopesDefaultB, 48, myDBTool->get_BnoiseCuts(), myDBTool->get_Bslopes());

      float Eta = 99;
      float Phi = 99;
      int TowerEt = -99;
      int Fpga = m_fpgaId;
      char IsSaturated = 0;

      float etaSum = 0;

      SG::ReadHandle<gTowerContainer> gFEXFPGA_gTowerContainer(m_gFEXFPGA_gTowerContainerKey /*,ctx*/);     // 200 MeV
      SG::ReadHandle<gTowerContainer> gFEXFPGA_gTower50Container(m_gFEXFPGA_gTower50ContainerKey /*,ctx*/); // 50 MeV

      bool is_mc = false;
      if (!gFEXFPGA_gTower50Container.isValid())
      {
         is_mc = true;
      }

      int rows = gTowersIDs_central.size();
      int cols = gTowersIDs_central[0].size();

      for (int myrow = 0; myrow < rows; myrow++)
      {
         for (int mycol = 0; mycol < cols; mycol++)
         {

            output_gTower_energies[myrow][mycol] = 0;
            output_gTower50_energies[myrow][mycol] = 0;
            output_saturation[myrow][mycol] = 0;

            int towerID = gTowersIDs_central[myrow][mycol];
            if (towerID == 0)
               continue;

            const LVL1::gTower *tmpTower = gFEXFPGA_gTowerContainer->findTower(towerID);
            const LVL1::gTower *tmpTower50 = tmpTower;
            if (!is_mc)
            {
               tmpTower50 = gFEXFPGA_gTower50Container->findTower(towerID);
            }

            if (tmpTower == nullptr)
               continue;

            TowerEt = tmpTower->getET();
            Eta = tmpTower->eta();
            Phi = tmpTower->phi();

            etaSum += Eta;

            int iPhiFW, iEtaFW;
            uint32_t gFEXtowerID = tmpTower->getFWID(iPhiFW, iEtaFW);
            IsSaturated = tmpTower->isSaturated();
            std::unique_ptr<xAOD::gFexTower> gTowerEDM(new xAOD::gFexTower());
            gTowersContainer->push_back(std::move(gTowerEDM));
            gTowersContainer->back()->initialize(iEtaFW, iPhiFW, Eta, Phi, TowerEt, Fpga, IsSaturated, gFEXtowerID);

            output_gTower_energies[myrow][mycol] = tmpTower->getET();
            output_gTower50_energies[myrow][mycol] = is_mc ? tmpTower50->getET() * 4. : tmpTower50->getET();
            output_saturation[myrow][mycol] = tmpTower->isSaturated();
         }
      }

      // apply defualt slopes set in initialization.
      // In the future these values will be read from the online COOL data base
      // Note the unforutnate hack used to figure out if we are in FPGA A or B.
      // 

      if (etaSum < 0)
      {
         // FPGA A
         gFEXFPGA::gtCalib(output_gTower_energies, m_offsetsDefaultA, m_noiseCutsDefaultA, m_slopesDefaultA);
      }
      else
      {
         // FPGA B
         gFEXFPGA::gtCalib(output_gTower_energies, m_offsetsDefaultB, m_noiseCutsDefaultB, m_slopesDefaultB);
      }
   }

   void gFEXFPGA::FillgTowerEDMForward(SG::WriteHandle<xAOD::gFexTowerContainer> &gTowersContainer,
                                       gTowersForward &gTowersIDs_forward_n,
                                       gTowersForward &gTowersIDs_forward_p,
                                       gTowersType &output_gTower_energies,
                                       gTowersType &output_gTower50_energies,
                                       gTowersType &output_saturation)
   {
      SG::ReadCondHandle<gFEXDBCondData> myDBTool = SG::ReadCondHandle<gFEXDBCondData>(m_DBToolKey);
      if (!myDBTool.isValid())
      {
         ATH_MSG_ERROR("Could not retrieve DB tool " << m_DBToolKey);
      }
      gFEXFPGA::calExpand(m_offsetsDefaultC, m_noiseCutsDefaultC, m_slopesDefaultC, 48, myDBTool->get_CnoiseCuts(), myDBTool->get_Cslopes());

      char IsSaturated = 0;

      SG::ReadHandle<gTowerContainer> gFEXFPGA_gTowerContainer(m_gFEXFPGA_gTowerContainerKey /*,ctx*/);
      SG::ReadHandle<gTowerContainer> gFEXFPGA_gTower50Container(m_gFEXFPGA_gTower50ContainerKey /*,ctx*/);

      bool is_mc = false;
      if (!gFEXFPGA_gTower50Container.isValid())
      {
         is_mc = true;
      }

      //
      // C-N
      //
      int rows = gTowersIDs_forward_n.size();
      int cols = gTowersIDs_forward_n[0].size();

      for (int myrow = 0; myrow < rows; myrow++)
      {
         for (int mycol = 0; mycol < cols; mycol++)
         {

            int towerID = gTowersIDs_forward_n[myrow][mycol];
            if (towerID == 0)
               continue;

            const LVL1::gTower *tmpTower = gFEXFPGA_gTowerContainer->findTower(towerID);
            const LVL1::gTower *tmpTower50 = tmpTower;
            if (!is_mc)
            {
               tmpTower50 = gFEXFPGA_gTower50Container->findTower(towerID);
            }

            if (tmpTower == nullptr)
               continue;

            int TowerEt = tmpTower->getET();
            float Eta = tmpTower->eta();
            float Phi = tmpTower->phi();
            int Fpga = m_fpgaId;
            int iPhiFW, iEtaFW;
            uint32_t gFEXtowerID = tmpTower->getFWID(iPhiFW, iEtaFW);
            IsSaturated = tmpTower->isSaturated();
            std::unique_ptr<xAOD::gFexTower> gTowerEDM(new xAOD::gFexTower());
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

      for (int myrow = 0; myrow < rows; myrow++)
      {
         for (int mycol = 0; mycol < cols; mycol++)
         {

            int towerID = gTowersIDs_forward_p[myrow][mycol];
            if (towerID == 0)
               continue;

            const LVL1::gTower *tmpTower = gFEXFPGA_gTowerContainer->findTower(towerID);
            const LVL1::gTower *tmpTower50 = tmpTower;
            if (!is_mc)
            {
               tmpTower50 = gFEXFPGA_gTower50Container->findTower(towerID);
            }

            if (tmpTower == nullptr)
               continue;

            int TowerEt = tmpTower->getET();
            float Eta = tmpTower->eta();
            float Phi = tmpTower->phi();
            int Fpga = m_fpgaId;
            int iPhiFW, iEtaFW;
            uint32_t gFEXtowerID = tmpTower->getFWID(iPhiFW, iEtaFW);
            IsSaturated = tmpTower->isSaturated();
            std::unique_ptr<xAOD::gFexTower> gTowerEDM(new xAOD::gFexTower());
            gTowersContainer->push_back(std::move(gTowerEDM));
            gTowersContainer->back()->initialize(iEtaFW, iPhiFW, Eta, Phi, TowerEt, Fpga, IsSaturated, gFEXtowerID);

            output_gTower_energies[iPhiFW][iEtaFW - 32 + 6] = tmpTower->getET();
            output_gTower50_energies[iPhiFW][iEtaFW - 32 + 6] = is_mc ? tmpTower50->getET() * 4. : tmpTower50->getET();
            output_saturation[iPhiFW][iEtaFW - 32 + 6] = tmpTower->isSaturated();
         }
      }

      // apply defualt slopes set in initialization.
      // In the future these values will be read from the online COOL data base

      gFEXFPGA::gtCalib(output_gTower_energies, m_offsetsDefaultC, m_noiseCutsDefaultC, m_slopesDefaultC);
   }

   void gFEXFPGA::gtCalib(gTowersType &twrs, const gTowersType &offsets, const gTowersType &noiseCuts, const gTowersType &slopes) const
   {
      int rows = twrs.size();
      int cols = twrs[0].size();
      for (int irow = 0; irow < rows; irow++)
      {
         for (int jcolumn = 0; jcolumn < cols; jcolumn++)
         {
            twrs[irow][jcolumn] = twrs[irow][jcolumn] + offsets[irow][jcolumn];
            calLookup(&twrs[irow][jcolumn], offsets[irow][jcolumn], noiseCuts[irow][jcolumn], slopes[irow][jcolumn]);
         }
      }
   }

   void gFEXFPGA::calLookup(int *tower, const int offset, const int noiseCut, const int calib) const
   {
      int address = *tower;

      if (address < 0)
      {
         ATH_MSG_DEBUG("gTower lookup address out of range " << address);
         address = 0;
      }
      if (address > 2047)
      {
         ATH_MSG_DEBUG("gTower lookup address out of range " << address);
         address = 2047;
      }

      // noise cut is made before calibraiton
      if ((address - offset) < noiseCut)
         address = offset;

      int calTower = ((calib * address + 511) >> 10) - ((calib * offset + 511) >> 10);

      if (calTower < -2048)
         calTower = -2048;
      if (calTower > 2047)
         calTower = 2047;
      if (address == 2047)  calTower = 2047;

      *tower = calTower;
   }

   void gFEXFPGA::calExpand(gTowersType &offsets, gTowersType &noiseCuts, gTowersType &slopes, const int offset, const std::array<int, 12>& columnNoiseCuts, const std::array<int, 12>& columnSlopes) const
   {

      int rows = offsets.size();
      int cols = offsets[0].size();
      for (int irow = 0; irow < rows; irow++)
      {
         for (int jcolumn = 0; jcolumn < cols; jcolumn++)
         {
            offsets[irow][jcolumn] = offset;
            noiseCuts[irow][jcolumn] = columnNoiseCuts[jcolumn];
            slopes[irow][jcolumn] = columnSlopes[jcolumn];
         }
      }
   }

} // end of namespace bracket
