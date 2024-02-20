/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/***********************************************************************

NAME:     MCTruthClassifier.cxx
PACKAGE:  atlasoff/PhysicsAnalysis/MCTruthClassifier

AUTHORS:  O. Fedin
CREATED:  Sep 2007

PURPOSE:  to classify  truth particles according to their origin. Based on
          the truth particle classification the tool provide classification of
          ID and combined muon tracks, egamma electrons (including forward
          electrons) and egamma photons. Both AOD and ESD files can be used.
          See  for details:
    https://twiki.cern.ch/twiki/bin/view/AtlasProtected/EgammaMCTruthClassifier
Updated:
************************************************************************
Updated Feb 2024 by Andrii Verbytskyi <andrii.verbytskyi@mpp.mpg.de>
************************************************************************/

#include "MCTruthClassifier/MCTruthClassifier.h"
using namespace MCTruthPartClassifier;

// -----------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------
MCTruthClassifier::MCTruthClassifier(const std::string& type) : asg::AsgTool(type) {

#if !defined(XAOD_ANALYSIS) && !defined(GENERATIONBASE)
  declareProperty("FwdElectronUseG4Sel" , m_FwdElectronUseG4Sel = true, "Use Geant4 selection for forward electrons calo clusters");
  declareProperty("FwdElectronTruthExtrEtaCut", m_FwdElectronTruthExtrEtaCut = 2.4, "Cut on the eta of the truth Particles to be extrapolated for Fwd electrons");
  declareProperty("FwdElectronTruthExtrEtaWindowCut", m_FwdElectronTruthExtrEtaWindowCut = 0.15, "Cut on the delta eta of the truth Particles to be extrapolated for Fwd electrons and the current FwdElectron");
  declareProperty("partExtrConePhi", m_partExtrConePhi = 0.4);
  declareProperty("partExtrConeEta", m_partExtrConeEta = 0.2);
  declareProperty("phtClasConePhi", m_phtClasConePhi = 0.05);
  declareProperty("phtClasConeEta", m_phtClasConeEta = 0.025);
  declareProperty("useCaching", m_useCaching = true);
  declareProperty("phtdRtoTrCut", m_phtdRtoTrCut = 0.1);
  declareProperty("fwrdEledRtoTrCut", m_fwrdEledRtoTrCut = 0.15);
  declareProperty("ROICone", m_ROICone = false);
#endif
#ifndef GENERATIONBASE
  declareProperty("deltaRMatchCut", m_deltaRMatchCut = 0.2);
  declareProperty("deltaPhiMatchCut", m_deltaPhiMatchCut = 0.2);
  declareProperty("NumOfSiHitsCut", m_NumOfSiHitsCut = 3);
  declareProperty("jetPartDRMatch", m_jetPartDRMatch = 0.4);
#endif
  // Properties Available in all
  declareProperty("inclEgammaFwrdEle", m_inclEgammaFwrdEle = true);
  declareProperty("inclEgammaPhoton", m_inclEgammaPhoton = true);
  declareProperty("pTChargePartCut", m_pTChargePartCut = 1.0);
  declareProperty("pTNeutralPartCut", m_pTNeutralPartCut = 0.);
  declareProperty("inclG4part", m_inclG4part = false);

}

MCTruthClassifier::~MCTruthClassifier() = default;

StatusCode MCTruthClassifier::initialize(){
  ATH_MSG_INFO(" Initializing MCTruthClassifier");
#ifndef XAOD_ANALYSIS
  // Only needed for GenParticle interface
  if (!m_truthLinkVecReadHandleKey.key().empty()) {
    ATH_CHECK(m_truthLinkVecReadHandleKey.initialize());
  }
#endif
  ATH_CHECK(m_truthParticleContainerKey.initialize());

#if !defined(XAOD_ANALYSIS) && !defined(GENERATIONBASE)
  if (!m_caloExtensionTool.empty()) {
    ATH_CHECK(m_caloExtensionTool.retrieve());
  } else {
    m_caloExtensionTool.disable();
  }

  ATH_CHECK(m_caloMgrKey.initialize(SG::AllowEmpty));

  if (!m_truthInConeTool.empty()) {
    ATH_CHECK(m_truthInConeTool.retrieve());
  } else {
    m_truthInConeTool.disable();
  }
#endif
  return StatusCode::SUCCESS;
}

StatusCode MCTruthClassifier::finalize()
{
  return StatusCode::SUCCESS;
}
