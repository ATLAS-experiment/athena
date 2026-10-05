/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/TauEleOverlapChecker.h"

TauEleOverlapChecker::TauEleOverlapChecker(const std::string& name) :
    TauRecToolBase(name) {
}

StatusCode TauEleOverlapChecker::initialize()  {
    ATH_CHECK( m_removedClustersContainer.initialize() );
    ATH_CHECK( m_removedTracksContainer.initialize() );
    return StatusCode::SUCCESS;
}

StatusCode TauEleOverlapChecker::executeTool(xAOD::TauJet& tau,
					     const EventContext& ctx) const {
    // Checking if the seed jet is valid
    auto jet_seed = tau.jet();
    if (jet_seed == nullptr) {
        ATH_MSG_ERROR("Tau jet link is invalid.");
        return StatusCode::FAILURE;
    }
    // retrieve the input removed tracks and clusters containers
    SG::ReadHandle<xAOD::CaloClusterContainer>   removedClustersHandle(m_removedClustersContainer, ctx);
    SG::ReadHandle<xAOD::TrackParticleContainer> removedTracksHandle(m_removedTracksContainer, ctx);
    if (!removedClustersHandle.isValid() || !removedTracksHandle.isValid()) {
        ATH_MSG_ERROR (
            "Could not retrieve HiveDataObj with key " << 
            (!removedClustersHandle.isValid() ? removedClustersHandle.key() : "") <<
            (!removedTracksHandle.isValid()   ? removedTracksHandle.key()   : "") 
        );
        return StatusCode::FAILURE;
    }
    const xAOD::CaloClusterContainer   *removed_clusters_cont = removedClustersHandle.cptr();
    const xAOD::TrackParticleContainer *removed_tracks_cont   = removedTracksHandle.cptr();

    for (const auto* removal_direction : *removed_tracks_cont) {
        if (removal_direction->p4().DeltaR(jet_seed->p4()) < m_checkingCone) {
            return StatusCode::SUCCESS;
        }
    }
    for (const auto* removal_direction : *removed_clusters_cont) {
        if (removal_direction->p4().DeltaR(jet_seed->p4()) < m_checkingCone) {
            return StatusCode::SUCCESS;
        }
    }
    ATH_MSG_DEBUG("TauJet do not overlap with removal direction, skipping...");
    return StatusCode::FAILURE;
}
