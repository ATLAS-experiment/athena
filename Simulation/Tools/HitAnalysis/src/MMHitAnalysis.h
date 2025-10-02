/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MM_HIT_ANALYSIS_H
#define MM_HIT_ANALYSIS_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "MuonSimEvent/MMSimHitCollection.h"
#include "StoreGate/ReadHandleKey.h"



class TH1;
class TH2;
class TTree;

class MMHitAnalysis : public AthHistogramAlgorithm {

 public:

  using AthHistogramAlgorithm::AthHistogramAlgorithm;
  ~MMHitAnalysis() = default;
  virtual StatusCode initialize() override final;
  virtual StatusCode execute() override final;

 private:

  /** Some variables**/
  TH2 *m_h_S1_xy_A{nullptr};
  TH2 *m_h_S1_rz_A{nullptr};
  TH1 *m_h_S1_r_A{nullptr};
  TH2 *m_h_S1_xy_C{nullptr};
  TH2 *m_h_S1_rz_C{nullptr};
  TH1 *m_h_S1_r_C{nullptr};
  TH2 *m_h_S2_xy_A{nullptr};
  TH2 *m_h_S2_rz_A{nullptr};
  TH1 *m_h_S2_r_A{nullptr};
  TH2 *m_h_S2_xy_C{nullptr};
  TH2 *m_h_S2_rz_C{nullptr};
  TH1 *m_h_S2_r_C{nullptr};
  TH2 *m_h_S_xy_A{nullptr};
  TH2 *m_h_S_xy_C{nullptr};
  TH2 *m_h_S_rz_A{nullptr};
  TH2 *m_h_S_rz_C{nullptr};

  TH2 *m_h_xy_A{nullptr};
  TH2 *m_h_xy_C{nullptr};
  TH2 *m_h_rz_A{nullptr};
  TH2 *m_h_rz_C{nullptr};

  TH2 *m_h_L1_xy_A{nullptr};
  TH2 *m_h_L1_rz_A{nullptr};
  TH1 *m_h_L1_r_A{nullptr};
  TH2 *m_h_L1_xy_C{nullptr};
  TH2 *m_h_L1_rz_C{nullptr};
  TH1 *m_h_L1_r_C{nullptr};
  TH2 *m_h_L2_xy_A{nullptr};
  TH2 *m_h_L2_rz_A{nullptr};
  TH1 *m_h_L2_r_A{nullptr};
  TH2 *m_h_L2_xy_C{nullptr};
  TH2 *m_h_L2_rz_C{nullptr};
  TH1 *m_h_L2_r_C{nullptr};

  TH2 *m_h_L_xy_A{nullptr};
  TH2 *m_h_L_xy_C{nullptr};
  TH2 *m_h_L_rz_A{nullptr};
  TH2 *m_h_L_rz_C{nullptr};

  SG::ReadHandleKey<MMSimHitCollection> m_readKey{this,  "InputKey", "MM_Hits"};
  Gaudi::Property<std::string> m_path{this, "HistPath", "/MMTHitAnalysis/"};

};

#endif // MM_HIT_ANALYSIS_H
