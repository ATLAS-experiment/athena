/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TruthCollectionMakerBottom.cxx
// Create truth bottom collection decorated with bottom decay specific variables

// Class header file
#include "DerivationFrameworkMCTruth/TruthCollectionMakerBottom.h"
#include "TruthUtils/HepMCHelpers.h"

std::vector<int> DerivationFramework::TruthCollectionMakerBottom::updateMask(const xAOD::TruthParticleContainer* truthParticles) const {
  std::vector<int> entries;
  entries.reserve(truthParticles->size());
  for (const auto* truthParticle : *truthParticles) {
    entries.push_back( (std::abs(truthParticle->pdgId()) == MC::BQUARK) ? 1 : 0);
  }
  return entries;
}
