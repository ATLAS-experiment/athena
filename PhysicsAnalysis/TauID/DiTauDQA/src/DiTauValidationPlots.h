// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef DITAUDQA_DITAUVALIDATIONPLOTS_H
#define DITAUDQA_DITAUVALIDATIONPLOTS_H

// PlotBase objects
#include "CorePlots.h"

class DiTauValidationPlots:public PlotBase {
    public:
      
      DiTauValidationPlots(PlotBase* pParent, const std::string& sDir, const std::string& sDiTauJetContainerName);
      DiTau::CorePlots m_oNewCorePlots;
      DiTau::CorePlots m_oNewCorePlotsNom; // passing nominal selection      
    
      DiTau::CorePlots m_oNewCorePlotsTrue;
      DiTau::CorePlots m_oNewCorePlotsNomTrue;

      DiTau::CorePlots m_oNewCorePlotsFake;
      DiTau::CorePlots m_oNewCorePlotsNomFake;

};

#endif // not DITAUDQA_DITAUVALIDATIONPLOTS_H
