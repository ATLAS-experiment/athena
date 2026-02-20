/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TruthCollectionMakerTop.cxx
// Create truth top collection decorated with top decay specific variables

// Class header file
#include "DerivationFrameworkMCTruth/TruthCollectionMakerTop.h"
#include "TruthUtils/HepMCHelpers.h"

std::vector<int> DerivationFramework::TruthCollectionMakerTop::updateMask(const xAOD::TruthParticleContainer* truthParticles) const {
  std::vector<int> entries;
  entries.reserve(truthParticles->size());
  for (const auto* truthParticle : *truthParticles) {
    entries.push_back( (std::abs(truthParticle->pdgId()) == MC::TQUARK) ? 1 : 0);
  }
  return entries;
}
