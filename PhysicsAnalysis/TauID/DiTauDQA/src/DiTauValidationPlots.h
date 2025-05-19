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

};

#endif // not DITAUDQA_DITAUVALIDATIONPLOTS_H
