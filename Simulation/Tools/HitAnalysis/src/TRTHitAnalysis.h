/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HITANALYSIS_TRTHITANALYSIS_H
#define HITANALYSIS_TRTHITANALYSIS_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "InDetSimEvent/TRTUncompressedHitCollection.h"
#include "StoreGate/ReadHandleKey.h"

namespace InDetDD {
  class TRT_DetectorManager;
}

class TRTHitAnalysis : public AthHistogramAlgorithm {

 public:

    using AthHistogramAlgorithm::AthHistogramAlgorithm;  
   virtual StatusCode initialize() override;
   virtual StatusCode execute() override;

 private:

   /** Some variables**/
   TH1* m_h_TRT_y{nullptr};
   TH1* m_h_TRT_x{nullptr};
   TH1* m_h_TRT_z{nullptr};
   TH1* m_h_TRT_r{nullptr};
   TH2* m_h_TRT_xy{nullptr};
   TH2* m_h_TRT_zr{nullptr};
   TH1* m_h_TRT_time_photons{nullptr};
   TH1* m_h_TRT_time_nonphotons{nullptr};
   TH1* m_h_TRT_edep_photons{nullptr};
   TH1* m_h_TRT_edep_nonphotons{nullptr};
   TH1* m_h_TRT_kine_photons{nullptr};
   TH1* m_h_TRT_kine_nonphotons{nullptr};
   TH1* m_h_TRT_barcode{nullptr};

   std::vector<float>* m_TRT_x{nullptr};
   std::vector<float>* m_TRT_y{nullptr};
   std::vector<float>* m_TRT_z{nullptr};
   std::vector<float>* m_TRT_r{nullptr};
   std::vector<float>* m_TRT_time_photons{nullptr};
   std::vector<float>* m_TRT_time_nonphotons{nullptr};
   std::vector<float>* m_TRT_edep_photons{nullptr};
   std::vector<float>* m_TRT_edep_nonphotons{nullptr};
   std::vector<float>* m_TRT_kine_photons{nullptr};
   std::vector<float>* m_TRT_kine_nonphotons{nullptr};
   std::vector<float>* m_TRT_barcode{nullptr};
   
   TTree * m_tree{nullptr};

   Gaudi::Property<std::string> m_path{this, "HistPath","/TRTHitAnalysis/"};
   Gaudi::Property<std::string> m_ntupleFileName{this, "NtupleFileName","/TRTHitAnalysis/"}; 
   SG::ReadHandleKey<TRTUncompressedHitCollection> m_readKey{this, "InputKey", "TRTUncompressedHits"};
   const InDetDD::TRT_DetectorManager* m_detMgr{nullptr};
};

#endif // TRT_HIT_ANALYSIS_H
