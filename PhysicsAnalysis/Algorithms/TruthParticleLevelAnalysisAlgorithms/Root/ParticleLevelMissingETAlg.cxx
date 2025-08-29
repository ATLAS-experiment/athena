/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#include "TruthParticleLevelAnalysisAlgorithms/ParticleLevelMissingETAlg.h"

namespace CP {

StatusCode ParticleLevelMissingETAlg::initialize() {

  ANA_CHECK(m_metKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode ParticleLevelMissingETAlg::execute(const EventContext &ctx) const {

  SG::ReadHandle<xAOD::MissingETContainer> met(m_metKey, ctx);

  // decorators
  static const SG::Decorator<float> dec_phi("phi");
  static const SG::Decorator<float> dec_met("met");

  for (const auto* etmiss : *met) {
    dec_met(*etmiss) = etmiss->met();
    dec_phi(*etmiss) = etmiss->phi();
  }

  return StatusCode::SUCCESS;
}

}  // namespace CP
