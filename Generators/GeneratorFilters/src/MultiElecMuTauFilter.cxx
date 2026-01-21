/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/MultiElecMuTauFilter.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "TruthUtils/HepMCHelpers.h"

MultiElecMuTauFilter::MultiElecMuTauFilter(const std::string& name, ISvcLocator* pSvcLocator)
  : GenFilter(name,pSvcLocator)
{
declareProperty("MinPt",                             m_minPt                            = 5000. );
declareProperty("MaxEta",                            m_maxEta                           = 10.0  );
declareProperty("MinVisPtHadTau",                    m_minVisPtHadTau                   = 10000.);
declareProperty("NLeptons",                          m_NLeptons                         = 4     );
declareProperty("IncludeHadTaus",                    m_incHadTau                        = true  );
declareProperty("TwoSameSignLightLeptonsOneHadTau",  m_TwoSameSignLightLeptonsOneHadTau = false );
}


StatusCode MultiElecMuTauFilter::filterEvent() {
  int numLeptons = 0;
  int numLightLeptons = 0;
  int numHadTaus = 0;
  if (m_TwoSameSignLightLeptonsOneHadTau) {
    if (m_NLeptons!=3) ATH_MSG_ERROR("TwoSameSignLightLeptonsOneHadTau request possible only for NLeptons = 3. Check your jobOptions.");
  }
  int charge1 = 0; 
  int charge2 = 0; 

  McEventCollection::const_iterator itr;
  for (itr = events()->begin(); itr != events()->end(); ++itr) {
    const HepMC::GenEvent* genEvt = *itr;
    for (const auto& pitr: *genEvt) {
      if (not pitr) continue;
      // Electrons and muons
      if (MC::isStable(pitr) && (MC::isElectron(pitr) || MC::isMuon(pitr))) {
        if (pitr->momentum().perp() >= m_minPt && std::abs(pitr->momentum().pseudoRapidity()) <= m_maxEta) {
          ATH_MSG_DEBUG("Found lepton " << pitr);
          numLeptons++;
          numLightLeptons++;
          if (numLightLeptons==1) { if (pitr->pdg_id() < 0) { charge1 = -1; } else { charge1 = 1; } } 
          if (numLightLeptons==2) { if (pitr->pdg_id() < 0) { charge2 = -1; } else { charge2 = 1; } } 

        }
        continue;
      }

      // Look for Hadronic taus (leptonic ones will be saved by above) that have status!=3 and don't decay to another tau (FSR)
      if (!m_incHadTau) continue;
      HepMC::ConstGenParticlePtr   tau= nullptr;
      HepMC::ConstGenParticlePtr   taunu= nullptr;
      if (MC::isTau(pitr) || !MC::isPhysical(pitr)) continue;
        tau = pitr;
        if(!tau->end_vertex()) continue;
        // Loop over children and:
        // 1. Find if it is hadronic (i.e. no decay lepton).
        // 2. Veto tau -> tau (FSR)
        // 3. Store the tau neutino to calculate the visible momentum
        // cppcheck-suppress nullPointerRedundantCheck
        for (const auto& citr: *(tau->end_vertex())) {
          // Ignore tau -> tau (FSR)
          if (pitr->pdg_id() == citr->pdg_id()) {
            ATH_MSG_DEBUG("FSR tau decay.  Ignoring!");
            tau = nullptr;
            break;
          }

          // Ignore leptonic decays
          if (std::abs(citr->pdg_id()) == 13 || std::abs(citr->pdg_id()) == 11) {
            tau = nullptr;
            break;
          }

          // Find tau decay nu
          if (std::abs(citr->pdg_id()) == 16) {
            taunu = citr;
          }
        }

        if (tau and taunu) {
          // Good hadronic decay
          CLHEP::HepLorentzVector tauVisMom = CLHEP::HepLorentzVector(tau->momentum().px() - taunu->momentum().px(),
                                                        tau->momentum().py() - taunu->momentum().py(),
                                                        tau->momentum().pz() - taunu->momentum().pz(),
                                                        tau->momentum().e()  - taunu->momentum().e());
          if (tauVisMom.perp() >= m_minVisPtHadTau && std::abs(tauVisMom.pseudoRapidity()) <= m_maxEta) {
            ATH_MSG_DEBUG("Found hadronic tau decay " << tau);
            numLeptons++;
            numHadTaus++;
          }
        }
      }
 }

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
