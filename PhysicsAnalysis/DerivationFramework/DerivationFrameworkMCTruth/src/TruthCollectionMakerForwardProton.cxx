/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TruthCollectionMakerForwardProton.cxx
// Create truth forwardproton collection decorated with forwardproton decay specific variables

// Class header file
#include "DerivationFrameworkMCTruth/TruthCollectionMakerForwardProton.h"
#include "TruthUtils/HepMCHelpers.h"

std::vector<int> DerivationFramework::TruthCollectionMakerForwardProton::updateMask(const xAOD::TruthParticleContainer* truthParticles) const {
  std::vector<int> entries;
  entries.reserve(truthParticles->size());
  for (const auto* truthParticle : *truthParticles) {
    entries.push_back( ((std::abs(truthParticle->pdgId()) == MC::PROTON) && truthParticle->isStable() && truthParticle->e() > 0.8*m_beamEnergy) ? 1 : 0); // TODO Check whether isGenStable was intended here?
  }
  return entries;
}
