/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "WritexAOD.h"

StatusCode WritexAOD::initialize() {

  // Initialise the data handles
  ATH_CHECK(m_trackKey.initialize());
  ATH_CHECK(m_newKey.initialize());
  // Initialise the tool handles
  ATH_CHECK(m_trackSelectionTool.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode WritexAOD::execute(const EventContext &ctx) const {

  // Get the particle containers requested
  // EventContext is for multi-threading
  SG::ReadHandle<xAOD::TrackParticleContainer> tracksIn{m_trackKey, ctx};
  if (!tracksIn.isValid()) {
    ATH_MSG_ERROR("Couldn't retrieve xAOD::TrackParticles with key: "
                  << m_trackKey.key());
    return StatusCode::FAILURE;
  }

  // Create the new container
  // Note the creation of the aux container, for the data payload, in addition
  // to the object container
  SG::WriteHandle<xAOD::TrackParticleContainer> tracksOut{m_newKey, ctx};
  ATH_CHECK(
      tracksOut.record(std::make_unique<xAOD::TrackParticleContainer>(),
                       std::make_unique<xAOD::TrackParticleAuxContainer>()));

  // Loop over the tracks, select those passing selections, push a copy into the
  // output container
  for (const xAOD::TrackParticle *track : *tracksIn) {
    if (m_trackSelectionTool->accept(*track)) {
      xAOD::TrackParticle* newTrack = tracksOut->push_back (std::make_unique<xAOD::TrackParticle>());
      *newTrack = *track;
    }
  }

  return StatusCode::SUCCESS;
}