/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
#include "WTAConeJetMaker.h"
#include "WTAConeParallelHelper.h"

 
 std::vector<Gep::Jet> Gep::WTAConeJetMaker::makeJets(const std::vector<Gep::Cluster>& inTopoTowers) const
 { // Makesure the WTA parameters are set before calling makeJets()
 
   std::vector<WTATrigObj> input_towers;
   for(const auto &TopoTower: inTopoTowers)
   {
     WTATrigObj this_tower(TopoTower.vec.Pt(), TopoTower.vec.Eta(), TopoTower.vec.Phi(), TopoTower.vec.M());
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
     WTAJetList = MyWTAConeMaker->GetSeedList();
   }
 
   std::vector<Gep::Jet> GepJetList;
   for(const auto& WTAJet: WTAJetList)
   {
     Gep::Jet thisjet;
     thisjet.vec.SetPtEtaPhiM(WTAJet.pt(), WTAJet.eta(), WTAJet.phi(), WTAJet.m());
     thisjet.nConstituents = WTAJet.GetConstituentCount();
     GepJetList.push_back(thisjet);
   }
 
   return GepJetList;
 }
 