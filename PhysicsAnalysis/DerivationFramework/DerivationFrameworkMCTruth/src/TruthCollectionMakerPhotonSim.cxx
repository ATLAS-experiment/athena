/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TruthCollectionMakerPhotonSim.cxx
// Create truth photonsim collection decorated with photonsim decay specific variables

// Class header file
#include "DerivationFrameworkMCTruth/TruthCollectionMakerPhotonSim.h"
#include "TruthUtils/HepMCHelpers.h"
#include "AthContainers/ConstAccessor.h"
#include "GaudiKernel/SystemOfUnits.h"

std::vector<int> DerivationFramework::TruthCollectionMakerPhotonSim::updateMask(const xAOD::TruthParticleContainer* truthParticles) const {
  static const SG::ConstAccessor<unsigned int> classifierParticleOriginAcc("classifierParticleOrigin");
  std::vector<int> entries;
  entries.reserve(truthParticles->size());
  for (const auto* truthParticle : *truthParticles) {
    const unsigned int origin = classifierParticleOriginAcc(*truthParticle);
    entries.push_back( ((std::abs(truthParticle->pdgId()) == MC::PHOTON) && truthParticle->isGenStable() && ((origin != 42 && origin != 23 ) || (truthParticle->pt() > 20.0*Gaudi::Units::GeV))) ? 1 : 0);
  }
  return entries;
}
