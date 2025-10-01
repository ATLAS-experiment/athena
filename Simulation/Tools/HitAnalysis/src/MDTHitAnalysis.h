/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MDT_HIT_ANALYSIS_H
#define MDT_HIT_ANALYSIS_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"

#include "MuonSimEvent/MDTSimHitCollection.h"
#include "StoreGate/ReadHandleKey.h"


class MDTHitAnalysis : public AthHistogramAlgorithm {

 public:

   using AthHistogramAlgorithm::AthHistogramAlgorithm;
   ~MDTHitAnalysis() = default;

   virtual StatusCode initialize() override final;
   virtual StatusCode execute() override final;

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
   TH1* m_h_hits_lx{nullptr};
   TH1* m_h_hits_ly{nullptr};
   TH1* m_h_hits_lz{nullptr};
   TH1* m_h_hits_driftR{nullptr};
   TH1* m_h_hits_time{nullptr};
   TH1* m_h_hits_edep{nullptr};
   TH1* m_h_hits_kine{nullptr};
   TH1* m_h_hits_step{nullptr};

   SG::ReadHandleKey<MDTSimHitCollection> m_readKey{this,  "InputKey", "MDT_Hits"};
   Gaudi::Property<std::string> m_path{this, "HistPath", "/MDTHitAnalysis/"};

};

#endif // MDT_HIT_ANALYSIS_H
