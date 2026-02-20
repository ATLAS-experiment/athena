/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TruthCollectionMakerPhoton.cxx
// Create truth photon collection decorated with photon decay specific variables

// Class header file
#include "DerivationFrameworkMCTruth/TruthCollectionMakerPhoton.h"
#include "TruthUtils/HepMCHelpers.h"

std::vector<int> DerivationFramework::TruthCollectionMakerPhoton::updateMask(const xAOD::TruthParticleContainer* truthParticles) const {
  std::vector<int> entries;
  entries.reserve(truthParticles->size());
  for (const auto* truthParticle : *truthParticles) {
    entries.push_back( ((std::abs(truthParticle->pdgId()) == MC::PHOTON) && truthParticle->isGenStable()) ? 1 : 0);
  }
  return entries;
}
