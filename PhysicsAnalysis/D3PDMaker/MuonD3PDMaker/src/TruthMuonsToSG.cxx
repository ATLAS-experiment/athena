/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file MuonD3PDMaker/src/TruthMuonsToSG.h
 * @author Srivas Prasad <srivas.prasad@cern.ch>
 * @date March 2010
 * @brief Create collection of true muons in StoreGate, based on analogous code in TauD3PDMaker
 */


#include "TruthMuonsToSG.h"

#include "xAODTruth/TruthParticleContainer.h"
#include "TruthUtils/HepMCHelpers.h"
#include "AthContainers/ConstDataVector.h"
#include "AthenaKernel/errorcheck.h"


namespace D3PD {


TruthMuonsToSG::TruthMuonsToSG(const std::string& name, ISvcLocator* pSvcLocator)
  : AthAlgorithm(name, pSvcLocator),
    m_resolver (name, evtStore(), m_truthContainerKey),
    m_classifier ("MCTruthClassifier")
{
  declareProperty("TruthContainer",      m_truthContainerKey = "TruthParticle" );
  declareProperty("TruthMuonContainer",  m_truthMuonContainerKey = "TruthMuons" );
  declareProperty ("Classifier", m_classifier, "Classifier tool instance.");

}

StatusCode TruthMuonsToSG::initialize() {
  CHECK( m_resolver.initialize<xAOD::TruthParticleContainer>() );
  CHECK( m_classifier.retrieve() );
  return StatusCode::SUCCESS;
}

StatusCode TruthMuonsToSG::finalize() {
  return StatusCode::SUCCESS;
}


StatusCode TruthMuonsToSG::execute() {
  const xAOD::TruthParticleContainer *truthCollection = nullptr;
  StatusCode sc=evtStore()->retrieve( truthCollection, m_resolver.key());
  if( sc.isFailure()  ||  !truthCollection) {
    ATH_MSG_ERROR("Failed to retrieve xAOD::TruthParticleContainer " << m_resolver.key());
    return StatusCode::FAILURE;
  }

  // output container for muons
  ConstDataVector<xAOD::TruthParticleContainer>*  muonColl
    = new ConstDataVector<xAOD::TruthParticleContainer>(SG::VIEW_ELEMENTS);

  for (const xAOD::TruthParticle* p : *truthCollection) {
    if( abs(p->pdgId()) == MC::MUON && MC::isStable(p) ) {
      muonColl->push_back(p);
    }
    else if( MC::isPhoton(p) ) {
      auto Classify = m_classifier->particleTruthClassifier( p );
      if( (MCTruthPartClassifier::ParticleOrigin)(Classify.second) == MCTruthPartClassifier::FSRPhot  ) {
        muonColl->push_back(p);
      }
    }
  }
  
  if( evtStore()->record(muonColl, m_truthMuonContainerKey).isFailure() ) return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}


} // namespace D3PD
