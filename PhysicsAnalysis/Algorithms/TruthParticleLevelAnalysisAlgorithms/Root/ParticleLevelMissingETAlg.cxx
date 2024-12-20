/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#include "TruthParticleLevelAnalysisAlgorithms/ParticleLevelMissingETAlg.h"

namespace CP {

StatusCode ParticleLevelMissingETAlg::initialize() {

  ANA_CHECK(m_metKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode ParticleLevelMissingETAlg::execute() {

  SG::ReadHandle<xAOD::MissingETContainer> met(m_metKey);

  // decorators
  static const SG::AuxElement::Decorator<float> dec_phi("phi");
  static const SG::AuxElement::Decorator<float> dec_met("met");

  for (const auto* etmiss : *met) {
    dec_met(*etmiss) = etmiss->met();
    dec_phi(*etmiss) = etmiss->phi();
  }

  return StatusCode::SUCCESS;
}

}  // namespace CP
