/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "DiTauValidationPlots.h"

DiTauValidationPlots::DiTauValidationPlots(PlotBase* pParent, const std::string& sDir, const std::string& sDiTauJetContainerName):
  PlotBase(pParent, sDir),
  m_oNewCorePlots(this,"NoCuts/RecDiTau/All/", sDiTauJetContainerName)             // all ditau reco
{}	

// no fill method implement in order to let filling logic stay in the ManagedMonitoringTool
