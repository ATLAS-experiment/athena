/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUDQA_RESOLUTIONPLOTS_H
#define TAUDQA_RESOLUTIONPLOTS_H
#include "TrkValHistUtils/PlotBase.h"
#include "TauAnalysisTools/ITauTruthMatchingTool.h"
#include "xAODTau/TauJet.h"
#include "TH1.h"
namespace Tau{

  class ResolutionPlots: public PlotBase {
  public:
    ResolutionPlots(PlotBase *pParent, const std::string& sDir, std::string sTauJetContainerName);
    virtual ~ResolutionPlots();
    void fill(const xAOD::TauJet& tau, const xAOD::TruthParticle&, float weight);
    
    TH1* m_ptResolution;
    TH1* m_etaResolution;
    TH1* m_phiResolution;	 
    
  private:
    void initializePlots();
    std::string m_sTauJetContainerName;
  };
  
}

#endif
