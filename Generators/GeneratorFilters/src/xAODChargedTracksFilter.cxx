/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/xAODChargedTracksFilter.h"
#include "TruthUtils/HepMCHelpers.h"


xAODChargedTracksFilter::xAODChargedTracksFilter(const std::string& name, ISvcLocator* pSvcLocator)
  : GenFilter(name, pSvcLocator)
{
  /// @todo Document
  declareProperty("Ptcut", m_Ptmin = 50.0);
  declareProperty("Etacut", m_EtaRange = 2.5);
  declareProperty("NTracks", m_NTracks = 40);
}


StatusCode xAODChargedTracksFilter::filterEvent() {

  // Retrieve TruthGen container from xAOD Gen slimmer, contains all particles witout barcode_zero and 
// duplicated barcode ones
  const xAOD::TruthParticleContainer* xTruthParticleContainer;
  if (evtStore()->retrieve(xTruthParticleContainer, "TruthGen").isFailure()) {
      ATH_MSG_ERROR("No TruthParticle collection with name " << "TruthGen" << " found in StoreGate!");
      return StatusCode::FAILURE;
  }


    int nChargedTracks = 0;
  unsigned int nPart = xTruthParticleContainer->size();
  for (unsigned int iPart = 0; iPart < nPart; ++iPart) {
            const xAOD::TruthParticle* part =  (*xTruthParticleContainer)[iPart];
            // We only care about stable particles
            if (!part->isGenStable()) continue;

            // Particle's charge
            int pID = part->pdgId();
            double pCharge = MC::charge(pID);

            // Count tracks in specified acceptance
            const double pT = part->pt();
            const double eta = part->eta();
            if (pT >= m_Ptmin && std::abs(eta) <= m_EtaRange && pCharge != 0) {
                ATH_MSG_DEBUG("Found particle, " <<
                            " pT = " << pT <<
                            " eta = " << eta <<
                            " pdg id = " << pID <<
                            " charge = " << pCharge << " in acceptance");
                nChargedTracks += 1;
            }

        } //end loop on particles

    // Summarise event
    ATH_MSG_DEBUG("# of tracks " << nChargedTracks <<
                " with pT >= " << m_Ptmin <<
                " |eta| < " << m_EtaRange <<
                " minNTracks = " << m_NTracks);

    // Record passed status
    setFilterPassed(nChargedTracks > m_NTracks);
    return StatusCode::SUCCESS;
}
