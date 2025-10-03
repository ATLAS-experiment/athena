/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LUCID_HIT_ANALYSIS_H
#define LUCID_HIT_ANALYSIS_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "LUCID_SimEvent/LUCID_SimHitCollection.h"
#include "StoreGate/ReadHandleKey.h"

class LucidHitAnalysis : public AthHistogramAlgorithm {

 public:

  using AthHistogramAlgorithm::AthHistogramAlgorithm; 
  ~LucidHitAnalysis() = default;

  virtual StatusCode initialize() override final;
  virtual StatusCode execute() override final;

 private:

   /** Some histograms**/
   TH1* m_h_hit_x{nullptr};
   TH1* m_h_hit_y{nullptr};
   TH1* m_h_hit_z{nullptr};
   TH2* m_h_xy{nullptr};
   TH2* m_h_zr{nullptr};
   TH1* m_h_hit_post_x{nullptr};
   TH1* m_h_hit_post_y{nullptr};
   TH1* m_h_hit_post_z{nullptr};
   TH1* m_h_hit_edep{nullptr};
   TH1* m_h_hit_pdgid{nullptr};
   TH1* m_h_hit_pretime{nullptr};
   TH1* m_h_hit_posttime{nullptr};
   TH1* m_h_genvolume{nullptr};
   TH1* m_h_wavelength{nullptr};

   std::vector<float>* m_hit_x{nullptr};
   std::vector<float>* m_hit_y{nullptr};
   std::vector<float>* m_hit_z{nullptr};
   std::vector<float>* m_hit_post_x{nullptr};
   std::vector<float>* m_hit_post_y{nullptr};
   std::vector<float>* m_hit_post_z{nullptr};
   std::vector<float>* m_hit_edep{nullptr};
   std::vector<float>* m_hit_pdgid{nullptr};
   std::vector<float>* m_hit_pretime{nullptr};
   std::vector<float>* m_hit_posttime{nullptr};
   std::vector<float>* m_gen_volume{nullptr};
   std::vector<float>* m_wavelength{nullptr};
   
   TTree * m_tree{nullptr};

   StringProperty m_ntupleFileName{this, "NtupleFileName", "/LucidHitAnalysis/"};
   StringProperty m_path{this, "HistPath", "/LucidHitAnalysis/"};
   SG::ReadHandleKey<LUCID_SimHitCollection> m_readKey{this, "InputKey", "LucidSimHitsVector"};

};

#endif // LUCID_HIT_ANALYSIS_H
