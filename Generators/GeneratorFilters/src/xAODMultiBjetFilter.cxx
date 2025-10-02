/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
// This is a general-purpose multi-b-jet filter. It can cut on:
//    - Multiplicity of b-jets (both min and max can be specified)
//    - Multiplicity of jets (regardless of flavor)
//    - The pT of the leading jet
//
// Written by Bill Balunas (balunas@cern.ch)

// Header for this module:-
#include "GeneratorFilters/xAODMultiBjetFilter.h"

// Other classes used by this class:-
#include <math.h>
#include "GaudiKernel/SystemOfUnits.h"
#include "CxxUtils/BasicTypes.h"
#include "TruthUtils/HepMCHelpers.h"
#include "TLorentzVector.h"

#include <fstream>


StatusCode xAODMultiBjetFilter::filterInitialize() {

  m_Nevt = 0;
  m_NPass = 0;
  m_SumOfWeights_Pass = 0;
  m_SumOfWeights_Evt = 0;

  CHECK(m_TruthJetContainerName.initialize());
  CHECK(m_truthPartContKey.initialize());
  ATH_MSG_INFO("Initialized");
  return StatusCode::SUCCESS;
}

StatusCode xAODMultiBjetFilter::filterFinalize() {

  ATH_MSG_INFO( m_NPass << " Events out of " << m_Nevt << " passed the filter");
  ATH_MSG_INFO(  m_SumOfWeights_Pass << " out of " << m_SumOfWeights_Evt << " SumOfWeights counter, passed/total");
  return StatusCode::SUCCESS;
}


StatusCode xAODMultiBjetFilter::filterEvent() {

  bool pass = true;
  m_Nevt++;

  // Retrieve truth jets
  SG::ReadHandle<xAOD::JetContainer>  truthjetTES{m_TruthJetContainerName};
  if (!truthjetTES.isValid()) {
    ATH_MSG_WARNING("No xAOD::JetContainer with name " << m_TruthJetContainerName.key() << " found in StoreGate!");
    return StatusCode::SUCCESS;
  }

  xAOD::JetContainer::const_iterator jitr;
  double lead_jet_pt = 0.0;

  // Select jets according to kinematic cuts, record leading jet pt
  std::vector<xAOD::JetContainer::const_iterator> jets,bjets;
  for(jitr = (*truthjetTES).begin(); jitr !=(*truthjetTES).end(); ++jitr) {
    if((*jitr)->pt() < m_jetPtMin) continue;
    if(std::abs((*jitr)->eta()) > m_jetEtaMax) continue;
    if((*jitr)->pt() > lead_jet_pt) lead_jet_pt = (*jitr)->pt();
    jets.push_back(jitr);
  }

  // Retrieve TruthGen container from xAOD Gen slimmer, contains all particles witout barcode_zero and 
  // duplicated barcode ones
  SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainer{m_truthPartContKey};
  CHECK(xTruthParticleContainer.isValid());

  // Apply leading jet pt cut
  if(lead_jet_pt < m_leadJet_ptMin || (lead_jet_pt > m_leadJet_ptMax && m_leadJet_ptMax > 0)) pass = false;

  // Apply jet multiplicity cut
  int njets = jets.size();
  if(njets < m_nJetsMin) pass = false;
  if(njets > m_nJetsMax && m_nJetsMax > 0) pass = false;

  int bJetCounter = 0;
 
  // Make a vector containing all the event's b-hadrons
  std::vector< const xAOD::TruthParticle* > bHadrons;  
  for (const xAOD::TruthParticle* part : *xTruthParticleContainer) {
      if( !MC::isWeaklyDecayingBHadron(part) ) continue;
      if( part->pt() < m_bottomPtMin ) continue;
      if( std::abs( part->abseta() ) > m_bottomEtaMax) continue;
      bHadrons.push_back(part);
    }

    // Count how many truth jets contain b-hadrons
    for(uint i = 0; i < jets.size(); i++){
      for(uint j = 0; j < bHadrons.size(); j++){
        TLorentzVector genpart(bHadrons.at(j)->px(), bHadrons.at(j)->py(), bHadrons.at(j)->pz(), bHadrons.at(j)->e());
        double dR = (*jets[i])->p4().DeltaR(genpart);
        if(dR<m_deltaRFromTruth){
          bJetCounter++;
          bjets.push_back(jets[i]);     
          break;
        }
      }
    }

  // Apply b-jet multiplicity cut
  if(bJetCounter < m_nBJetsMin) pass = false;
  if(bJetCounter > m_nBJetsMax && m_nBJetsMax > 0) pass = false;

  // Bookkeeping
 double weight = 1;
 for(const HepMC::GenEvent* genEvt : *events_const()) {
   weight = genEvt->weights().front();
 }
  m_SumOfWeights_Evt += weight;
  if(pass){
    m_NPass++;
    m_SumOfWeights_Pass += weight;
  }

  setFilterPassed(pass);
  return StatusCode::SUCCESS;
}
