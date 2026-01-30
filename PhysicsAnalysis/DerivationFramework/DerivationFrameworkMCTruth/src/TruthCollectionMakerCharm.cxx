/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TruthCollectionMakerCharm.cxx
// Create truth charm collection decorated with charm decay specific variables

// Class header file
#include "DerivationFrameworkMCTruth/TruthCollectionMakerCharm.h"
#include "TruthUtils/HepMCHelpers.h"

std::vector<int> DerivationFramework::TruthCollectionMakerCharm::updateMask(const xAOD::TruthParticleContainer* truthParticles) const {
  std::vector<int> entries;
  entries.reserve(truthParticles->size());
  for (const auto* truthParticle : *truthParticles) {
    entries.push_back( (std::abs(truthParticle->pdgId()) == MC::CQUARK) ? 1 : 0);
  }
  return entries;
}
