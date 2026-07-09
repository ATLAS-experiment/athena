/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//    gFEXSysSim - Overall gFEX simulation
//                              -------------------
//     begin                : 01 04 2021
//     email                : cecilia.tosciri@cern.ch
//***************************************************************************

#include "gFEXSysSim.h"
#include "gFEXSim.h"
#include "L1CaloFEXSim/gTower.h"
#include "L1CaloFEXSim/gTowerContainer.h"

#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"

#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"

namespace LVL1 {

   //---------------- Initialisation -------------------------------------------------

   StatusCode gFEXSysSim::initialize()
   {

      ATH_CHECK(m_gTowerContainerSGKey.initialize());

      ATH_CHECK(m_gFEXSimTool.retrieve());

      ATH_CHECK(m_gFexRhoOutKey.initialize()); 

      ATH_CHECK(m_gFexBlockOutKey.initialize());      

      ATH_CHECK(m_gFexJetOutKey.initialize());

      ATH_CHECK(m_gScalarEJwojOutKey.initialize());

      ATH_CHECK(m_gMETComponentsJwojOutKey.initialize());

      ATH_CHECK(m_gMHTComponentsJwojOutKey.initialize());
      
      ATH_CHECK(m_gMSTComponentsJwojOutKey.initialize());

      ATH_CHECK(m_gMETComponentsNoiseCutOutKey.initialize());

      ATH_CHECK(m_gMETComponentsRmsOutKey.initialize());

      ATH_CHECK(m_gScalarENoiseCutOutKey.initialize());

      ATH_CHECK(m_gScalarERmsOutKey.initialize());

      ATH_CHECK(m_l1MenuKey.initialize());

      return StatusCode::SUCCESS;
   }



  void gFEXSysSim::cleanup()   {
  }


   int gFEXSysSim::calcTowerID(int eta, int phi, int nphi, int mod) const {

      return ((nphi*eta) + phi + mod);
   }


   StatusCode gFEXSysSim::execute(const EventContext& ctx, gFEXOutputCollection* gFEXOutputs) {

      SG::ReadHandle<LVL1::gTowerContainer> this_gTowerContainer(m_gTowerContainerSGKey,ctx);
      if(!this_gTowerContainer.isValid()){
         ATH_MSG_FATAL("Could not retrieve gTowerContainer " << m_gTowerContainerSGKey.key());
         return StatusCode::FAILURE;
      }

      // int centralNphi = 32;
      // int forwardNphi = 16;

      int fcalEta = 19; int fcalPhi = 0; int fcalMod = 900000;
      int initialFCAL = calcTowerID(fcalEta,fcalPhi,FEXAlgoSpaceDefs::forwardNphi,fcalMod);//900304
      int transfcalEta = 15; int transfcalPhi = 0; int transfcalMod = 700000;
      int initialTRANSFCAL = calcTowerID(transfcalEta,transfcalPhi,FEXAlgoSpaceDefs::centralNphi,transfcalMod);//700480
      int emecEta = 11; int emecPhi = 0; int emecMod = 500000;
      int initialEMEC = calcTowerID(emecEta,emecPhi,FEXAlgoSpaceDefs::centralNphi,emecMod);//500384
      int transembEta = 7; int transembPhi = 0; int transembMod = 300000;
      int initialTRANSEMB = calcTowerID(transembEta,transembPhi,FEXAlgoSpaceDefs::centralNphi,transembMod);///300224
      int embEta = 6; int embPhi = 0; int embMod = 100000;
      int initialEMB = calcTowerID(embEta,embPhi,FEXAlgoSpaceDefs::centralNphi,embMod);//100192


      int embposEta = 0; int embposPhi = 0; int embposMod = 200000;
      int initialposEMB = calcTowerID(embposEta,embposPhi,FEXAlgoSpaceDefs::centralNphi,embposMod);//200000
      int transembposEta = 7; int transembposPhi = 0; int transembposMod = 400000;
      int initialposTRANSEMB = calcTowerID(transembposEta,transembposPhi,FEXAlgoSpaceDefs::centralNphi,transembposMod);//400224
      int emecposEta = 8; int emecposPhi = 0; int emecposMod = 600000;
      int initialposEMEC = calcTowerID(emecposEta,emecposPhi,FEXAlgoSpaceDefs::centralNphi,emecposMod);//600256
      int transfcalposEta = 12; int transfcalposPhi = 0; int transfcalposMod = 800000;
      int initialposTRANSFCAL = calcTowerID(transfcalposEta,transfcalposPhi,FEXAlgoSpaceDefs::centralNphi,transfcalposMod);//800416
      int fcalposEta = 16; int fcalposPhi = 0; int fcalposMod = 1000000;
      int initialposFCAL = calcTowerID(fcalposEta,fcalposPhi,FEXAlgoSpaceDefs::forwardNphi,fcalposMod);//1000240


      // Since gFEX consists of a single module, here we are just (re)assigning the gTowerID

      // Defining a matrix 32x40 corresponding to the gFEX structure (32 phi x 40 eta in the most general case - forward region has 16 phi bins)
      typedef  std::array<std::array<int, FEXAlgoSpaceDefs::totalNeta>, FEXAlgoSpaceDefs::centralNphi> gTowersIDs;
      gTowersIDs tmp_gTowersIDs_subset;

      int rows = tmp_gTowersIDs_subset.size();
      int cols = tmp_gTowersIDs_subset[0].size();

      // set the FCAL negative part
      for(int thisCol=0; thisCol<4; thisCol++){
         for(int thisRow=0; thisRow<rows/2; thisRow++){
            int towerid = initialFCAL - ((thisCol) * (FEXAlgoSpaceDefs::forwardNphi)) + thisRow;
            tmp_gTowersIDs_subset[thisRow][thisCol] = towerid;
         }
      }

      // set the TRANSFCAL negative part (FCAL-EMEC overlap)
      for(int thisCol=4; thisCol<8; thisCol++){
         for(int thisRow=0; thisRow<rows; thisRow++){
            int towerid = initialTRANSFCAL - ((thisCol-4) * (FEXAlgoSpaceDefs::centralNphi)) + thisRow;
            tmp_gTowersIDs_subset[thisRow][thisCol] = towerid;
         }
      }

      // set the EMEC negative part
      for(int thisCol=8; thisCol<12; thisCol++){
         for(int thisRow=0; thisRow<rows; thisRow++){
            int towerid = initialEMEC - ((thisCol-8) * (FEXAlgoSpaceDefs::centralNphi)) + thisRow;
            tmp_gTowersIDs_subset[thisRow][thisCol] = towerid;
         }
      }

      // set the TRANSEMB (EMB-EMEC overlap) negative part
      for(int thisRow = 0; thisRow < rows; thisRow++){
         int thisCol = 12;
         int towerid = initialTRANSEMB + thisRow;
         tmp_gTowersIDs_subset[thisRow][thisCol] = towerid;
      }

      // set the EMB negative part
      for(int thisCol = 13; thisCol < 20; thisCol++){
         for(int thisRow=0; thisRow<rows; thisRow++){
           int towerid = initialEMB - ( (thisCol-13) * (FEXAlgoSpaceDefs::centralNphi)) + thisRow;
           tmp_gTowersIDs_subset[thisRow][thisCol] = towerid;
         }
      }

         // set the EMB positive part
      for(int thisCol = 20; thisCol < 27; thisCol++){
         for(int thisRow=0; thisRow<rows; thisRow++){
            int towerid = initialposEMB + ( (thisCol-20) * (FEXAlgoSpaceDefs::centralNphi)) + thisRow;
            tmp_gTowersIDs_subset[thisRow][thisCol] = towerid;
         }
      }

      // set the TRANSEMB (EMB-EMEC overlap) positive part
      for(int thisRow = 0; thisRow < rows; thisRow++){
         int thisCol = 27;
         int towerid = initialposTRANSEMB + thisRow;
         tmp_gTowersIDs_subset[thisRow][thisCol] = towerid;
      }
      // set the EMEC positive part
      for(int thisCol=28; thisCol<32; thisCol++){
         for(int thisRow=0; thisRow<rows; thisRow++){
            int towerid = initialposEMEC + ((thisCol-28) * (FEXAlgoSpaceDefs::centralNphi)) + thisRow;
            tmp_gTowersIDs_subset[thisRow][thisCol] = towerid;
         }
      }

      // set the TRANSFCAL positive part (EMEC-FCAL overlap)
      for(int thisCol=32; thisCol<36; thisCol++){
         for(int thisRow=0; thisRow<rows; thisRow++){
            int towerid = initialposTRANSFCAL + ((thisCol-32) * (FEXAlgoSpaceDefs::centralNphi)) + thisRow;
            tmp_gTowersIDs_subset[thisRow][thisCol] = towerid;
         }
      }

      // set the FCAL positive part
      for(int thisCol=36; thisCol<cols; thisCol++){
         for(int thisRow=0; thisRow<rows/2; thisRow++){
            int towerid = initialposFCAL + ((thisCol-36) * (FEXAlgoSpaceDefs::forwardNphi)) + thisRow;
            tmp_gTowersIDs_subset[thisRow][thisCol] = towerid;
         }
      }

      if(false){
         ATH_MSG_DEBUG("CONTENTS OF gFEX : ");
         for (int thisRow=rows-1; thisRow>=0; thisRow--){
            for (int thisCol=0; thisCol<cols; thisCol++){
               int tmptowerid = tmp_gTowersIDs_subset[thisRow][thisCol];
               const float tmptowereta = this_gTowerContainer->findTower(tmptowerid)->eta();
               const float tmptowerphi = this_gTowerContainer->findTower(tmptowerid)->phi();
               if(thisCol != cols-1){ ATH_MSG_DEBUG("|   " << tmptowerid << "([" << tmptowerphi << "][" << tmptowereta << "])   "); }
               else { ATH_MSG_DEBUG("|   " << tmptowerid << "([" << tmptowereta << "][" << tmptowerphi << "])   |"); }
            }
         }
      }

      ATH_CHECK(m_gFEXSimTool->executegFEXSim(tmp_gTowersIDs_subset, gFEXOutputs));
      
      const std::vector<uint32_t>& allgRhoTobs = m_gFEXSimTool->getgRhoTOBs();
      const std::vector<uint32_t>& allgBlockTobs = m_gFEXSimTool->getgBlockTOBs();
      const std::vector<uint32_t>& allgJetTobs = m_gFEXSimTool->getgJetTOBs();
       
      const std::vector<int32_t>& allgScalarEJwojTobs = m_gFEXSimTool->getgScalarEJwojTOBs();
      const std::vector<uint32_t>& allgMETComponentsJwojTobs = m_gFEXSimTool->getgMETComponentsJwojTOBs();
      const std::vector<uint32_t>& allgMHTComponentsJwojTobs = m_gFEXSimTool->getgMHTComponentsJwojTOBs();
      const std::vector<uint32_t>& allgMSTComponentsJwojTobs = m_gFEXSimTool->getgMSTComponentsJwojTOBs();

      const std::vector<uint32_t>& allgMETComponentsNoiseCutTobs = m_gFEXSimTool->getgMETComponentsNoiseCutTOBs();
      const std::vector<uint32_t>& allgMETComponentsRmsTobs = m_gFEXSimTool->getgMETComponentsRmsTOBs();
      const std::vector<uint32_t>& allgScalarENoiseCutTobs = m_gFEXSimTool->getgScalarENoiseCutTOBs();
      const std::vector<uint32_t>& allgScalarERmsTobs = m_gFEXSimTool->getgScalarERmsTOBs();

      m_gFEXSimTool->reset();

      //Makes containers for different gFEX Jet objects
      std::unique_ptr< xAOD::gFexJetRoIContainer > gRhoContainer = std::make_unique<xAOD::gFexJetRoIContainer> ();
      std::unique_ptr< xAOD::gFexJetRoIAuxContainer > gRhoAuxContainer = std::make_unique<xAOD::gFexJetRoIAuxContainer> ();
      gRhoContainer->setStore(gRhoAuxContainer.get());

      std::unique_ptr< xAOD::gFexJetRoIContainer > gBlockContainer = std::make_unique<xAOD::gFexJetRoIContainer> ();
      std::unique_ptr< xAOD::gFexJetRoIAuxContainer > gBlockAuxContainer = std::make_unique<xAOD::gFexJetRoIAuxContainer> ();
      gBlockContainer->setStore(gBlockAuxContainer.get());

      std::unique_ptr< xAOD::gFexJetRoIContainer > gJetContainer = std::make_unique<xAOD::gFexJetRoIContainer> ();
      std::unique_ptr< xAOD::gFexJetRoIAuxContainer > gJetAuxContainer = std::make_unique<xAOD::gFexJetRoIAuxContainer> ();
      gJetContainer->setStore(gJetAuxContainer.get());

      //Makes containers for different gFEX Global objects (for JwoJ algorithm quantities)
      std::unique_ptr< xAOD::gFexGlobalRoIContainer > gScalarEJwojContainer = std::make_unique<xAOD::gFexGlobalRoIContainer> ();
      std::unique_ptr< xAOD::gFexGlobalRoIAuxContainer > gScalarEJwojAuxContainer = std::make_unique<xAOD::gFexGlobalRoIAuxContainer> ();
      gScalarEJwojContainer->setStore(gScalarEJwojAuxContainer.get());

      std::unique_ptr< xAOD::gFexGlobalRoIContainer > gMETComponentsJwojContainer = std::make_unique<xAOD::gFexGlobalRoIContainer> ();
      std::unique_ptr< xAOD::gFexGlobalRoIAuxContainer > gMETComponentsJwojAuxContainer = std::make_unique<xAOD::gFexGlobalRoIAuxContainer> ();
      gMETComponentsJwojContainer->setStore(gMETComponentsJwojAuxContainer.get());

      std::unique_ptr< xAOD::gFexGlobalRoIContainer > gMHTComponentsJwojContainer = std::make_unique<xAOD::gFexGlobalRoIContainer> ();
      std::unique_ptr< xAOD::gFexGlobalRoIAuxContainer > gMHTComponentsJwojAuxContainer = std::make_unique<xAOD::gFexGlobalRoIAuxContainer> ();
      gMHTComponentsJwojContainer->setStore(gMHTComponentsJwojAuxContainer.get());

      std::unique_ptr< xAOD::gFexGlobalRoIContainer > gMSTComponentsJwojContainer = std::make_unique<xAOD::gFexGlobalRoIContainer> ();
      std::unique_ptr< xAOD::gFexGlobalRoIAuxContainer > gMSTComponentsJwojAuxContainer = std::make_unique<xAOD::gFexGlobalRoIAuxContainer> ();
      gMSTComponentsJwojContainer->setStore(gMSTComponentsJwojAuxContainer.get());

      //Makes containers for different gFEX Global objects (for Noise Cut and RMS algorithms quantities)
      std::unique_ptr< xAOD::gFexGlobalRoIContainer > gMETComponentsNoiseCutContainer = std::make_unique<xAOD::gFexGlobalRoIContainer> ();
      std::unique_ptr< xAOD::gFexGlobalRoIAuxContainer > gMETComponentsNoiseCutAuxContainer = std::make_unique<xAOD::gFexGlobalRoIAuxContainer> ();
      gMETComponentsNoiseCutContainer->setStore(gMETComponentsNoiseCutAuxContainer.get());
      
      std::unique_ptr< xAOD::gFexGlobalRoIContainer > gMETComponentsRmsContainer = std::make_unique<xAOD::gFexGlobalRoIContainer> ();
      std::unique_ptr< xAOD::gFexGlobalRoIAuxContainer > gMETComponentsRmsAuxContainer = std::make_unique<xAOD::gFexGlobalRoIAuxContainer> ();
      gMETComponentsRmsContainer->setStore(gMETComponentsRmsAuxContainer.get());

      std::unique_ptr< xAOD::gFexGlobalRoIContainer > gScalarENoiseCutContainer = std::make_unique<xAOD::gFexGlobalRoIContainer> ();
      std::unique_ptr< xAOD::gFexGlobalRoIAuxContainer > gScalarENoiseCutAuxContainer = std::make_unique<xAOD::gFexGlobalRoIAuxContainer> ();
      gScalarENoiseCutContainer->setStore(gScalarENoiseCutAuxContainer.get());

      std::unique_ptr< xAOD::gFexGlobalRoIContainer > gScalarERmsContainer = std::make_unique<xAOD::gFexGlobalRoIContainer> ();
      std::unique_ptr< xAOD::gFexGlobalRoIAuxContainer > gScalarERmsAuxContainer = std::make_unique<xAOD::gFexGlobalRoIAuxContainer> ();
      gScalarERmsContainer->setStore(gScalarERmsAuxContainer.get());


      // Retrieve the L1 menu configuration
      SG::ReadHandle<TrigConf::L1Menu> l1Menu (m_l1MenuKey,ctx);
      ATH_CHECK(l1Menu.isValid());

      auto & thr_gJ = l1Menu->thrExtraInfo().gJ();
      auto & thr_gLJ = l1Menu->thrExtraInfo().gLJ();
      auto & thr_gXE = l1Menu->thrExtraInfo().gXE();
      auto & thr_gTE = l1Menu->thrExtraInfo().gTE();

      int gJ_scale = thr_gJ.resolutionMeV();
      int gLJ_scale = thr_gLJ.resolutionMeV();
      int gXE_scale = thr_gXE.resolutionMeV();
      int gTE_scale = thr_gTE.resolutionMeV();


      //iterate over all gRho Tobs and fill EDM with them
      for(auto tob : allgRhoTobs){
	ATH_CHECK(fillgRhoEDM(gRhoContainer.get(), tob, gJ_scale));
      }
      //iterate over all gBlock Tobs and fill EDM with them
      for(auto tob : allgBlockTobs){
	ATH_CHECK(fillgBlockEDM(gBlockContainer.get(), tob, gJ_scale));
      }

      //iterate over all gJet Tobs and fill EDM with them
      for(auto tob : allgJetTobs){
	ATH_CHECK(fillgJetEDM(gJetContainer.get(), tob, gLJ_scale));
      }

      //iterate over all JwoJ scalar energy Tobs and fill EDM with them (should be only one)
      for(auto tob : allgScalarEJwojTobs){
	ATH_CHECK(fillgScalarEJwojEDM(gScalarEJwojContainer.get(), tob, gXE_scale, gTE_scale));
      }
      //iterate over all JwoJ METcomponents Tobs and fill EDM with them (should be only one)
      for(auto tob : allgMETComponentsJwojTobs){
	ATH_CHECK(fillgMETComponentsJwojEDM(gMETComponentsJwojContainer.get(), tob, gXE_scale, gXE_scale));
      }
      //iterate over all JwoJ MHTcomponents Tobs and fill EDM with them (should be only one)
      for(auto tob : allgMHTComponentsJwojTobs){
	ATH_CHECK(fillgMHTComponentsJwojEDM(gMHTComponentsJwojContainer.get(), tob, gXE_scale, gXE_scale));
      }
      //iterate over all JwoJ MSTcomponents Tobs and fill EDM with them (should be only one)
      for(auto tob : allgMSTComponentsJwojTobs){
	ATH_CHECK(fillgMSTComponentsJwojEDM(gMSTComponentsJwojContainer.get(), tob, gXE_scale, gXE_scale));
      }

      //iterate over all NoiseCut METcomponents Tobs and fill EDM with them (should be only one)
      for(auto tob : allgMETComponentsNoiseCutTobs){
	ATH_CHECK(fillgMETComponentsNoiseCutEDM(gMETComponentsNoiseCutContainer.get(), tob, gXE_scale, gXE_scale));
      }
      //iterate over all RMS METcomponents Tobs and fill EDM with them (should be only one)
      for(auto tob : allgMETComponentsRmsTobs){
	ATH_CHECK(fillgMETComponentsRmsEDM(gMETComponentsRmsContainer.get(), tob, gXE_scale, gXE_scale));
      }
      //iterate over all NoiseCut scalar energy Tobs and fill EDM with them (should be only one)
      for(auto tob : allgScalarENoiseCutTobs){
	ATH_CHECK(fillgScalarENoiseCutEDM(gScalarENoiseCutContainer.get(), tob, gXE_scale, gTE_scale));
      }
      //iterate over all RMS scalar energy Tobs and fill EDM with them (should be only one)
      for(auto tob : allgScalarERmsTobs){
	ATH_CHECK(fillgScalarERmsEDM(gScalarERmsContainer.get(), tob, gXE_scale, gTE_scale));
      }

      
      SG::WriteHandle<xAOD::gFexJetRoIContainer> outputgFexRhoHandle(m_gFexRhoOutKey,ctx);
      ATH_MSG_DEBUG("   write: " << outputgFexRhoHandle.key() << " = " << "..." );
      ATH_CHECK(outputgFexRhoHandle.record(std::move(gRhoContainer),std::move(gRhoAuxContainer)));

      SG::WriteHandle<xAOD::gFexJetRoIContainer> outputgFexBlockHandle(m_gFexBlockOutKey,ctx);
      ATH_MSG_DEBUG("   write: " << outputgFexBlockHandle.key() << " = " << "..." );
      ATH_CHECK(outputgFexBlockHandle.record(std::move(gBlockContainer),std::move(gBlockAuxContainer)));

      SG::WriteHandle<xAOD::gFexJetRoIContainer> outputgFexJetHandle(m_gFexJetOutKey,ctx);
      ATH_MSG_DEBUG("   write: " << outputgFexJetHandle.key() << " = " << "..." );
      ATH_CHECK(outputgFexJetHandle.record(std::move(gJetContainer),std::move(gJetAuxContainer)));

      SG::WriteHandle<xAOD::gFexGlobalRoIContainer> outputgScalarEJwojHandle(m_gScalarEJwojOutKey,ctx);
      ATH_MSG_DEBUG("   write: " << outputgScalarEJwojHandle.key() << " = " << "..." );
      ATH_CHECK(outputgScalarEJwojHandle.record(std::move(gScalarEJwojContainer),std::move(gScalarEJwojAuxContainer)));

      SG::WriteHandle<xAOD::gFexGlobalRoIContainer> outputgMETComponentsJwojHandle(m_gMETComponentsJwojOutKey,ctx);
      ATH_MSG_DEBUG("   write: " << outputgMETComponentsJwojHandle.key() << " = " << "..." );
      ATH_CHECK(outputgMETComponentsJwojHandle.record(std::move(gMETComponentsJwojContainer),std::move(gMETComponentsJwojAuxContainer)));

      SG::WriteHandle<xAOD::gFexGlobalRoIContainer> outputgMHTComponentsJwojHandle(m_gMHTComponentsJwojOutKey,ctx);
      ATH_MSG_DEBUG("   write: " << outputgMHTComponentsJwojHandle.key() << " = " << "..." );
      ATH_CHECK(outputgMHTComponentsJwojHandle.record(std::move(gMHTComponentsJwojContainer),std::move(gMHTComponentsJwojAuxContainer)));

      SG::WriteHandle<xAOD::gFexGlobalRoIContainer> outputgMSTComponentsJwojHandle(m_gMSTComponentsJwojOutKey,ctx);
      ATH_MSG_DEBUG("   write: " << outputgMSTComponentsJwojHandle.key() << " = " << "..." );
      ATH_CHECK(outputgMSTComponentsJwojHandle.record(std::move(gMSTComponentsJwojContainer),std::move(gMSTComponentsJwojAuxContainer)));

      SG::WriteHandle<xAOD::gFexGlobalRoIContainer> outputgMETComponentsNoiseCutHandle(m_gMETComponentsNoiseCutOutKey,ctx);
      ATH_MSG_DEBUG("   write: " << outputgMETComponentsNoiseCutHandle.key() << " = " << "..." );
      ATH_CHECK(outputgMETComponentsNoiseCutHandle.record(std::move(gMETComponentsNoiseCutContainer),std::move(gMETComponentsNoiseCutAuxContainer)));

      SG::WriteHandle<xAOD::gFexGlobalRoIContainer> outputgMETComponentsRmsHandle(m_gMETComponentsRmsOutKey,ctx);
      ATH_MSG_DEBUG("   write: " << outputgMETComponentsRmsHandle.key() << " = " << "..." );
      ATH_CHECK(outputgMETComponentsRmsHandle.record(std::move(gMETComponentsRmsContainer),std::move(gMETComponentsRmsAuxContainer)));

      SG::WriteHandle<xAOD::gFexGlobalRoIContainer> outputgScalarENoiseCutHandle(m_gScalarENoiseCutOutKey,ctx);
      ATH_MSG_DEBUG("   write: " << outputgScalarENoiseCutHandle.key() << " = " << "..." );
      ATH_CHECK(outputgScalarENoiseCutHandle.record(std::move(gScalarENoiseCutContainer),std::move(gScalarENoiseCutAuxContainer)));

      SG::WriteHandle<xAOD::gFexGlobalRoIContainer> outputgScalarERmsHandle(m_gScalarERmsOutKey,ctx);
      ATH_MSG_DEBUG("   write: " << outputgScalarERmsHandle.key() << " = " << "..." );
      ATH_CHECK(outputgScalarERmsHandle.record(std::move(gScalarERmsContainer),std::move(gScalarERmsAuxContainer)));

      return StatusCode::SUCCESS;
   }

  StatusCode gFEXSysSim::fillgRhoEDM(xAOD::gFexJetRoIContainer* gRhoContainer, uint32_t tobWord, int gJ_scale) const {

      std::unique_ptr<xAOD::gFexJetRoI> myEDM (new xAOD::gFexJetRoI());
      gRhoContainer->push_back(std::move(myEDM));
      gRhoContainer->back()->initialize(tobWord, gJ_scale);

      return StatusCode::SUCCESS;
   }

  StatusCode gFEXSysSim::fillgBlockEDM(xAOD::gFexJetRoIContainer* gBlockContainer, uint32_t tobWord, int gJ_scale) const {

      std::unique_ptr<xAOD::gFexJetRoI> myEDM (new xAOD::gFexJetRoI());
      gBlockContainer->push_back(std::move(myEDM));
      gBlockContainer->back()->initialize(tobWord, gJ_scale);

      return StatusCode::SUCCESS;
   }

  StatusCode gFEXSysSim::fillgJetEDM(xAOD::gFexJetRoIContainer* gJetContainer, uint32_t tobWord, int gLJ_scale) const {

      std::unique_ptr<xAOD::gFexJetRoI> myEDM (new xAOD::gFexJetRoI());
      gJetContainer->push_back(std::move(myEDM));
      gJetContainer->back()->initialize(tobWord, gLJ_scale);

      return StatusCode::SUCCESS;
   }

  StatusCode gFEXSysSim::fillgMETComponentsJwojEDM(xAOD::gFexGlobalRoIContainer* gMETComponentsJwojContainer, uint32_t tobWord, int scale1, int scale2) const {

      std::unique_ptr<xAOD::gFexGlobalRoI> myEDM (new xAOD::gFexGlobalRoI());
      gMETComponentsJwojContainer->push_back(std::move(myEDM));
      gMETComponentsJwojContainer->back()->initialize(tobWord, scale1, scale2);

      return StatusCode::SUCCESS;
   }

  StatusCode gFEXSysSim::fillgMHTComponentsJwojEDM(xAOD::gFexGlobalRoIContainer* gMHTComponentsJwojContainer, uint32_t tobWord, int scale1, int scale2) const {

      std::unique_ptr<xAOD::gFexGlobalRoI> myEDM (new xAOD::gFexGlobalRoI());
      gMHTComponentsJwojContainer->push_back(std::move(myEDM));
      gMHTComponentsJwojContainer->back()->initialize(tobWord, scale1, scale2);

      return StatusCode::SUCCESS;
   }

  StatusCode gFEXSysSim::fillgMSTComponentsJwojEDM(xAOD::gFexGlobalRoIContainer* gMSTComponentsJwojContainer, uint32_t tobWord, int scale1, int scale2) const {

      std::unique_ptr<xAOD::gFexGlobalRoI> myEDM (new xAOD::gFexGlobalRoI());
      gMSTComponentsJwojContainer->push_back(std::move(myEDM));
      gMSTComponentsJwojContainer->back()->initialize(tobWord, scale1, scale2);

      return StatusCode::SUCCESS;
   }

  StatusCode gFEXSysSim::fillgScalarEJwojEDM(xAOD::gFexGlobalRoIContainer* gScalarEJwojContainer, uint32_t tobWord, int scale1, int scale2) const {

      std::unique_ptr<xAOD::gFexGlobalRoI> myEDM (new xAOD::gFexGlobalRoI());
      gScalarEJwojContainer->push_back(std::move(myEDM));
      gScalarEJwojContainer->back()->initialize(tobWord, scale1, scale2);

      return StatusCode::SUCCESS;
   }

  StatusCode gFEXSysSim::fillgMETComponentsNoiseCutEDM(xAOD::gFexGlobalRoIContainer* gMETComponentsNoiseCutContainer, uint32_t tobWord, int scale1, int scale2) const {

      std::unique_ptr<xAOD::gFexGlobalRoI> myEDM (new xAOD::gFexGlobalRoI());
      gMETComponentsNoiseCutContainer->push_back(std::move(myEDM));
      gMETComponentsNoiseCutContainer->back()->initialize(tobWord, scale1, scale2);

      return StatusCode::SUCCESS;
   }

  StatusCode gFEXSysSim::fillgMETComponentsRmsEDM(xAOD::gFexGlobalRoIContainer* gMETComponentsRmsContainer, uint32_t tobWord, int scale1, int scale2) const {

      std::unique_ptr<xAOD::gFexGlobalRoI> myEDM (new xAOD::gFexGlobalRoI());
      gMETComponentsRmsContainer->push_back(std::move(myEDM));
      gMETComponentsRmsContainer->back()->initialize(tobWord, scale1, scale2);

      return StatusCode::SUCCESS;
   }

  StatusCode gFEXSysSim::fillgScalarENoiseCutEDM(xAOD::gFexGlobalRoIContainer* gScalarENoiseCutContainer, uint32_t tobWord, int scale1, int scale2) const {

      std::unique_ptr<xAOD::gFexGlobalRoI> myEDM (new xAOD::gFexGlobalRoI());
      gScalarENoiseCutContainer->push_back(std::move(myEDM));
      gScalarENoiseCutContainer->back()->initialize(tobWord, scale1, scale2);

      return StatusCode::SUCCESS;
   }

  StatusCode gFEXSysSim::fillgScalarERmsEDM(xAOD::gFexGlobalRoIContainer* gScalarERmsContainer, uint32_t tobWord, int scale1, int scale2) const {

      std::unique_ptr<xAOD::gFexGlobalRoI> myEDM (new xAOD::gFexGlobalRoI());
      gScalarERmsContainer->push_back(std::move(myEDM));
      gScalarERmsContainer->back()->initialize(tobWord, scale1, scale2);

      return StatusCode::SUCCESS;
   }


} // end of namespace bracket
