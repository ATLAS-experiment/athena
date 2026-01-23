/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TruthCollectionMakerNeutrino.cxx
// Create truth neutrino collection decorated with neutrino decay specific variables

// Class header file
#include "DerivationFrameworkMCTruth/TruthCollectionMakerNeutrino.h"
#include "TruthUtils/HepMCHelpers.h"

std::vector<int> DerivationFramework::TruthCollectionMakerNeutrino::updateMask(const xAOD::TruthParticleContainer* truthParticles) const {
  std::vector<int> entries;
  entries.reserve(truthParticles->size());
  for (const auto* truthParticle : *truthParticles) {
    entries.push_back( (truthParticle->isNeutrino() && truthParticle->isGenStable()) ? 1 : 0);
  }
  return entries;
}
