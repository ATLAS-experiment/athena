/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak

#include <TauAnalysisAlgorithms/TauExtraVariablesAlg.h>

#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>


namespace CP {

  StatusCode TauExtraVariablesAlg::initialize() {

    if (m_nTracksChargedKey.contHandleKey().key() == m_nTracksChargedKey.key()) {
      m_nTracksChargedKey = m_tausKey.key() + "." + m_nTracksChargedKey.key();
    }

    ANA_CHECK(m_tausKey.initialize());
    ANA_CHECK(m_nTracksChargedKey.initialize());

    // register type for output
    SG::ConstAccessor<int> (m_nTracksChargedKey.key().substr (m_nTracksChargedKey.key().find_last_of(".") + 1));

    return StatusCode::SUCCESS;
  }

  StatusCode TauExtraVariablesAlg::execute(const EventContext &ctx) const {

    SG::ReadHandle<xAOD::TauJetContainer> taus(m_tausKey, ctx);

    SG::WriteDecorHandle<xAOD::TauJetContainer, int> nTracksChargedHandle(m_nTracksChargedKey, ctx);
    for (const xAOD::TauJet *tau : *taus) {
      nTracksChargedHandle(*tau) = tau->nTracksCharged();
    }

    return StatusCode::SUCCESS;
  }

} // namespace
