/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DITAUDQA_RESOLUTIONPLOTS_H
#define DITAUDQA_RESOLUTIONPLOTS_H
#include "TrkValHistUtils/PlotBase.h"
#include "xAODTau/DiTauJet.h"

namespace DiTau{

  class ResolutionPlots: public PlotBase {
  public:
    ResolutionPlots(PlotBase *pParent, const std::string& sDir, const std::string& sDiTauJetContainerName);
    virtual ~ResolutionPlots();
    void fill(const xAOD::DiTauJet& ditau, float weight);
    
    TH1* m_lead_subjet_ptResolution{};
    TH1* m_lead_subjet_etaResolution{};
    TH1* m_lead_subjet_phiResolution{};	 
    
    TH1* m_sublead_subjet_ptResolution{};
    TH1* m_sublead_subjet_etaResolution{};
    TH1* m_sublead_subjet_phiResolution{};


  private:
    void initializePlots();
    std::string m_sDiTauJetContainerName;
  };
  
}

#endif
