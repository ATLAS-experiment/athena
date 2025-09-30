/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef sTGC_HIT_ANALYSIS_H
#define sTGC_HIT_ANALYSIS_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"

#include "MuonSimEvent/sTGCSimHitCollection.h"
#include "StoreGate/ReadHandleKey.h"


class sTGCHitAnalysis : public AthHistogramAlgorithm {

 public:


  using AthHistogramAlgorithm::AthHistogramAlgorithm;
  ~sTGCHitAnalysis() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;

 private:

  /** Some variables**/

  TH2 *m_h_S_rz_A{nullptr};
  TH2 *m_h_S_rz_C{nullptr};
  TH2 *m_h_S_xy_A{nullptr};
  TH2 *m_h_S_xy_C{nullptr};
  TH2 *m_h_L_rz_A{nullptr};
  TH2 *m_h_L_rz_C{nullptr};
  TH2 *m_h_L_xy_A{nullptr};
  TH2 *m_h_L_xy_C{nullptr};
  TH2 *m_h_rz_A{nullptr};
  TH2 *m_h_rz_C{nullptr};
  TH2 *m_h_xy_A{nullptr};
  TH2 *m_h_xy_C{nullptr};
  TH1 *m_h_r_A{nullptr};
  TH1 *m_h_r_C{nullptr};

  SG::ReadHandleKey<sTGCSimHitCollection> m_readKey{this,  "InputKey", "sTGC_Hits"};
  Gaudi::Property<std::string> m_path{this, "HistPath", "/sTGCHitAnalysis/"};


};

#endif // sTGC_HIT_ANALYSIS_H
