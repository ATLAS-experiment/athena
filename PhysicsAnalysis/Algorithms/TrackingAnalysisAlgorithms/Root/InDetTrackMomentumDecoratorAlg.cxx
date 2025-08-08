/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Daniel Werner

#include <TrackingAnalysisAlgorithms/InDetTrackMomentumDecoratorAlg.h>

namespace CP {

  StatusCode InDetTrackMomentumDecoratorAlg::initialize() {

    ANA_CHECK(m_tracksHandle.initialize (m_systematicsList));
    ANA_CHECK(m_momentumDecor.initialize (m_systematicsList, m_tracksHandle));
    ANA_CHECK(m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode InDetTrackMomentumDecoratorAlg::execute() {

    for (const auto &sys : m_systematicsList.systematicsVector()) {

      const xAOD::TrackParticleContainer *tracks = nullptr;
      ANA_CHECK(m_tracksHandle.retrieve (tracks, sys));

      for (const xAOD::TrackParticle *track : *tracks) {
        m_momentumDecor.set(*track, track->pt(), sys);
      }

    }

    return StatusCode::SUCCESS;
  }

} // namespace
