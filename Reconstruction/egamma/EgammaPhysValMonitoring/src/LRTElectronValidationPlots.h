/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAPHYSVALMONITORING_LRTELECTRONVALIDATIONPLOTS_H
#define EGAMMAPHYSVALMONITORING_LRTELECTRONVALIDATIONPLOTS_H


#include "GaudiKernel/ToolHandle.h"
#include "TrkValHistUtils/PlotBase.h"
#include "TrkValHistUtils/ParamPlots.h"
#include "LRTElectronPlots.h"
#include "ElectronFrwdPlots.h"
#include "KinematicsPlots.h"
#include "xAODEgamma/Electron.h"
#include "xAODTruth/TruthParticle.h"

class LRTElectronValidationPlots:public PlotBase {
  public:
    LRTElectronValidationPlots(PlotBase* pParent, const std::string& sDir);
    void fill(const xAOD::Electron& electron, const xAOD::EventInfo& eventInfo, bool isPrompt, bool pass_LHVeryLooseNoPix, bool pass_LHLooseNoPix, bool pass_LHMediumNoPix, bool pass_LHTightNoPix);

    std::string m_sParticleType;

    Egamma::LRTElectronPlots           m_oCentralElecPlots;
      
    TH1* author;
    TProfile* res_et;
    TProfile* res_eta;
    TProfile* res_et_cut;
    TProfile* res_eta_cut;
    TProfile* res_et_cut_pt_20;
    TProfile* res_eta_cut_pt_20;
    TH2* matrix;

  private:
    virtual void initializePlots();

};

#endif
