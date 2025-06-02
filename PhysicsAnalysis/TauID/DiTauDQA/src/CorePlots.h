/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DITAUDQA_COREPLOTS_H
#define DITAUDQA_COREPLOTS_H

#include "TrkValHistUtils/PlotBase.h" //inheritance
#include "xAODTau/DiTauJet.h"

namespace DiTau{

class CorePlots: public PlotBase {
  public:
    CorePlots(PlotBase *pParent, const std::string& sDir, std::string sDiTauJetContainerName);
    virtual ~CorePlots();
    void fill(const xAOD::DiTauJet& ditau, float weight);

    TH1* eta{};
    TH1* phi{};
    TH1* pt{};
    TH1* mass{};
    TH1* nsubjets{};
    TH1* charge{};

    TH2* eta_phi{};
    TH2* eta_pt{};


  private:
    void initializePlots();
    std::string m_sDiTauJetContainerName;
};

}

#endif
