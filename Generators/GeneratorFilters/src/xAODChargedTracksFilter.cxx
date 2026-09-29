/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorFilters/xAODChargedTracksFilter.h"
#include "TruthUtils/HepMCHelpers.h"
#include "xAODTruth/TruthVertex.h"


StatusCode xAODChargedTracksFilter::filterInitialize()
{
    CHECK(m_truthPartContKey.initialize());
    return StatusCode::SUCCESS;
}


StatusCode xAODChargedTracksFilter::filterEvent(const EventContext& ctx) {

  // Retrieve TruthGen container from xAOD Gen slimmer, contains all particles witout barcode_zero and
  // duplicated barcode ones
  SG::ReadHandle<xAOD::TruthParticleContainer> xTruthParticleContainer{m_truthPartContKey, ctx};
  CHECK(xTruthParticleContainer.isValid());


  int nChargedTracks = 0;
  // Loop over all particles in the event
  for (const xAOD::TruthParticle* part : *xTruthParticleContainer) {
            // We only care about stable particles
            if (!part->isGenStable()) continue;

            // Particle's charge
            int pID = part->pdgId();
            double pCharge = MC::charge(pID);
            if (pCharge==0) continue;

            // Count tracks in specified acceptance
            const double pT = part->pt();
            const double eta = part->eta();

            // Skip explicitly excluded particle types, and particles whose
            // decay ancestry passes through an excluded particle (e.g. a
            // stable pion from a decayed tau that is itself excluded)
            if (isExcludedParticle(pID, pT)) {
                ATH_MSG_DEBUG("  -> excluded directly: pdgId = " << pID << " pt = " << pT);
                continue;
            }
            if (hasExcludedAncestor(part)) {
                ATH_MSG_DEBUG("  -> excluded via ancestor: pdgId = " << pID << " pt = " << pT);
                continue;
            }

            if (pT >= m_Ptmin && std::abs(eta) <= m_EtaRange) {
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
                " minNTracks = " << m_NTracks <<
                " maxNTracks = " << m_NTracksMax);

    // Record passed status
    setFilterPassed((m_NTracksMax > 0) ?
        (nChargedTracks >= m_NTracks && nChargedTracks <= m_NTracksMax) :
        (nChargedTracks >= m_NTracks), ctx);
    return StatusCode::SUCCESS;
}

bool xAODChargedTracksFilter::isExcludedParticle(int pdgId, double pt) const {

    const auto & excluded = m_excludedPdgIdPtMin.value();
    auto it = excluded.find(std::abs(pdgId));
    if (it == excluded.end()) {
        ATH_MSG_VERBOSE("    isExcludedParticle: pdgId = " << pdgId <<
                    " not in ExcludedPdgIdPtMin map");
        return false;
    }
    const bool excludedByPt = (pt >= it->second);
    ATH_MSG_DEBUG("    isExcludedParticle: pdgId = " << pdgId <<
                " pt = " << pt << " threshold = " << it->second <<
                " -> " << (excludedByPt ? "EXCLUDED" : "kept (below threshold)"));
    return excludedByPt;
}

bool xAODChargedTracksFilter::hasExcludedAncestor(const xAOD::TruthParticle* part) const {

    // Walk up the decay chain (not just the immediate parent) so that
    // daughters produced via an intermediate resonance (e.g. tau -> rho ->
    // pi pi) are still caught if the tau itself is excluded
    ATH_MSG_DEBUG("  Walking ancestry for daughter pdgId = " << part->pdgId() <<
                " pt = " << part->pt());
    const xAOD::TruthVertex* prodVtx = part->prodVtx();
    while (prodVtx && prodVtx->nIncomingParticles() > 0) {
        const xAOD::TruthParticle* parent = prodVtx->incomingParticle(0);
        if (!parent) break;
        ATH_MSG_DEBUG("    ancestor: pdgId = " << parent->pdgId() <<
                    " pt = " << parent->pt() <<
                    " eta = " << parent->eta() <<
                    " isGenStable = " << parent->isGenStable());
        if (!parent->isGenStable() && isExcludedParticle(parent->pdgId(), parent->pt())) {
            ATH_MSG_DEBUG("    -> ancestor pdgId = " << parent->pdgId() <<
                        " pt = " << parent->pt() << " matches exclusion criteria");
            return true;
        }
        prodVtx = parent->prodVtx();
    }
    return false;
}
