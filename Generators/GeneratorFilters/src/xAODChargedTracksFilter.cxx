/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/xAODChargedTracksFilter.h"
#include "TruthUtils/HepMCHelpers.h"


StatusCode xAODChargedTracksFilter::filterInitialize()
{
    CHECK(m_truthPartContKey.initialize());
    return StatusCode::SUCCESS;
}


StatusCode xAODChargedTracksFilter::filterEvent() {

  // Retrieve TruthGen container from xAOD Gen slimmer, contains all particles witout barcode_zero and
  // duplicated barcode ones
  SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainer{m_truthPartContKey};
  CHECK(xTruthParticleContainer.isValid());


  int nChargedTracks = 0;
  // Loop over all particles in the event
  for (const xAOD::TruthParticle* part : *xTruthParticleContainer) {
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
