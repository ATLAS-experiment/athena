/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TruthCollectionMakerBSM.cxx
// Create truth bsm collection decorated with bsm decay specific variables

// Class header file
#include "DerivationFrameworkMCTruth/TruthCollectionMakerBSM.h"
#include "TruthUtils/HepMCHelpers.h"

std::vector<int> DerivationFramework::TruthCollectionMakerBSM::updateMask(const xAOD::TruthParticleContainer* truthParticles) const {
  std::vector<int> entries;
  entries.reserve(truthParticles->size());
  for (const auto* truthParticle : *truthParticles) {
    entries.push_back( (truthParticle->isBSM()) ? 1 : 0);
  }
  return entries;
}
