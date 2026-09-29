/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#include "TruthParticleLevelAnalysisAlgorithms/ParticleLevelMissingETAlg.h"

#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

namespace CP {

StatusCode ParticleLevelMissingETAlg::initialize() {

  ANA_CHECK(m_metKey.initialize());
  ANA_CHECK(m_decMetKey.initialize());
  ANA_CHECK(m_decPhiKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode ParticleLevelMissingETAlg::execute(const EventContext &ctx) const {

  SG::ReadHandle<xAOD::MissingETContainer> met(m_metKey, ctx);

  // decorators
  SG::WriteDecorHandle<xAOD::MissingETContainer, float> dec_phi(m_decPhiKey, ctx);
  SG::WriteDecorHandle<xAOD::MissingETContainer, float> dec_met(m_decMetKey, ctx);

  for (const auto* etmiss : *met) {
    dec_met(*etmiss) = etmiss->met();
    dec_phi(*etmiss) = etmiss->phi();
  }

  return StatusCode::SUCCESS;
}

}  // namespace CP
