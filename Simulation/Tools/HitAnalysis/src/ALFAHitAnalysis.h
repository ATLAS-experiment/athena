/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ALFA_HIT_ANALYSIS_H
#define ALFA_HIT_ANALYSIS_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "CxxUtils/ArrayHelper.h"
#include "ALFA_SimEv/ALFA_HitCollection.h"
#include "StoreGate/ReadHandleKey.h"

class ALFAHitAnalysis : public AthHistogramAlgorithm {

 public:

   using AthHistogramAlgorithm::AthHistogramAlgorithm;
   ~ALFAHitAnalysis() = default;

   virtual StatusCode initialize();
   virtual StatusCode execute();

 private:

   /** Some variables**/
   std::array<TH1*, 8> m_h_E_full_sum_h{make_array<TH1*, 8>(nullptr)};
   std::array<TH1*, 8> m_h_E_layer_sum_h{make_array<TH1*, 8>(nullptr)};
   std::array<TH1*, 8> m_h_hit_layer{make_array<TH1*, 8>(nullptr)};
   std::array<TH1*, 8> m_h_hit_fiber{make_array<TH1*, 8>(nullptr)};
   
   std::vector<int>* m_station{nullptr};
   std::vector<int>* m_plate{nullptr};
   std::vector<int>* m_fiber{nullptr};
   std::vector<int>* m_sign{nullptr};
   std::vector<double>* m_energy{nullptr};

   
   TTree * m_tree{nullptr};
   Gaudi::Property<std::string> m_ntupleFileName{this, "NtupleFileName", "/AFPHitAnalysis/" };
   Gaudi::Property<std::string> m_path{this, "HistPath", "/AFPHitAnalysis/"}; 
   SG::ReadHandleKey<ALFA_HitCollection> m_readKey{this, "InpuKey", "ALFA_HitCollection"};

};

#endif // ALFA_HIT_ANALYSIS_H
