/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONHISTUTILS_MOMENTUMPULLPLOTS_H
#define MUONHISTUTILS_MOMENTUMPULLPLOTS_H

#include "TrkValHistUtils/PlotBase.h"
#include "xAODMuon/Muon.h"

namespace Muon {

class MomentumPullPlots : public PlotBase {
   public:
    MomentumPullPlots(PlotBase* pParent, const std::string& sDir)
        : PlotBase(pParent, sDir) {}
    void fill(const xAOD::Muon& mu, float weight = 1.0);

    TH1* dpt_idme{nullptr};
    TH1* ddpt_idme{nullptr};
    TH1* dphi_idme{nullptr};
    TH1* deta_idme{nullptr};

    TH1* dpt_cbme{nullptr};
    TH1* ddpt_cbme{nullptr};
    TH1* dphi_cbme{nullptr};
    TH1* deta_cbme{nullptr};

    TH2* pt_cbme{nullptr};
    TH2* pt_cbid{nullptr};
    TH2* pt_meid{nullptr};

   private:
    void initializePlots();
};

}  // namespace Muon

#endif
