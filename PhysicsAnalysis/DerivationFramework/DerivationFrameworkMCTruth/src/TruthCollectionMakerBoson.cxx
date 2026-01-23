/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TruthCollectionMakerBoson.cxx
// Create truth boson collection decorated with boson decay specific variables

// Class header file
#include "DerivationFrameworkMCTruth/TruthCollectionMakerBoson.h"
#include "TruthUtils/HepMCHelpers.h"

std::vector<int> DerivationFramework::TruthCollectionMakerBoson::updateMask(const xAOD::TruthParticleContainer* truthParticles) const {
  std::vector<int> entries;
  entries.reserve(truthParticles->size());
  for (const auto* truthParticle : *truthParticles) {
    entries.push_back( ((truthParticle->pdgId() == MC::Z0BOSON) || (std::abs(truthParticle->pdgId()) == MC::WPLUSBOSON) || (truthParticle->pdgId() == MC::HIGGSBOSON) ) ? 1 : 0);
  }
  return entries;
}
