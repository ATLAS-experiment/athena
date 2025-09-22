// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef TAUDQA_TAUVALIDATIONPLOTS_H
#define TAUDQA_TAUVALIDATIONPLOTS_H

// PlotBase objects
#include "TauKinematicPlots.h"
#include "GeneralTauPlots.h"
#include "TauIDVariablesPlots.h"
#include "EVetoPlots.h"
#include "ResolutionPlots.h"
#include "TauParticleFlowPlots.h"
#include "CorePlots.h"
#include "DecayModeMigration.h"
#include "EfficiencyPlots.h"

#include "xAODJet/JetContainer.h"
#include "xAODEgamma/ElectronContainer.h" 
#include "xAODTau/TauJetContainer.h" 

class TauValidationPlots:public PlotBase {
    public:
      TauValidationPlots(PlotBase* pParent, const std::string& sDir, const std::string& sTauJetContainerName);
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

      // Plots with the "nominal" tau selection
      Tau::TauKinematicPlots m_oElMatchedParamPlotsNom;
      Tau::EVetoPlots m_oElMatchedEVetoPlotsNom;
      Tau::EfficiencyPlots m_oElMatchedEff1PPlotsNom;
      Tau::GeneralTauPlots m_oFakeGeneralNom;
      Tau::TauIDVariablesPlots m_oFakeHad1ProngNom;
      Tau::TauIDVariablesPlots m_oFakeHad3ProngNom;
      Tau::EfficiencyPlots m_oFakeTauEffPlotsNom;
      Tau::EfficiencyPlots m_oFakeTauEff1PPlotsNom;
      Tau::EfficiencyPlots m_oFakeTauEff3PPlotsNom;
      Tau::TauParticleFlowPlots m_oFakeTauRecoTauPlotsNom;
      Tau::CorePlots m_oNewCoreFakePlotsNom;
      
      Tau::GeneralTauPlots m_oRecoGeneralNom;
      Tau::TauIDVariablesPlots m_oRecoHad1ProngNom;
      Tau::TauIDVariablesPlots m_oRecoHad3ProngNom;
      Tau::EfficiencyPlots m_oRecTauEffPlotsNom;
      Tau::EfficiencyPlots m_oRecTauEff1PPlotsNom;
      Tau::EfficiencyPlots m_oRecTauEff3PPlotsNom;
      Tau::TauParticleFlowPlots m_oRecTauRecoTauPlotsNom;
      Tau::CorePlots m_oNewCoreRecTauPlotsNom;
      
      Tau::GeneralTauPlots m_oMatchedGeneralNom;
      Tau::ResolutionPlots m_oMatchedResolutionPlotsNom;
      Tau::ResolutionPlots m_oMatchedResolution1PPlotsNom;
      Tau::ResolutionPlots m_oMatchedResolution3PPlotsNom;
      Tau::TauIDVariablesPlots m_oMatchedHad1ProngNom;
      Tau::TauIDVariablesPlots m_oMatchedHad3ProngNom;
      Tau::EfficiencyPlots m_oMatchedTauEffPlotsNom;
      Tau::EfficiencyPlots m_oMatchedTauEff1PPlotsNom;
      Tau::EfficiencyPlots m_oMatchedTauEff3PPlotsNom;
      Tau::TauParticleFlowPlots m_oMatchedTauRecoTauPlotsNom;
      Tau::DecayModeMigration m_oMigrationPlotsNom;
      Tau::CorePlots m_oNewCoreMatchedPlotsNom;


};

#endif // not TAUDQA_TAUVALIDATIONPLOTS_H
