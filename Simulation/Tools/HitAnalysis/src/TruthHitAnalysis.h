/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRUTH_HIT_ANALYSIS_H
#define TRUTH_HIT_ANALYSIS_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "GeneratorObjects/McEventCollection.h"



class TruthHitAnalysis : public AthHistogramAlgorithm {

 public:

  using AthHistogramAlgorithm::AthHistogramAlgorithm;
  ~TruthHitAnalysis() = default;
  virtual StatusCode execute() override final;
  virtual StatusCode initialize() override final;

 private:

   /** Some variables**/ 
   TH1* m_h_n_vert{nullptr};
   TH1* m_h_n_part{nullptr};
   TH1* m_h_n_vert_prim{nullptr};
   TH1* m_h_n_part_prim{nullptr};
   TH1* m_h_n_vert_sec{nullptr};
   TH1* m_h_n_part_sec{nullptr};
   TH1* m_h_vtx_x{nullptr};
   TH1* m_h_vtx_y{nullptr};
   TH1* m_h_vtx_z{nullptr};
   TH1* m_h_vtx_r{nullptr};
   TH2* m_h_vtx_prim_xy{nullptr};
   TH2* m_h_vtx_prim_zr{nullptr};
   TH2* m_h_vtx_sec_xy{nullptr};
   TH2* m_h_vtx_sec_zr{nullptr};
   TH1* m_h_n_generations{nullptr};
   TH1* m_h_truth_px{nullptr};
   TH1* m_h_truth_py{nullptr}; 
   TH1* m_h_truth_pz{nullptr};
   TH1* m_h_truth_pt{nullptr};
   TH1* m_h_truth_eta{nullptr};
   TH1* m_h_truth_phi{nullptr}; 
   TH1* m_h_barcode{nullptr};
   TH1* m_h_part_status{nullptr};
   TH1* m_h_part_pdgid{nullptr};
   TH1* m_h_part_pdgid_sec{nullptr};
   TH1* m_h_part_eta{nullptr};
   TH1* m_h_part_phi{nullptr};
   TH1* m_h_part_p{nullptr};

   std::vector<float>* m_vtx_x{nullptr};
   std::vector<float>* m_vtx_y{nullptr};
   std::vector<float>* m_vtx_z{nullptr};
   std::vector<float>* m_vtx_r{nullptr};
   std::vector<float>* m_vtx_barcode{nullptr};
   std::vector<float>* m_truth_px{nullptr};
   std::vector<float>* m_truth_py{nullptr};
   std::vector<float>* m_truth_pz{nullptr};
   std::vector<float>* m_truth_pt{nullptr};
   std::vector<float>* m_truth_eta{nullptr};
   std::vector<float>* m_truth_phi{nullptr};
   std::vector<float>* m_barcode{nullptr};
   std::vector<float>* m_status{nullptr};
   std::vector<float>* m_pdgid{nullptr};

   TTree * m_tree{nullptr};

   Gaudi::Property<std::string> m_path{this, "HistPath","/TruthHitAnalysis/"};
   Gaudi::Property<std::string> m_ntupleFileName{this, "NtupleFileName","/TruthHitAnalysis/"}; 
   SG::ReadHandleKey<McEventCollection> m_readKey{this, "InputKey", "TruthEvent"};

};

#endif // TRUTH_HIT_ANALYSIS_H
