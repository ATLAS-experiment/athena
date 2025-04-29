/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/xAODDirectPhotonFilter.h"
#include <algorithm>
#include "xAODTruth/TruthVertex.h"
#include "TruthUtils/HepMCHelpers.h"

StatusCode xAODDirectPhotonFilter::filterInitialize() {

  CHECK(m_truthPartContKey.initialize());

  ATH_MSG_INFO("Initialising DirectPhoton filter with OrderPhotons="<<m_OrderPhotons);

  if (m_Ptmin.size()>m_NPhotons || m_Ptmax.size()>m_NPhotons) {
    ATH_MSG_ERROR("Too many Ptmin/max for given NPhotons");
    return StatusCode::FAILURE;
  }

  if (!m_OrderPhotons && (m_Ptmin.size()>1 || m_Ptmax.size()>1)) {
    ATH_MSG_ERROR("Too many Ptmin/max requirements for OrderPhotons=false case.");
    return StatusCode::FAILURE;
  }

  // allow specifying only one pTmin/max to be applied to all (further) photons
  // for backward compatibility
  if (m_Ptmin.size()<m_NPhotons) {
    size_t origsize = m_Ptmin.size();
    double lastPt = m_Ptmin.value().back();
    m_Ptmin.value().resize(m_NPhotons);
    for (size_t i=origsize; i<m_NPhotons; ++i) m_Ptmin[i]=lastPt;
  }
  if (m_Ptmax.size()<m_NPhotons) {
    size_t origsize = m_Ptmax.size();
    double lastPt = m_Ptmax.value().back();
    m_Ptmax.value().resize(m_NPhotons);
    for (size_t i=origsize; i<m_NPhotons; ++i) m_Ptmax[i]=lastPt;
  }
  return StatusCode::SUCCESS;
}

bool xAODDirectPhotonFilterCmpByPt(const xAOD::TruthParticle* p1, const xAOD::TruthParticle* p2) {
  return (p1->pt()>p2->pt());
}

StatusCode xAODDirectPhotonFilter::filterEvent() {

  std::vector<const xAOD::TruthParticle*> promptPhotonsInEta;

  int phot = 0;
  // Retrieve TruthGen container from xAOD Gen slimmer, contains all particles witout barcode_zero and
  // duplicated barcode ones
  SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainer{m_truthPartContKey};
  CHECK(xTruthParticleContainer.isValid());

  // Loop over all particles in the event and find photons in given eta range
  for (const xAOD::TruthParticle* pitr : *xTruthParticleContainer) {
      if (MC::isPhoton(pitr) &&
          MC::isStable(pitr) &&
          std::abs(pitr->eta()) <= m_EtaRange) {
        
        // iterate over parent particles to exclude photons from hadron decays
        bool fromHadron(false);
        for(size_t thisParent_id=0; thisParent_id < pitr->prodVtx()->nIncomingParticles(); thisParent_id++)
        { 
          auto parent = pitr->prodVtx()->incomingParticle(thisParent_id);
          int pdgindex =  std::abs(parent->pdgId()); 
          ATH_MSG_DEBUG("Looping on Production (parents) vertex : " << parent->pdgId());
          if (pdgindex > 100) {
            fromHadron = true;
            if (m_AllowSUSYDecay && ( (pdgindex > 1000000 && pdgindex < 1000040) || (pdgindex > 2000000 && pdgindex < 2000016) ) ) fromHadron = false;
          }
        }
        phot++;
        if (!fromHadron) promptPhotonsInEta.push_back(pitr);
        else ATH_MSG_DEBUG("non-prompt photon ignored");
      }
    } //loop over TruthParticles
  

  ATH_MSG_DEBUG("number of photons" << phot);

  if (promptPhotonsInEta.size()<m_NPhotons) {
    setFilterPassed(false);
  }
  else {
    for (const auto& photon: promptPhotonsInEta) {

      ATH_MSG_DEBUG("Found prompt photon with pt="<<photon->pt());
    }
    if (m_OrderPhotons) { // apply cuts to leading/subleading/... photon as specified
      std::sort(promptPhotonsInEta.begin(), promptPhotonsInEta.end(), xAODDirectPhotonFilterCmpByPt);

      bool pass = true;
      for (size_t i = 0; i < m_NPhotons; ++i) {
        double pt = promptPhotonsInEta[i]->pt();
        if (pt < m_Ptmin[i] || pt > m_Ptmax[i]) {

          ATH_MSG_DEBUG("  rejected pt="<<pt);
          pass = false;
        }
      }

      if (pass) {
         ATH_MSG_DEBUG("Passed!");
         }
      setFilterPassed(pass);
    }
    else { // just require NPhotons to pass m_Ptmin/max[0]
      size_t NPhotons=0;
      for (size_t i = 0; i < promptPhotonsInEta.size(); ++i) {
        double pt = promptPhotonsInEta[i]->pt();
        if (pt > m_Ptmin[0] && pt < m_Ptmax[0]) ++NPhotons;
      }

      if (NPhotons>=m_NPhotons) ATH_MSG_DEBUG("Passed!");
      setFilterPassed(NPhotons>=m_NPhotons);
    }
  }
  return StatusCode::SUCCESS;
}
