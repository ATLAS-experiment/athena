/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUDQA_TAUIDVARIABLESPLOTS_H
#define TAUDQA_TAUIDVARIABLESPLOTS_H

#include "GeneralTauPlots.h"
#include "xAODTau/TauJet.h"

namespace Tau{

class TauIDVariablesPlots: public PlotBase {
  public:
    TauIDVariablesPlots(PlotBase *pParent, const std::string& sDir, const std::string& sTauJetContainerName);
    virtual ~TauIDVariablesPlots();

    void fill(const xAOD::TauJet& tau, float weight);

    Tau::GeneralTauPlots m_oGeneralTauPlots;
    TH1* m_tauCoreFrac{};
    TH1* m_tauEoverPTrk{};
    TH1* m_tauTrkAvgDist{};
    TH1* m_tauIpSig{};
    TH1* m_tauDRMax{};
    TH1* m_tauMtrks{};
    TH1* m_SumPtTrkFrac{};
    
    TH1* m_innerTrkAvgDist{};
    TH1* m_ptRatioEflowApprox{};
    TH1* m_mEflowApprox{};

    TH1* m_ChPiEMEOverCaloEME{};
    TH1* m_EMPOverTrkSysP{};

    TH1* m_IsoFrac{};

    TH1* m_tauSflight{}; 

  private:
    void initializePlots();
    std::string m_sTauJetContainerName;
};

}

#endif
