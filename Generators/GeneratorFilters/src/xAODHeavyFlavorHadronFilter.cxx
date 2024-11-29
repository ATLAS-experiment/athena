/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/xAODHeavyFlavorHadronFilter.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "xAODJet/JetContainer.h"
#include "CxxUtils/BasicTypes.h"
#include "TruthUtils/HepMCHelpers.h"
#include <cmath>


xAODHeavyFlavorHadronFilter::xAODHeavyFlavorHadronFilter(const std::string& name, ISvcLocator* pSvcLocator)
  : GenFilter(name, pSvcLocator),
    m_NPass(0), m_Nevt(0), m_NbPass(0), 
    m_NcPass(0), m_NBHadronPass(0), m_NDHadronPass(0), 
    m_NPDGIDPass(0)
{
  
}


StatusCode xAODHeavyFlavorHadronFilter::filterInitialize() {
  m_Nevt = 0;
  m_NPass = 0;
  m_NbPass = 0;
  m_NcPass = 0;
  m_NBHadronPass = 0;
  m_NDHadronPass = 0;
  m_NPDGIDPass = 0;
  return StatusCode::SUCCESS;
}


StatusCode xAODHeavyFlavorHadronFilter::filterFinalize() {
  ATH_MSG_INFO(m_NPass << " events out of " << m_Nevt << " passed the filter");
  if (m_Request_bQuark) ATH_MSG_INFO(m_NbPass << " passed b-quark selection");
  if (m_Request_cQuark) ATH_MSG_INFO(m_NcPass << " passed c-quark selection");
  if (m_RequestBottom) ATH_MSG_INFO(m_NBHadronPass << " passed B Hadron selection");
  if (m_RequestCharm) ATH_MSG_INFO(m_NDHadronPass << " passed Charm Hadron selection");
  if (m_RequestSpecificPDGID) ATH_MSG_INFO(m_NPDGIDPass << " passed selection for PDG code " << m_PDGID);
  return StatusCode::SUCCESS;
}


StatusCode xAODHeavyFlavorHadronFilter::filterEvent() {
  bool pass = false;
  bool bPass = false;
  bool cPass = false;
  bool BHadronPass = false;
  bool DHadronPass = false;
  bool PDGIDPass = false;

  m_Nevt++;

  std::vector<xAOD::JetContainer::const_iterator> jets;
  if (m_RequireTruthJet) {
    const xAOD::JetContainer* truthjetTES;
    CHECK(evtStore()->retrieve( truthjetTES, m_TruthJetContainerName));
    for (xAOD::JetContainer::const_iterator j = truthjetTES->begin(); j != truthjetTES->end() ; ++j) {
      if ((*j)->pt() > m_jetPtMin && std::abs((*j)->eta()) < m_jetEtaMax) {
        jets.push_back(j);
      }
    }
  }

  // Retrieve TruthGen container from xAOD Gen slimmer, contains all particles witout barcode_zero and 
// duplicated barcode ones
  const xAOD::TruthParticleContainer* xTruthParticleContainer;
  if (evtStore()->retrieve(xTruthParticleContainer, "TruthGen").isFailure()) {
      ATH_MSG_ERROR("No TruthParticle collection with name " << "TruthGen" << " found in StoreGate!");
      return StatusCode::FAILURE;
  }

  unsigned int nPart = xTruthParticleContainer->size();
  for (unsigned int iPart = 0; iPart < nPart; ++iPart) {
      const xAOD::TruthParticle* part =  (*xTruthParticleContainer)[iPart];
 
      // b-quarks
      // ==========
      // Warning:  Since no navigation is done, the filter does not distinguish
      // between the final quark in the decay chain and intermediates
      // That means the code is NOT appropriate for counting the number
      // of heavy flavor quarks!
      if (m_Request_bQuark && std::abs(part->pdgId())==5 &&
          part->pt()> m_bPtMin &&
          std::abs(part->rapidity())<m_bEtaMax) {
        if (m_RequireTruthJet) {
          TLorentzVector genpart(part->px(), part->py(), part->pz(), part->e());
          for (uint i=0; i<jets.size(); i++) {
            double dR = (*jets[i])->p4().DeltaR(genpart);
            if (dR<m_deltaRFromTruth) bPass=true;
          }
        } else {
          bPass = true;
        }
      }

      // c-quarks
      // ========
      // Warning:  Since no navigation is done, the filter does not distinguish
      // between the final quark in the decay chain and intermediates
      // That means the code is NOT appropriate for counting the number
      // of heavy flavor quarks!
      if (m_Request_cQuark &&
          std::abs(part->pdgId())==4 &&
          part->pt()>m_cPtMin &&
          std::abs(part->rapidity())<m_cEtaMax) {
        if (m_RequireTruthJet) {
          TLorentzVector genpart(part->px(), part->py(), part->pz(), part->e());
          for (uint i=0; i<jets.size(); i++) {
            double dR = (*jets[i])->p4().DeltaR(genpart);
            if (dR<m_deltaRFromTruth) cPass=true;
          }
        } else {
          cPass = true;
        }
      }

      // B hadrons
      // =========
      if (m_RequestBottom &&
          MC::isWeaklyDecayingBHadron(part) &&
          part->pt()>m_bottomPtMin &&
          std::abs(part->rapidity())<m_bottomEtaMax) {
        if (m_RequireTruthJet) {
          TLorentzVector genpart(part->px(), part->py(), part->pz(), part->e());
          for (uint i=0; i<jets.size(); i++) {
            double dR = (*jets[i])->p4().DeltaR(genpart);
            if (dR < m_deltaRFromTruth) BHadronPass=true;
          }
        } else {
          BHadronPass = true;
        }
      }

      // Charm Hadrons
      // ==============
      if (m_RequestCharm &&
          MC::isWeaklyDecayingCHadron(part) &&
          part->pt()>m_charmPtMin &&
          std::abs(part->rapidity())<m_charmEtaMax) {
        if (m_RequireTruthJet) {
          TLorentzVector genpart(part->px(), part->py(), part->pz(), part->e());
          for (uint i=0; i<jets.size(); i++) {
            double dR = (*jets[i])->p4().DeltaR(genpart);
            if (dR < m_deltaRFromTruth) DHadronPass=true;
          }
        } else {
          DHadronPass = true;
        }
      }

      // Request Specific PDGID
      // =========================
      bool pdgok = m_RequestSpecificPDGID &&
        (part->pdgId() == m_PDGID ||
         (m_PDGAntiParticleToo && std::abs(part->pdgId()) == m_PDGID));
      if (pdgok && part->pt() > m_PDGPtMin &&
          std::abs(part->rapidity()) < m_PDGEtaMax) {
        if (m_RequireTruthJet) {
          TLorentzVector genpart(part->px(), part->py(), part->pz(), part->e());
          for (size_t i = 0; i < jets.size(); ++i) {
            double dR = (*jets[i])->p4().DeltaR(genpart);
            if (dR < m_deltaRFromTruth) PDGIDPass = true;
          }
        } else {
          PDGIDPass = true;
        }
      }
    }//end of Particle loop

  /// @todo This could be so much more efficient!
  pass = BHadronPass || DHadronPass || bPass || cPass || PDGIDPass;
  if (pass)  m_NPass++;
  if (bPass) m_NbPass++;
  if (cPass) m_NcPass++;
  if (BHadronPass) m_NBHadronPass++;
  if (DHadronPass) m_NDHadronPass++;
  if (PDGIDPass)   m_NPDGIDPass++;
  setFilterPassed(pass);

  return StatusCode::SUCCESS;
}
