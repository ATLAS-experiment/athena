/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// GeneratorFilters/DecaysFinalStateFilter
//
// Picks events with a given number of quarks, leptons and neutrinos from
// decays of a list of specified resonances (e.g. W, Z, ...).
// Will work only if resonances are explicitly included in event record.
//
// Examples:
//
//   topAlg.DecaysFinalStateFilter.PDGAllowedParents = [ 23 ]
//   topAlg.DecaysFinalStateFilter.NQuarks = 2
//   topAlg.DecaysFinalStateFilter.NChargedLeptons = 2
//   -> picks semileptonic ZZ decays
//
//   topAlg.DecaysFinalStateFilter.PDGAllowedParents = [ 23, 24, -24 ]
//   topAlg.DecaysFinalStateFilter.NQuarks = 2
//   topAlg.DecaysFinalStateFilter.NChargedLeptons = 2
//   -> allows W(qq)Z(ll) and Z(qq)Z(ll)
//
// Requirements can be put on each lepton flavor individually or on charged
// leptons inclusively. To put requirements on a single lepton flavor, set
// the corresponding property while leaving the properties for the other
// lepton flavors as well as ChargedLeptons as their default values.
//
// Authors:
// Kerim Suruliz Nov 2014
// Frank Siegert Nov 2014
// Jason Veatch Nov 2023

#include "GeneratorFilters/xAODDecaysFinalStateFilter.h"
//#include "GaudiKernel/MsgStream.h"
#include <cmath>


xAODDecaysFinalStateFilter::xAODDecaysFinalStateFilter(const std::string& name, ISvcLocator* pSvcLocator)
  : GenFilter(name,pSvcLocator) 
{
  declareProperty("PDGAllowedParents", m_PDGAllowedParents);

  declareProperty("NQuarks", m_NQuarks = -1);
  declareProperty("NElectrons", m_NElectrons = -1);
  declareProperty("NMuons", m_NMuons = -1);
  declareProperty("NTaus", m_NTaus = -1);
  declareProperty("NChargedLeptons", m_NChargedLeptons = -1);
  declareProperty("NNeutrinos", m_NNeutrinos = -1);
  declareProperty("NPhotons", m_NPhotons = -1);

  declareProperty("MinNQuarks", m_MinNQuarks = 0);
  declareProperty("MinNElectrons", m_MinNElectrons = 0);
  declareProperty("MinNMuons", m_MinNMuons = 0);
  declareProperty("MinNTaus", m_MinNTaus = 0);
  declareProperty("MinNChargedLeptons", m_MinNChargedLeptons = 0);
  declareProperty("MinNNeutrinos", m_MinNNeutrinos = 0);
  declareProperty("MinNPhotons", m_MinNPhotons = 0);
}


StatusCode xAODDecaysFinalStateFilter::filterEvent() {
    int nElectrons = 0;
    int nMuons = 0;
    int nTaus = 0;
    int nChargedLeptons = 0;
    int nQuarks = 0;
    int nNeutrinos = 0;
    int nPhotons = 0;

// Retrieve TruthGen container from xAOD Gen slimmer, contains all particles witout barcode_zero and
// duplicated barcode ones
  const xAOD::TruthParticleContainer* xTruthParticleContainer;
  if (evtStore()->retrieve(xTruthParticleContainer, "TruthGen").isFailure()) {
      ATH_MSG_ERROR("No TruthParticle collection with name " << "TruthGen" << " found in StoreGate!");
      return StatusCode::FAILURE;
  }


  // Loop over all particles in the event and build up the grid
  unsigned int nPart = xTruthParticleContainer->size();
  for (unsigned int iPart = 0; iPart < nPart; ++iPart) {
      const xAOD::TruthParticle* part =  (*xTruthParticleContainer)[iPart];

            // look only at the allowed parents (e.g. W, Z)
            bool allowedParent = false;
            for (size_t i=0; i<m_PDGAllowedParents.size(); ++i) {
            if ( part->pdgId() == m_PDGAllowedParents[i]) allowedParent = true;
            }
            if (!allowedParent) continue;

            if (!part->decayVtx()) continue;
            const xAOD::TruthVertex* decayVertex = part->decayVtx();
            int num_outgoing_particles = decayVertex->nOutgoingParticles();

            for (int iOutPart = 0; iOutPart< num_outgoing_particles; iOutPart++) {
                const xAOD::TruthParticle* out_part =  decayVertex->outgoingParticle(iOutPart);
                int apid = std::abs(out_part->pdgId());
                if (apid == 1 || apid == 2 || apid == 3 || apid == 4 || apid ==5) nQuarks++;
                if (apid == 11) { nElectrons++; nChargedLeptons++; }
                if (apid == 13) { nMuons++; nChargedLeptons++; }
                if (apid == 15) { nTaus++; nChargedLeptons++; }
                if (apid == 12 || apid == 14 || apid == 16) nNeutrinos++;
                if (apid == 22) nPhotons++;
            }

       }//loop over TruthParticles
    

  if (nQuarks < m_MinNQuarks || (m_NQuarks != -1 && nQuarks != m_NQuarks)) {
    setFilterPassed(false);
  }
  else if (nElectrons < m_MinNElectrons || (m_NElectrons != -1 && nElectrons != m_NElectrons)) {
    setFilterPassed(false);
  } 
  else if (nMuons < m_MinNMuons || (m_NMuons != -1 && nMuons != m_NMuons)) {
    setFilterPassed(false);
  } 
  else if (nTaus < m_MinNTaus || (m_NTaus != -1 && nTaus != m_NTaus)) {
    setFilterPassed(false);
  } 
  else if (nChargedLeptons < m_MinNChargedLeptons || (m_NChargedLeptons != -1 && nChargedLeptons != m_NChargedLeptons)) {
    setFilterPassed(false);
  } 
  else if (nNeutrinos < m_MinNNeutrinos || (m_NNeutrinos != -1 && nNeutrinos != m_NNeutrinos)) {
    setFilterPassed(false);
  } 
  else if (nPhotons < m_MinNPhotons || (m_NPhotons != -1 && nPhotons != m_NPhotons)) {
    setFilterPassed(false);
  } 
  else {
    setFilterPassed(true);
  }
  return StatusCode::SUCCESS;
}
