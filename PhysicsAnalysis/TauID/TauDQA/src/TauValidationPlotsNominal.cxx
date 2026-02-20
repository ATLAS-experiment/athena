/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "TauValidationPlotsNominal.h"

TauValidationPlotsNominal::TauValidationPlotsNominal(PlotBase* pParent, const std::string& sDir, const std::string& sTauJetContainerName):
  PlotBase(pParent, sDir),

  // Plots with the "nominal" tau selection
  m_oElMatchedParamPlotsNom(this, "Nominal/Elec/", sTauJetContainerName),     // electron veto variables for electrons matching tau candidates and passing nominal selection        
  m_oElMatchedEVetoPlotsNom(this, "Nominal/Elec/", sTauJetContainerName),     // electron veto variables for electrons matching tau candidates and passing nominal selection
  m_oElMatchedEff1PPlotsNom(this, "Nominal/Elec/Eff/1P/", sTauJetContainerName),
  m_oFakeGeneralNom(this,"Nominal/Fake/", sTauJetContainerName),
  m_oFakeHad1ProngNom(this,"Nominal/Fake/Jet1P/", sTauJetContainerName),
  m_oFakeHad3ProngNom(this,"Nominal/Fake/Jet3P/", sTauJetContainerName),
  m_oFakeTauEffPlotsNom(this, "Nominal/Fake/Eff/All/", sTauJetContainerName),
  m_oFakeTauEff1PPlotsNom(this, "Nominal/Fake/Eff/Jet1P/", sTauJetContainerName),
  m_oFakeTauEff3PPlotsNom(this, "Nominal/Fake/Eff/Jet3P/", sTauJetContainerName),
  m_oFakeTauRecoTauPlotsNom(this, "Nominal/Fake/PFOs/", sTauJetContainerName),
  m_oNewCoreFakePlotsNom(this, "Nominal/Fake/All/", sTauJetContainerName),

  m_oRecoGeneralNom(this,"Nominal/RecTau/", sTauJetContainerName),
  m_oRecoHad1ProngNom(this,"Nominal/RecTau/1P/", sTauJetContainerName),
  m_oRecoHad3ProngNom(this,"Nominal/RecTau/3P/", sTauJetContainerName),
  m_oRecTauEffPlotsNom(this, "Nominal/RecTau/Eff/All/", sTauJetContainerName),
  m_oRecTauEff1PPlotsNom(this, "Nominal/RecTau/Eff/1P/", sTauJetContainerName),
  m_oRecTauEff3PPlotsNom(this, "Nominal/RecTau/Eff/3P/", sTauJetContainerName),
  m_oRecTauRecoTauPlotsNom(this, "Nominal/RecTau/PFOs/", sTauJetContainerName),
  m_oNewCoreRecTauPlotsNom(this, "Nominal/RecTau/All/", sTauJetContainerName),

  m_oMatchedGeneralNom(this,"Nominal/Matched/", sTauJetContainerName),
  m_oMatchedResolutionPlotsNom(this,"Nominal/Matched/All/", sTauJetContainerName),
  m_oMatchedResolution1PPlotsNom(this,"Nominal/Matched/Tau1P/", sTauJetContainerName),
  m_oMatchedResolution3PPlotsNom(this,"Nominal/Matched/Tau3P/", sTauJetContainerName),
  m_oMatchedHad1ProngNom(this,"Nominal/Matched/Tau1P/", sTauJetContainerName),
  m_oMatchedHad3ProngNom(this,"Nominal/Matched/Tau3P/", sTauJetContainerName),
  m_oMatchedTauEffPlotsNom(this, "Nominal/Matched/Eff/All/", sTauJetContainerName),
  m_oMatchedTauEff1PPlotsNom(this, "Nominal/Matched/Eff/1P/", sTauJetContainerName),
  m_oMatchedTauEff3PPlotsNom(this, "Nominal/Matched/Eff/3P/", sTauJetContainerName),
  m_oMatchedTauRecoTauPlotsNom(this, "Nominal/Matched/PFOs/", sTauJetContainerName),
  m_oMigrationPlotsNom(this, "Nominal/Matched/Migration/", sTauJetContainerName),
  m_oNewCoreMatchedPlotsNom(this, "Nominal/Matched/All/", sTauJetContainerName),
  
  m_oEventPlotsNom(this, "EventInfo") 

{}	

// no fill method implement in order to let filling logic stay in the ManagedMonitoringTool
