/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */
#include "WTAConeJetMaker.h"
#include "WTAConeParallelHelper.h"
#include <iostream>

 
 std::vector<Gep::Jet> Gep::WTAConeJetMaker::makeJets(const std::vector<Gep::Cluster>& inTopoTowers) const
 { // Makesure the WTA parameters are set before calling makeJets()
 
   std::vector<WTATrigObj> input_towers;
   const unsigned int inTopoTowersN = inTopoTowers.size();
   for(unsigned int i = 0; i < inTopoTowersN; i++)
   {
    const auto & TopoTower = inTopoTowers[i];
    #ifndef FLOATING_POINT_SIMULATION
     WTATrigObj this_tower = fTower_to_iTower(TopoTower, i);
    #else
     WTATrigObj this_tower(TopoTower.vec.Pt(), TopoTower.vec.Eta(), TopoTower.vec.Phi(), TopoTower.vec.M(), i); // Floating can take the raw values
    #endif
     input_towers.push_back(this_tower);
   }
 
   std::unique_ptr<WTAConeMaker> MyWTAConeMaker = CreateWTAConeMaker(static_cast<WTAConeMakerEnum>(m_SeedCleaningAlgo));
   if(auto* MyWTACone2PassMaker = dynamic_cast<WTACone2PassMaker*>(MyWTAConeMaker.get()))MyWTACone2PassMaker->SetRollOffBufferSize(m_RollOffBufferSize); // Only for two-pass algorithm
   std::vector<WTAJet> WTAJetList;
   MyWTAConeMaker->m_WTAConeMakerParameter = m_GEPWTAParameters; // Pass the WTAConeParameters
   if(m_BlockN != 1)
   { // Run parallel WTAConeJets
     WTAConeParallelHelper wta_parallel_helper;
     wta_parallel_helper.SetBlockN(m_BlockN);
     wta_parallel_helper.CreateBlocks(input_towers);
     wta_parallel_helper.RunParallelWTA(MyWTAConeMaker);
     wta_parallel_helper.CheckJetInCore();
     WTAJetList = wta_parallel_helper.GetAllJets();
   }
   else
   { // Run over whole calorimeter
     MyWTAConeMaker->InitiateInputs(input_towers);
     MyWTAConeMaker->SeedCleaning();
     MyWTAConeMaker->MergeConstsToSeeds();
     MyWTAConeMaker->CreateERingInfo();
     WTAJetList = MyWTAConeMaker->GetSeedList();
   }
 
   std::vector<Gep::Jet> GepJetList;
   for(const auto& WTAJet: WTAJetList)
   {
     Gep::Jet thisjet;
     #ifndef FLOATING_POINT_SIMULATION
     thisjet = iJet_to_fJet(WTAJet);
     #else
     thisjet.vec.SetPtEtaPhiM(WTAJet.pt(), WTAJet.eta(), WTAJet.phi(), WTAJet.m()); // Floating can take the raw values
      // Store ERing information
      WTA4JetERingInfo ering_info = WTAJet.GetERingInfo();
      thisjet.ring0_Et = ering_info.ring0_Et;
      thisjet.ring1_Et = ering_info.ring1_Et;
      thisjet.ring2_Et = ering_info.ring2_Et;
      thisjet.ring3_Et = ering_info.ring3_Et;
      thisjet.ring4_Et = ering_info.ring4_Et;
      thisjet.total_TobN = ering_info.total_TobN;
      thisjet.ring0_TobN = ering_info.ring0_TobN;
      thisjet.ring1_TobN = ering_info.ring1_TobN;
      thisjet.ring2_TobN = ering_info.ring2_TobN;
      thisjet.ring3_TobN = ering_info.ring3_TobN;
      thisjet.ring4_TobN = ering_info.ring4_TobN;
     #endif
    //  WTAJet.PrintERingInfo(); // Debug ERing TobN and Et
     thisjet.nConstituents = WTAJet.GetConstituentCount();
     thisjet.seedEt = WTAJet.GetSeed().pt();
     thisjet.seedEta = WTAJet.GetSeed().eta();
     thisjet.seedPhi = WTAJet.GetSeed().phi();
     const std::vector<WTATrigObj> ConstituentList = WTAJet.GetConstituentList();
     thisjet.constituentsIndices.clear();
     for(const auto& constituent: ConstituentList)
     {
      thisjet.constituentsIndices.push_back(constituent.idx());
     }

     GepJetList.push_back(std::move(thisjet));
   }
 
   return GepJetList;
 }
 