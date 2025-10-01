/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACK_RECORD_ANALYSIS_H
#define TRACK_RECORD_ANALYSIS_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "TrackRecord/TrackRecordCollection.h"
#include "StoreGate/ReadHandleKey.h"


class TrackRecordAnalysis : public AthHistogramAlgorithm {

 public:
   using AthHistogramAlgorithm::AthHistogramAlgorithm;
   ~TrackRecordAnalysis() = default;

   virtual StatusCode initialize()override;
   virtual StatusCode execute()override;

 private:
   
   /** Some variables**/
   TH1* m_h_hits_x{nullptr};
   TH1* m_h_hits_y{nullptr};
   TH1* m_h_hits_z{nullptr};
   TH1* m_h_hits_r{nullptr};
   TH2* m_h_xy{nullptr};
   TH2* m_h_zr{nullptr};
   TH1* m_h_hits_eta{nullptr};
   TH1* m_h_hits_phi{nullptr};
   TH1* m_h_hits_px{nullptr};
   TH1* m_h_hits_py{nullptr};
   TH1* m_h_hits_pz{nullptr};
   TH1* m_h_hits_pt{nullptr};
   TH1* m_h_time{nullptr};
   TH1* m_h_edep{nullptr};
   TH1* m_h_pdg{nullptr};

   std::vector<float>* m_x{nullptr};
   std::vector<float>* m_y{nullptr};
   std::vector<float>* m_z{nullptr};
   std::vector<float>* m_r{nullptr};
   std::vector<float>* m_eta{nullptr};
   std::vector<float>* m_phi{nullptr};
   std::vector<float>* m_px{nullptr};
   std::vector<float>* m_py{nullptr};
   std::vector<float>* m_pz{nullptr};
   std::vector<float>* m_pt{nullptr};
   std::vector<float>* m_time{nullptr};
   std::vector<float>* m_edep{nullptr};
   std::vector<float>* m_pdg{nullptr};

   SG::ReadHandleKey<TrackRecordCollection> m_readKey{this, "CollectionName", "CaloEntryLayer"};
        
   TTree * m_tree{nullptr};
   Gaudi::Property<std::string> m_ntupleFileName{this, "NtupleFileName", "/TrackRecordAnalysis/"}; 
   Gaudi::Property<std::string> m_path{this, "HistPath", "/TrackRecordAnalysis/"}; 

};

#endif // TRACK_RECORD_ANALYSIS_H
