/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUDQA_EVETOPLOTS_H
#define TAUDQA_EVETOPLOTS_H

#include "TrkValHistUtils/PlotBase.h"
#include "xAODTau/TauJet.h"

namespace Tau{

class EVetoPlots: public PlotBase {
  public:
    EVetoPlots(PlotBase *pParent, const std::string& sDir, std::string sTauJetContainerName);
    virtual ~EVetoPlots() = default;
    void fill(const xAOD::TauJet& tau, float weight);

    // RNN
    TH1* m_id_RNNEleScore{};
    TH1* m_id_RNNEleScoreSigTrans{};
    TH1* m_pt_eleRNNloose{};
    TH1* m_pt_eleRNNmed{}; 
    TH1* m_pt_eleRNNtight{};
    TH1* m_pt_eleRNNlooseHighPt{};
    TH1* m_pt_eleRNNmedHighPt{}; 
    TH1* m_pt_eleRNNtightHighPt{};
    // GNN
    TH1* m_id_GNNEleScore{};
    TH1* m_id_GNNEleScoreSigTrans{};
    TH1* m_pt_eleGNNloose{};
    TH1* m_pt_eleGNNmed{};
    TH1* m_pt_eleGNNtight{};
    TH1* m_pt_eleGNNlooseHighPt{};
    TH1* m_pt_eleGNNmedHighPt{};
    TH1* m_pt_eleGNNtightHighPt{};
    
  private:
    void initializePlots();
    std::string m_sTauJetContainerName;
};

}

#endif
