// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef TAUDQA_TAUVALIDATIONPLOTSNOMINAL_H
#define TAUDQA_TAUVALIDATIONPLOTSNOMINAL_H

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

class TauValidationPlotsNominal:public PlotBase {
   public:
      TauValidationPlotsNominal(PlotBase* pParent, const std::string& sDir, const std::string& sTauJetContainerName);


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

#endif // not TAUDQA_TAUVALIDATIONPLOTSNOMINAL_H
