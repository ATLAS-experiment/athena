/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/xAODMultiElecMuTauFilter.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "xAODTruth/TruthVertex.h"
#include "TruthUtils/HepMCHelpers.h"

StatusCode xAODMultiElecMuTauFilter::filterInitialize()
{
  CHECK(m_truthPartContKey.initialize());
  return StatusCode::SUCCESS;
}


StatusCode xAODMultiElecMuTauFilter::filterEvent() {
  int numLeptons = 0;
  int numLightLeptons = 0;
  int numHadTaus = 0;
  if (m_TwoSameSignLightLeptonsOneHadTau) {
    if (m_NLeptons!=3) ATH_MSG_ERROR("TwoSameSignLightLeptonsOneHadTau request possible only for NLeptons = 3. Check your jobOptions.");
  }
  int charge1 = 0; 
  int charge2 = 0; 

// Retrieve TruthGen container from xAOD Gen slimmer, contains all particles witout barcode_zero and
// duplicated barcode ones
  SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainer{m_truthPartContKey};
  CHECK(xTruthParticleContainer.isValid());

  // Loop over all particles in the event and build up the grid
  for (const xAOD::TruthParticle* pitr : *xTruthParticleContainer) {
       if (not pitr) continue;
       if (MC::isStable(pitr) && (MC::isElectron(pitr) || MC::isMuon(pitr))) {
         if (pitr->pt() >= m_minPt && std::abs(pitr->eta()) <= m_maxEta) {
           ATH_MSG_DEBUG("Found lepton" << pitr);
            numLeptons++;
            numLightLeptons++;
            if (numLightLeptons==1) { if (pitr->pdgId() < 0) { charge1 = -1; } else { charge1 = 1; } } 
           if (numLightLeptons==2) { if (pitr->pdgId() < 0) { charge2 = -1; } else { charge2 = 1; } } 
         }
         continue;
       }
       //Look for Hadronic taus (leptonic ones will be saved by above) that have status!=3 and don't decay to another tau (FSR)
       if (!m_incHadTau) continue;
       const xAOD::TruthParticle *tau= nullptr;
       const xAOD::TruthParticle *taunu= nullptr;
       if (!MC::isTau(pitr) || !MC::isPhysical(pitr)) continue;
         tau = pitr;
         if(!tau->decayVtx()) continue;
         // Loop over children and:
         // 1. Find if it is hadronic (i.e. no decay lepton).
         // 2. Veto tau -> tau (FSR)
         // 3. Store the tau neutino to calculate the visible momentum
         size_t nout = tau->decayVtx()->nOutgoingParticles();
         for (unsigned int iChild = 0; iChild < nout; ++iChild) {
             const xAOD::TruthParticle* citr =  tau->decayVtx()->outgoingParticle(iChild);
             //Ignore tau -> tau (FSR)
             if (pitr->pdgId() == citr->pdgId()) {
               ATH_MSG_DEBUG("FSR tau decay.  Ignoring!");
               tau = nullptr;
               break;
           }
             // Ignore leptonic decays
             if (MC::isMuon(citr) || MC::isElectron(citr)) {
               tau = nullptr;
               break;
          }
            // Find tau decay nu
             if (std::abs(citr->pdgId()) == MC::NU_TAU) {
               taunu = citr;
           }
         }
         if (tau and taunu) {
           // Good hadronic decay
           CLHEP::HepLorentzVector tauVisMom = CLHEP::HepLorentzVector(tau->px() - taunu->px(),
                                                         tau->py() - taunu->py(),
                                                         tau->pz() - taunu->pz(),
                                                         tau->e()  - taunu->e());
           if (tauVisMom.perp() >= m_minVisPtHadTau && std::abs(tauVisMom.eta()) <= m_maxEta) {
             ATH_MSG_DEBUG("Found hadronic tau decay" << tau);
             numLeptons++;
             numHadTaus++;
        
           }
       }
    }//loop over TruthParticles
  
 bool passed_event = false;
  if (m_TwoSameSignLightLeptonsOneHadTau) {
    if ( ((numLightLeptons+numHadTaus)==numLeptons) && numLightLeptons == 2 && numHadTaus == 1 && charge1==charge2 ) {
      ATH_MSG_DEBUG("Found " << numLeptons << " Leptons: two same sign ligh leptons + one hadronic tau! Event passed.");
      passed_event = true;
    }
  } else {
    if ( numLeptons >= m_NLeptons ) {
      ATH_MSG_DEBUG("Found " << numLeptons << " Leptons. Event passed.");
      passed_event = true;
    }
  }
  setFilterPassed(passed_event);

  return StatusCode::SUCCESS;
}
