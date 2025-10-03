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
#ifndef CHECKFLOW_H
#define CHECKFLOW_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "FlowAfterburner/GenAccessIO.h"
#include "GeneratorObjects/HijingEventParams.h"

#include <string>


class TH1F;                    //Forward declaration
class TH2F;                    //Forward declaration
class TH3F;                    //Forward declaration

class CheckFlow:public AthAlgorithm {
public:
  CheckFlow(const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

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

  //Histograms, used if m_produceHistogram is true = 1
  TH1F*   m_hgenerated{};
  TH1F*   m_b{};
  TH1F*   m_phi{};
  TH1F*   m_phiR{};
  TH1F*   m_phi_vs_phiR{};
  TH2F*   m_phiv1reco_vs_phiR{};
  TH2F*   m_phiv2reco_vs_phiR{};
  TH1F*   m_phi_vs_phiR_etap{};
  TH1F*   m_phi_vs_phiR_etan{};
  TH3F*   m_v2betapth{};
  TH3F*   m_ebetapth{};

  SG::ReadHandleKey<HijingEventParams> m_hijingKey{this, "HijingEventParmsKey","Hijing_event_params"};
  TruthHelper::GenAccessIO*    m_tesIO{};
};

#endif


