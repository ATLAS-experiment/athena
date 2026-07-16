/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//    gFEXSim - Simulation of the gFEX module
//                              -------------------
//     begin                : 01 04 2021
//     email                : cecilia.tosciri@cern.ch
//***************************************************************************

#include "gFEXSim.h"
#include "L1CaloFEXSim/gTower.h"
#include "gFEXFPGA.h"
#include "L1CaloFEXSim/gTowerContainer.h"
#include "gFEXJetAlgo.h"
#include "L1CaloFEXSim/gFEXJetTOB.h"
#include "L1CaloFEXSim/gFEXOutputCollection.h"

namespace LVL1 {

   gFEXSim::gFEXSim(const std::string& type,const std::string& name,const IInterface* parent):
      AthAlgTool(type,name,parent)
   {
      declareInterface<IgFEXSim>(this);
   }


   /** Destructor */
   gFEXSim::~gFEXSim(){
   }

   StatusCode gFEXSim::initialize(){
      ATH_CHECK( m_gFEXFPGA_Tool.retrieve() );
      ATH_CHECK( m_gFEXJetAlgoTool.retrieve() );
      ATH_CHECK( m_gFEXJwoJAlgoTool.retrieve() );
      ATH_CHECK( m_gFEXaltMetAlgoTool.retrieve() );
      ATH_CHECK(m_l1MenuKey.initialize());
      ATH_CHECK(m_gTowersWriteKey.initialize());

      return StatusCode::SUCCESS;
   }


 StatusCode gFEXSim::execute(const EventContext& ctx,
			     const gTowersIDs& tmp_gTowersIDs_subset,
			     gFEXOutputCollection* gFEXOutputs,
			     std::vector<uint32_t>& gRhoTobWords,
			     std::vector<uint32_t>& gBlockTobWords,
			     std::vector<uint32_t>& gJetTobWords,
			     std::vector<int32_t>&  gScalarEJwojTobWords,
			     std::vector<uint32_t>& gMETComponentsJwojTobWords,
			     std::vector<uint32_t>& gMHTComponentsJwojTobWords,
			     std::vector<uint32_t>& gMSTComponentsJwojTobWords,
			     std::vector<uint32_t>& gMETComponentsNoiseCutTobWords,
			     std::vector<uint32_t>& gMETComponentsRmsTobWords,
			     std::vector<uint32_t>& gScalarENoiseCutTobWords,
			     std::vector<uint32_t>& gScalarERmsTobWords) const {

   // Container to save gTowers
   SG::WriteHandle<xAOD::gFexTowerContainer> gTowersContainer(m_gTowersWriteKey, ctx);
   ATH_CHECK(gTowersContainer.record(std::make_unique<xAOD::gFexTowerContainer>(), std::make_unique<xAOD::gFexTowerAuxContainer>()));
   ATH_MSG_DEBUG("Recorded gFexTriggerTower container with key " << gTowersContainer.key());

   gTowersType Atwr = {{{0}}};
   gTowersType Btwr = {{{0}}};
   gTowersType Ctwr = {{{0}}};

   gTowersType Atwr50 = {{{0}}};
   gTowersType Btwr50 = {{{0}}};
   gTowersType Ctwr50 = {{{0}}};

   gTowersType Asat = {{{0}}};
   gTowersType Bsat = {{{0}}};
   gTowersType Csat = {{{0}}};


   //FPGA A
   gTowersCentral tmp_gTowersIDs_subset_centralFPGA;
   memset(&tmp_gTowersIDs_subset_centralFPGA, 0, sizeof tmp_gTowersIDs_subset_centralFPGA);
   for (int myrow = 0; myrow<FEXAlgoSpaceDefs::centralNphi; myrow++){
      for (int mycol = 0; mycol<12; mycol++){
         tmp_gTowersIDs_subset_centralFPGA[myrow][mycol] = tmp_gTowersIDs_subset[myrow][mycol+8];
      }
   }

   m_gFEXFPGA_Tool->FillgTowerEDMCentral(ctx, gTowersContainer, 0, tmp_gTowersIDs_subset_centralFPGA, Atwr, Atwr50, Asat);

   //FPGA B
   gTowersCentral tmp_gTowersIDs_subset_centralFPGA_B;
   memset(&tmp_gTowersIDs_subset_centralFPGA_B, 0, sizeof tmp_gTowersIDs_subset_centralFPGA_B);
   for (int myrow = 0; myrow<FEXAlgoSpaceDefs::centralNphi; myrow++){
      for (int mycol = 0; mycol<12; mycol++){
         tmp_gTowersIDs_subset_centralFPGA_B[myrow][mycol] = tmp_gTowersIDs_subset[myrow][mycol+20];
      }
   }

   m_gFEXFPGA_Tool->FillgTowerEDMCentral(ctx, gTowersContainer, 1, tmp_gTowersIDs_subset_centralFPGA_B,  Btwr, Btwr50, Bsat);

   //FPGA C

   // C-N
   //Use a matrix with 32 rows, even if FPGA-N (negative) also deals with regions of 16 bins in phi (those connected to FCAL).
   //We have 4 columns with 32 rows and 4 columns with 16 rows for each FPGA-C.
   //So we use a matrix 32x8 but we fill only half of it in the region 3.3<|eta|<4.8.
   gTowersForward tmp_gTowersIDs_subset_forwardFPGA_N;
   memset(&tmp_gTowersIDs_subset_forwardFPGA_N, 0, sizeof tmp_gTowersIDs_subset_forwardFPGA_N);
   for (int myrow = 0; myrow<FEXAlgoSpaceDefs::forwardNphi; myrow++){
      for (int mycol = 0; mycol<4; mycol++){
         tmp_gTowersIDs_subset_forwardFPGA_N[myrow][mycol] = tmp_gTowersIDs_subset[myrow][mycol];
      }
   }
   for (int myrow = 0; myrow<FEXAlgoSpaceDefs::centralNphi; myrow++){
      for (int mycol = 4; mycol<8; mycol++){
         tmp_gTowersIDs_subset_forwardFPGA_N[myrow][mycol] = tmp_gTowersIDs_subset[myrow][mycol];
      }
   }

   // C-P
   //Use a matrix with 32 rows, even if FPGA-C (positive) also deals with regions of 16 bins in phi (those connected to FCAL).
   //We have 4 columns with 32 rows and 4 columns with 16 rows for each FPGA-C.
   //So we use a matrix 32x8 but we fill only half of it in the region 3.3<|eta|<4.8.
   gTowersForward tmp_gTowersIDs_subset_forwardFPGA_P;
   memset(&tmp_gTowersIDs_subset_forwardFPGA_P, 0, sizeof tmp_gTowersIDs_subset_forwardFPGA_P);
   for (int myrow = 0; myrow<FEXAlgoSpaceDefs::centralNphi; myrow++){
      for (int mycol = 0; mycol<4; mycol++){
         tmp_gTowersIDs_subset_forwardFPGA_P[myrow][mycol] = tmp_gTowersIDs_subset[myrow][mycol+32];
      }
   }
   for (int myrow = 0; myrow<FEXAlgoSpaceDefs::forwardNphi; myrow++){
      for (int mycol = 4; mycol<8; mycol++){
         tmp_gTowersIDs_subset_forwardFPGA_P[myrow][mycol] = tmp_gTowersIDs_subset[myrow][mycol+32];
      }
   }

   m_gFEXFPGA_Tool->FillgTowerEDMForward(ctx, gTowersContainer, 2, tmp_gTowersIDs_subset_forwardFPGA_N, tmp_gTowersIDs_subset_forwardFPGA_P, Ctwr, Ctwr50, Csat);


   // Retrieve the L1 menu configuration
   SG::ReadHandle<TrigConf::L1Menu> l1Menu (m_l1MenuKey, ctx);
   ATH_CHECK(l1Menu.isValid());

   //Parameters related to gLJ (large-R jet objects - gJet)
   auto & thr_gLJ = l1Menu->thrExtraInfo().gLJ();
   int gLJ_seedThrA = thr_gLJ.seedThrCounts('A'); //defined in GeV by default
   int gLJ_seedThrB = thr_gLJ.seedThrCounts('B'); //defined in GeV by default
   int gLJ_seedThrC = thr_gLJ.seedThrCounts('C'); //defined in GeV by default

   int gLJ_ptMinToTopoCounts1 = thr_gLJ.ptMinToTopoCounts(1);
   int gLJ_ptMinToTopoCounts2 = thr_gLJ.ptMinToTopoCounts(2);
 
   float gLJ_rhoMaxA = (thr_gLJ.rhoTowerMax('A')*1000)/50;//Values are given in GeV, need to be converted with 50MeV scale to be used in PU calculation
   float gLJ_rhoMaxB = (thr_gLJ.rhoTowerMax('B')*1000)/50;//Values are given in GeV, need to be converted with 50MeV scale to be used in PU calculation
   float gLJ_rhoMaxC = (thr_gLJ.rhoTowerMax('C')*1000)/50;//Values are given in GeV, need to be converted with 50MeV scale to be used in PU calculation


   //Parameters related to gJ (small-R jet objects - gBlock)
   auto & thr_gJ = l1Menu->thrExtraInfo().gJ();
   int gJ_ptMinToTopoCounts1 = thr_gJ.ptMinToTopoCounts(1);
   int gJ_ptMinToTopoCounts2 = thr_gJ.ptMinToTopoCounts(2);


   int pucA = 0;
   int pucB = 0;
   int pucC = 0;
   int pucA_JWJ = 0;
   int pucB_JWJ = 0;
   int pucC_JWJ = 0;
   //note that jetThreshold is not a configurable parameter in firmware, it is used to check that jet values are positive
   int jetThreshold = FEXAlgoSpaceDefs::jetThr; //this threshold is set by the online software 

   if (FEXAlgoSpaceDefs::ENABLE_PUC == true){
      m_gFEXJetAlgoTool->pileUpCalculation(Atwr50, gLJ_rhoMaxA, 1, pucA, pucA_JWJ);
      m_gFEXJetAlgoTool->pileUpCalculation(Btwr50, gLJ_rhoMaxB, 1, pucB, pucB_JWJ);
      m_gFEXJetAlgoTool->pileUpCalculation(Ctwr50, gLJ_rhoMaxC, 1, pucC, pucC_JWJ);
   }
   
   

   // The output TOBs, to be filled by the gFEXJetAlgoTool
   std::array<uint32_t, 7> ATOB1_dat = {0};
   std::array<uint32_t, 7> ATOB2_dat = {0};
   std::array<uint32_t, 7> BTOB1_dat = {0};
   std::array<uint32_t, 7> BTOB2_dat = {0};
   std::array<uint32_t, 7> CTOB1_dat = {0};
   std::array<uint32_t, 7> CTOB2_dat = {0};


   // Pass the energy matrices to the algo tool, and run the algorithms
   auto tobs_v = m_gFEXJetAlgoTool->largeRfinder(Atwr, Btwr, Ctwr, Asat, Bsat, Csat, pucA, pucB, pucC,
                                                 gLJ_seedThrA, gLJ_seedThrB, gLJ_seedThrC, gJ_ptMinToTopoCounts1, gJ_ptMinToTopoCounts2, 
                                                 jetThreshold, gLJ_ptMinToTopoCounts1, gLJ_ptMinToTopoCounts2,
                                                 ATOB1_dat, ATOB2_dat,
                                                 BTOB1_dat, BTOB2_dat,
                                                 CTOB1_dat, CTOB2_dat);

   gRhoTobWords.resize(3);
   gBlockTobWords.resize(12);
   gJetTobWords.resize(6);

   gRhoTobWords[0] = ATOB2_dat[0];//Pile up correction A
   gRhoTobWords[1] = BTOB2_dat[0];//Pile up correction B
   gRhoTobWords[2] = CTOB2_dat[0];//Pile up correction C

   //Placing the gBlock TOBs into a dedicated array
   gBlockTobWords[0] = ATOB1_dat[1];//leading gBlock in FPGA A, eta bins (0--5)
   gBlockTobWords[1] = ATOB2_dat[1];//leading gBlock in FPGA A, eta bins (6--11)
   gBlockTobWords[2] = BTOB1_dat[1];//leading gBlock in FPGA B, eta bins (0--5)
   gBlockTobWords[3] = BTOB2_dat[1];//leading gBlock in FPGA B, eta bins (6--11)

   gBlockTobWords[4] = ATOB1_dat[2];//subleading gBlock in FPGA A, eta bins (0--5)
   gBlockTobWords[5] = ATOB2_dat[2];//subleading gBlock in FPGA A, eta bins (6--11)
   gBlockTobWords[6] = BTOB1_dat[2];//subleading gBlock in FPGA B, eta bins (0--5)
   gBlockTobWords[7] = BTOB2_dat[2];//subleading gBlock in FPGA B, eta bins (6--11)

   gBlockTobWords[8] = CTOB1_dat[1];//leading gBlock in FPGA C, eta negative
   gBlockTobWords[9] = CTOB2_dat[1];//leading gBlock in FPGA C, eta positive
   gBlockTobWords[10] = CTOB1_dat[2];//sub-leading gBlock in FPGA C, eta negative
   gBlockTobWords[11] = CTOB2_dat[2];//sub-leading gBlock in FPGA C, eta positive

   //Placing the gJet TOBs into a dedicated array
   gJetTobWords[0] = ATOB1_dat[3];//leading gJet in FPGA A, eta bins (0--5)
   gJetTobWords[1] = ATOB2_dat[3];//leading gJet in FPGA A, eta bins (6--11)
   gJetTobWords[2] = BTOB1_dat[3];//leading gJet in FPGA B, eta bins (0--5)
   gJetTobWords[3] = BTOB2_dat[3];//leading gJet in FPGA B, eta bins (6--11)
   gJetTobWords[4] = CTOB1_dat[3];//leading gJet in FPGA C negative
   gJetTobWords[5] = CTOB2_dat[3];//leading gJet in FPGA C positive


   // Use the gFEXJetAlgoTool
   std::array<int32_t, 4> outJwojTOB = {0};
   std::array<uint32_t, 4> outAltMetTOB = {0};

   // run the JwoJ algorithm
   auto global_tobs = m_gFEXJwoJAlgoTool->jwojAlgo(ctx, Atwr, pucA_JWJ, Btwr, pucB_JWJ, Ctwr, pucC_JWJ, outJwojTOB);

   gScalarEJwojTobWords.resize(1);
   gMETComponentsJwojTobWords.resize(1);
   gMHTComponentsJwojTobWords.resize(1);
   gMSTComponentsJwojTobWords.resize(1);


   //Placing the global TOBs into a dedicated array
   gScalarEJwojTobWords[0] = outJwojTOB[0];//
   gMETComponentsJwojTobWords[0] = outJwojTOB[1];//
   gMHTComponentsJwojTobWords[0] = outJwojTOB[2];//
   gMSTComponentsJwojTobWords[0] = outJwojTOB[3];//

   // run the altMet algorithm
   m_gFEXaltMetAlgoTool->altMetAlgo(ctx, Atwr, Btwr, Ctwr, outAltMetTOB);

   gMETComponentsNoiseCutTobWords.resize(1);
   gMETComponentsRmsTobWords.resize(1);
   gScalarENoiseCutTobWords.resize(1);
   gScalarERmsTobWords.resize(1);

   //Placing the global TOBs into a dedicated array
   gMETComponentsNoiseCutTobWords[0] = outAltMetTOB[0];//
   gMETComponentsRmsTobWords[0] = outAltMetTOB[1];//
   gScalarENoiseCutTobWords[0] = outAltMetTOB[2];//
   gScalarERmsTobWords[0] = outAltMetTOB[3];//
   
   for (int i = 0; i <14; i++){
     gFEXOutputs->addJetTob(tobs_v[i]->getWord());
     gFEXOutputs->addValueJet("EtaJet", tobs_v[i]->getEta());
     gFEXOutputs->addValueJet("PhiJet", tobs_v[i]->getPhi());
     gFEXOutputs->addValueJet("ETJet", tobs_v[i]->getET());
     gFEXOutputs->addValueJet("StatusJet", tobs_v[i]->getStatus());
     gFEXOutputs->addValueJet("TobIDJet", tobs_v[i]->getTobID());
     gFEXOutputs->fillJet();

   }

   for (int i = 0; i <4; i++){
     gFEXOutputs->addGlobalTob(global_tobs[i]->getWord());
     gFEXOutputs->addValueGlobal("GlobalQuantity1", global_tobs[i]->getQuantity1());
     gFEXOutputs->addValueGlobal("GlobalQuantity2", global_tobs[i]->getQuantity2());
     gFEXOutputs->addValueGlobal("SaturationGlobal", global_tobs[i]->getSaturation());
     gFEXOutputs->addValueGlobal("TobIDGlobal", global_tobs[i]->getTobID());
     gFEXOutputs->addValueGlobal("GlobalStatus1", global_tobs[i]->getStatus1());
     gFEXOutputs->addValueGlobal("GlobalStatus2", global_tobs[i]->getStatus2());
     gFEXOutputs->fillGlobal();

   }

    return StatusCode::SUCCESS;
}


} // end of namespace bracket
