// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef TAUDQA_TAUVALIDATIONPLOTSNOCUTS_H
#define TAUDQA_TAUVALIDATIONPLOTSNOCUTS_H

// PlotBase objects
#include "TrkValHistUtils/PlotBase.h"
#include "TauKinematicPlots.h"
#include "GeneralTauPlots.h"
#include "TauIDVariablesPlots.h"
#include "EVetoPlots.h"
#include "ResolutionPlots.h"
#include "TauParticleFlowPlots.h"
#include "CorePlots.h"
#include "DecayModeMigration.h"
#include "EfficiencyPlots.h"


class TauValidationPlotsNoCuts:public PlotBase {
    public:
      TauValidationPlotsNoCuts(PlotBase* pParent, const std::string& sDir, const std::string& sTauJetContainerName);
      // general tau all prongs plots
      Tau::GeneralTauPlots m_oGeneralTauAllProngsPlots;


    	// tau1P plots : variables for tau ID and EVeto
      Tau::TauIDVariablesPlots m_oHad1ProngPlots;

      // tau3P plots : variables for tau ID
      Tau::TauIDVariablesPlots m_oHad3ProngPlots;

      // electron veto variables for electrons matching tau candidates	
      Tau::TauKinematicPlots  m_oElMatchedParamPlots;
      Tau::EVetoPlots         m_oElMatchedEVetoPlots;

      // general tau all fake prongs plots
      Tau::GeneralTauPlots m_oFakeGeneralTauAllProngsPlots;


    	// tau1P fake plots : variables for tau ID
      Tau::TauIDVariablesPlots  m_oFakeHad1ProngPlots;		      

      // tau3P fake plots : variables for tau ID
      Tau::TauIDVariablesPlots  m_oFakeHad3ProngPlots;	        

      // general tau all fake prongs plots
      Tau::GeneralTauPlots m_oRecoGeneralTauAllProngsPlots;
	
    	// tau1P fake plots : variables for tau ID
      Tau::TauIDVariablesPlots  m_oRecoHad1ProngPlots;		      

      // tau3P fake plots : variables for tau ID
      Tau::TauIDVariablesPlots  m_oRecoHad3ProngPlots;	        

      // All tau Reco with no match to truth
      Tau::TauParticleFlowPlots m_oRecoTauAllProngsPlots;		

      Tau::TauParticleFlowPlots m_oMatchedTauAllProngsPlots;
      Tau::TauParticleFlowPlots m_oFakeTauAllProngsPlots;

      //Efficiency plots
      Tau::EfficiencyPlots m_oMatchedTauEffPlots;
      Tau::EfficiencyPlots m_oMatchedTauEff1PPlots;
      Tau::EfficiencyPlots m_oMatchedTauEff3PPlots;

      Tau::EfficiencyPlots m_oRecTauEffPlots;
      Tau::EfficiencyPlots m_oRecTauEff1PPlots;
      Tau::EfficiencyPlots m_oRecTauEff3PPlots;

      Tau::EfficiencyPlots m_oFakeTauEffPlots;
      Tau::EfficiencyPlots m_oFakeTauEff1PPlots;
      Tau::EfficiencyPlots m_oFakeTauEff3PPlots;

      // All tau Reco with Backwards compatability, for comparison with 17.X.Y
      Tau::CorePlots m_oNewCorePlots;		

      Tau::CorePlots m_oNewCoreMatchedPlots;
      Tau::CorePlots m_oNewCoreFakePlots;

      //DecayMode Migration Matrix plots
      Tau::DecayModeMigration m_oMigrationPlots;

      //Resolution Plots 
      Tau::ResolutionPlots m_oMatchedResolutionPlots;
      Tau::ResolutionPlots m_oMatchedResolution1PPlots;
      Tau::ResolutionPlots m_oMatchedResolution3PPlots;



};

#endif // not TAUDQA_TAUVALIDATIONPLOTSNOCUTS_H
