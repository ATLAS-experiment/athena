/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// File:  Generators/FlowAfterburner/CheckFlow.h
// Description:
//    This is a simple algorithm to histogram particle properties
//    for diagnosing of flow generation
//
//    It has a single important parameter m_rapcut 
//    to cut off particles from very forward pseudorapidity region
//
// AuthorList:
// Andrzej Olszewski: Initial Code February 2006
#ifndef CHECKFLOWNEW_H
#define CHECKFLOWNEW_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GeneratorObjects/HijingEventParams.h"
#include "GaudiKernel/ITHistSvc.h"
#include <string>

namespace TruthHelper{
  class GenAccessIO;
}
class TH1D;                    
class TProfile; 

class CheckFlow_New:public AthAlgorithm {
public:
  CheckFlow_New(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;

private:
  //Declare the algorithm's properties
  StringProperty  m_key{this, "McEventKey", "FLOW_EVENT"};
  BooleanProperty  m_produceHistogram{this, "HistogramFlag", true};

  DoubleProperty  m_bcut_min{this, "ImpactCutMin", 0.};
  DoubleProperty  m_bcut_max{this, "ImpactCutMax", 99.};
  DoubleProperty  m_ptcut_min{this, "PtCutMin", 0.};
  DoubleProperty  m_ptcut_max{this, "PtCutMax", 999999.};
  DoubleProperty  m_rapcut_min{this, "RapidityCutMin", 0.};
  DoubleProperty  m_rapcut_max{this, "RapidityCutMax", 5.5};

  enum{
  n_ptbin=16,
  n_etabin=8
  };

  //Histograms, used if m_produceHistogram is true = 1
  TH1D *m_hist_Psi_n_true    [6]{};
  TH1D *m_hist_Psi_n_reco    [6]{};
  TH1D *m_hist_psi_corr_true [36]{};
  TH1D *m_hist_psi_corr_reco [36]{};

  TH1D *m_hist_Psi_n_ebe     [6]{};
  TH1D *m_hist_Psi_n_ebe_pt  [6]{};
  TH1D *m_hist_vn_ebe        [6]{};

  TProfile *m_profile_pt_dep      [6][n_etabin]{};
  TProfile *m_profile_eta_dep     [6][n_ptbin ]{};
  TProfile *m_profile_pt_dep_reco [6][n_etabin]{};
  TProfile *m_profile_eta_dep_reco[6][n_ptbin ]{};

  TProfile *m_profile_resolution{};

  SG::ReadHandleKey<HijingEventParams> m_hijingKey{this, "HijingEventParmsKey","Hijing_event_params"};
  TruthHelper::GenAccessIO*    m_tesIO{};
};

#endif


