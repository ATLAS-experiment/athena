/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak

#include <TauAnalysisAlgorithms/TauExtraVariablesAlg.h>

#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>


namespace CP {

  StatusCode TauExtraVariablesAlg::initialize() {

    if (m_nTracksKey.contHandleKey().key() == m_nTracksKey.key()) {
      m_nTracksKey = m_tausKey.key() + "." + m_nTracksKey.key();
    }

    ANA_CHECK(m_tausKey.initialize());
    ANA_CHECK(m_nTracksKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode TauExtraVariablesAlg::execute(const EventContext &ctx) const {

    SG::ReadHandle<xAOD::TauJetContainer> taus(m_tausKey, ctx);

    SG::WriteDecorHandle<xAOD::TauJetContainer, int> nTracksHandle(m_nTracksKey, ctx);
    for (const xAOD::TauJet *tau : *taus) {
      nTracksHandle(*tau) = tau->nTracks();
    }

    return StatusCode::SUCCESS;
  }

} // namespace
