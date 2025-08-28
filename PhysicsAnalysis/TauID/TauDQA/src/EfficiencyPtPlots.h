/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUDQA_EFFICIENCYPTPLOTS_H
#define TAUDQA_EFFICIENCYPTPLOTS_H

#include "TrkValHistUtils/PlotBase.h"
#include "ParamPlots.h"
#include "xAODTau/TauJet.h"

namespace Tau{

class EfficiencyPtPlots: public PlotBase {
public:
  EfficiencyPtPlots(PlotBase *pParent, const std::string& sDir, std::string sTauJetContainerName);
  virtual ~EfficiencyPtPlots();
  
  void fill(const xAOD::TauJet& tau, float weight);
  
  TProfile* m_eff_pt_jetRNNloose{};
  TProfile* m_eff_pt_jetRNNmed{};
  TProfile* m_eff_pt_jetRNNtight{};
  TProfile* m_eff_pt_jetRNNlooseHighPt{};
  TProfile* m_eff_pt_jetRNNmedHighPt{};
  TProfile* m_eff_pt_jetRNNtightHighPt{};

  TProfile* m_eff_jetRNNloose{};
  TProfile* m_eff_jetRNNmed{};
  TProfile* m_eff_jetRNNtight{};

  TProfile* m_eff_pt_jetGNTauloose{};
  TProfile* m_eff_pt_jetGNTaumed{};
  TProfile* m_eff_pt_jetGNTautight{};

  TProfile* m_eff_pt_jetGNTaulooseHighPt{};
  TProfile* m_eff_pt_jetGNTaumedHighPt{};
  TProfile* m_eff_pt_jetGNTautightHighPt{};

  TProfile* m_eff_jetGNTauloose{};
  TProfile* m_eff_jetGNTaumed{};
  TProfile* m_eff_jetGNTautight{};

  TProfile* m_eff_pt_eVetoloose{};
  TProfile* m_eff_pt_eVetomed{};
  TProfile* m_eff_pt_eVetotight{};

  TProfile* m_eff_pt_eVetolooseHighPt{};
  TProfile* m_eff_pt_eVetomedHighPt{};
  TProfile* m_eff_pt_eVetotightHighPt{};

  TProfile* m_eff_eta_eVetoloose{};
  TProfile* m_eff_eta_eVetomed{};
  TProfile* m_eff_eta_eVetotight{};

  TProfile* m_eff_eVetoloose{};
  TProfile* m_eff_eVetomed{};
  TProfile* m_eff_eVetotight{};

  
private:
  void initializePlots();
  std::string m_sTauJetContainerName;
};
  
}

#endif
