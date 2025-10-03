/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "TauValidationPlotsNoCuts.h"

TauValidationPlotsNoCuts::TauValidationPlotsNoCuts(PlotBase* pParent, const std::string& sDir, const std::string& sTauJetContainerName):
  PlotBase(pParent, sDir),
  // Plots with the "primitive" tau selection
    
  m_oGeneralTauAllProngsPlots(this, "NoCuts/Matched/", sTauJetContainerName),
  m_oHad1ProngPlots(this, "NoCuts/Matched/Tau1P/", sTauJetContainerName),             // tau1P plots : variables for tau ID
  m_oHad3ProngPlots(this, "NoCuts/Matched/Tau3P/", sTauJetContainerName),             // tau3P plots : variables for tau ID
  m_oElMatchedParamPlots(this, "NoCuts/Elec/", sTauJetContainerName),     // electron veto variables for electrons matching tau candidates	  
  m_oElMatchedEVetoPlots(this, "NoCuts/Elec/", sTauJetContainerName),     // electron veto variables for electrons matching tau candidates
  m_oFakeGeneralTauAllProngsPlots(this,"NoCuts/Fake/", sTauJetContainerName),  // general tau all fake prongs plots
  m_oFakeHad1ProngPlots(this,"NoCuts/Fake/Jet1P/", sTauJetContainerName), 		     // tau1P fake plots : variables for tau ID
  m_oFakeHad3ProngPlots(this,"NoCuts/Fake/Jet3P/", sTauJetContainerName),  	       // tau3P fake plots : variables for tau ID
  m_oRecoGeneralTauAllProngsPlots(this,"NoCuts/RecTau/",sTauJetContainerName),// "recTau_General"),  // general tau all fake prongs plots
  m_oRecoHad1ProngPlots(this,"NoCuts/RecTau/1P/", sTauJetContainerName),//"recTau_1P"), 		     // tau1P fake plots : variables for tau ID
  m_oRecoHad3ProngPlots(this,"NoCuts/RecTau/3P/", sTauJetContainerName),// "recTau_3P"),  	       // tau3P fake plots : variables for tau ID
  m_oRecoTauAllProngsPlots(this,"NoCuts/RecTau/PFOs/", sTauJetContainerName),             // all tau reco, no match to truth
  m_oMatchedTauAllProngsPlots(this,"NoCuts/Matched/PFOs/", sTauJetContainerName),
  m_oFakeTauAllProngsPlots(this,"NoCuts/Fake/PFOs/", sTauJetContainerName),             // all tau reco, no match to truth
  m_oMatchedTauEffPlots  (this,"NoCuts/Matched/Eff/All/", sTauJetContainerName),
  m_oMatchedTauEff1PPlots(this,"NoCuts/Matched/Eff/Tau1P/", sTauJetContainerName),
  m_oMatchedTauEff3PPlots(this,"NoCuts/Matched/Eff/Tau3P/", sTauJetContainerName),
  m_oRecTauEffPlots  (this,"NoCuts/RecTau/Eff/All/", sTauJetContainerName),
  m_oRecTauEff1PPlots(this,"NoCuts/RecTau/Eff/Tau1P/", sTauJetContainerName),
  m_oRecTauEff3PPlots(this,"NoCuts/RecTau/Eff/Tau3P/", sTauJetContainerName),
  m_oFakeTauEffPlots  (this,"NoCuts/Fake/Eff/All/", sTauJetContainerName),
  m_oFakeTauEff1PPlots(this,"NoCuts/Fake/Eff/Tau1P/", sTauJetContainerName),
  m_oFakeTauEff3PPlots(this,"NoCuts/Fake/Eff/Tau3P/", sTauJetContainerName),
  m_oNewCorePlots(this,"NoCuts/RecTau/All/", sTauJetContainerName),             // all tau reco, newCore variables
  m_oNewCoreMatchedPlots(this,"NoCuts/Matched/All/", sTauJetContainerName),             // all tau reco, newCore variables
  m_oNewCoreFakePlots(this,"NoCuts/Fake/All/", sTauJetContainerName),             // all tau reco, newCore variables
  m_oMigrationPlots(this,"NoCuts/Matched/Migration/", sTauJetContainerName),             // Migration Matrix
  m_oMatchedResolutionPlots(this,"NoCuts/Matched/All/", sTauJetContainerName),
  m_oMatchedResolution1PPlots(this,"NoCuts/Matched/Tau1P/", sTauJetContainerName),
  m_oMatchedResolution3PPlots(this,"NoCuts/Matched/Tau3P/", sTauJetContainerName)
  

{}	

// no fill method implement in order to let filling logic stay in the ManagedMonitoringTool
