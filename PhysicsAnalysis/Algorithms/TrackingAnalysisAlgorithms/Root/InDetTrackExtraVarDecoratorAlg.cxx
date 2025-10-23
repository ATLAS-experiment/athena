/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Daniel Werner

#include <TrackingAnalysisAlgorithms/InDetTrackExtraVarDecoratorAlg.h>

namespace CP {

  StatusCode InDetTrackExtraVarDecoratorAlg::initialize() {

    ANA_CHECK(m_tracksHandle.initialize (m_systematicsList));
    ANA_CHECK(m_momentumDecor.initialize (m_systematicsList, m_tracksHandle));
    ANA_CHECK(m_etaDecor.initialize (m_systematicsList, m_tracksHandle));
    ANA_CHECK(m_chargeDecor.initialize (m_systematicsList, m_tracksHandle));
    ANA_CHECK(m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode InDetTrackExtraVarDecoratorAlg::execute() {

    for (const auto &sys : m_systematicsList.systematicsVector()) {

      const xAOD::TrackParticleContainer *tracks = nullptr;
      ANA_CHECK(m_tracksHandle.retrieve (tracks, sys));

      for (const xAOD::TrackParticle *track : *tracks) {
        m_momentumDecor.set( *track, track->pt(),     sys );
        m_etaDecor.set(      *track, track->eta(),    sys );
        m_chargeDecor.set(   *track, track->charge(), sys );
      }

    }

    return StatusCode::SUCCESS;
  }

} // namespace
