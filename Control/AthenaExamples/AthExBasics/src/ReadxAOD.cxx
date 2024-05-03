/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ReadxAOD.h"

ReadxAOD::ReadxAOD(const std::string &name, ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {
}

StatusCode ReadxAOD::initialize() {

    // Initialise the data handles
    ATH_CHECK(m_trackKey.initialize());
    // Initialise the tool handles
    ATH_CHECK(m_trackSelectionTool.retrieve());

    // Initialise the counters
    m_nTracksBelow = 0;
    m_nTracksAbove = 0;

    return StatusCode::SUCCESS;
}

StatusCode ReadxAOD::execute(const EventContext& ctx) const {

    // Get the particle containers requested
    // EventContext is for multi-threading
    SG::ReadHandle<xAOD::TrackParticleContainer> tracks{m_trackKey, ctx};
    if( ! tracks.isValid() ) {
        ATH_MSG_ERROR ("Couldn't retrieve xAOD::TrackParticles with key: " << m_trackKey.key() );
        return StatusCode::FAILURE;
    }

    // Loop over the tracks, select those passing selections, put into bins for final print-out
    for ( const xAOD::TrackParticle* track : *tracks) {
        if (m_trackSelectionTool->accept(*track)) {
            float pT = track->pt();
            if (pT <= m_cut) ++m_nTracksBelow;
            if (pT > m_cut) ++m_nTracksAbove;            
        }
    }

    return StatusCode::SUCCESS;

}

// Print the contents of the map
StatusCode ReadxAOD::finalize() {
    ATH_MSG_INFO("=======================");
    ATH_MSG_INFO("SUMMARY OF TRACK COUNTS");
    ATH_MSG_INFO("=======================");
    ATH_MSG_INFO("Number of tracks below the cut: " << std::to_string(m_nTracksBelow));
    ATH_MSG_INFO("Number of tracks above the cut: " << std::to_string(m_nTracksAbove));

    return StatusCode::SUCCESS;
}





