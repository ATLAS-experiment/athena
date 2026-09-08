/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUDQA_TRKCLASSEFFICIENCYPLOTS_H
#define TAUDQA_TRKCLASSEFFICIENCYPLOTS_H

#include "TrkValHistUtils/PlotBase.h"
#include "xAODTau/TauJet.h"
#include "xAODTruth/TruthParticle.h"

class TProfile;

namespace Tau{

class TrkClassEfficiencyPlots: public PlotBase {
public:
  TrkClassEfficiencyPlots(PlotBase *pParent, const std::string& sDir, std::string sTauJetContainerName);
  virtual ~TrkClassEfficiencyPlots() = default;
  
  void fill(std::vector<const xAOD::TruthParticle*> & truth_taus, std::vector<const xAOD::TauJet*> & reco_taus, float weight, float avg_mu);

  TProfile* m_eff_truth_pt_1p{};
  TProfile* m_eff_truth_eta_1p{};
  TProfile* m_eff_mu_1p{};

  TProfile* m_eff_truth_pt_3p{};
  TProfile* m_eff_truth_eta_3p{};
  TProfile* m_eff_mu_3p{};

private:
  void initializePlots();
  std::string m_sTauJetContainerName;
};
  
}

#endif
