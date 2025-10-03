/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGC_HIT_ANALYSIS_H
#define TGC_HIT_ANALYSIS_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"

#include "MuonSimEvent/TGCSimHitCollection.h"
#include "StoreGate/ReadHandleKey.h"


class TGCHitAnalysis : public AthHistogramAlgorithm {

 public:

   using AthHistogramAlgorithm::AthHistogramAlgorithm;
   virtual ~TGCHitAnalysis() = default;

   virtual StatusCode initialize() override;
   virtual StatusCode execute() override;

 private:

   /** Some variables**/
   TH1* m_h_hits_x{nullptr};
   TH1* m_h_hits_y{nullptr};
   TH1* m_h_hits_z{nullptr};
   TH1* m_h_hits_r{nullptr};
   TH2* m_h_xy{nullptr};
   TH2* m_h_rz{nullptr};
   TH1* m_h_hits_eta{nullptr};
   TH1* m_h_hits_phi{nullptr};
   TH1* m_h_hits_lx{nullptr};
   TH1* m_h_hits_ly{nullptr};
   TH1* m_h_hits_lz{nullptr};
   TH1* m_h_hits_dcx{nullptr};
   TH1* m_h_hits_dcy{nullptr};
   TH1* m_h_hits_dcz{nullptr};
   TH1* m_h_hits_time{nullptr};
   TH1* m_h_hits_edep{nullptr};
   TH1* m_h_hits_kine{nullptr};
   TH1* m_h_hits_step{nullptr};

   SG::ReadHandleKey<TGCSimHitCollection> m_readKey{this,  "InputKey", "TGC_Hits"};
   Gaudi::Property<std::string> m_path{this, "HistPath", "/sTGCHitAnalysis/"};

};

#endif // TGC_HIT_ANALYSIS_H
