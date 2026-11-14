/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCTRUTHCLASSIFIER_MCTRUTHCLASSIFIERGEN_H
#define MCTRUTHCLASSIFIER_MCTRUTHCLASSIFIERGEN_H
/*
 * Declarations of the free functions implementing the classification of
 * truth particles (see Root/MCTruthClassifierGen.cxx).
 */

#include "TruthUtils/TruthClasses.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include <utility>

namespace MCTruthPartClassifier {

/// Classify thePart (not nullptr), using truthParticleContainer to look up its ancestors.
/// Fills outCome with what happened to the particle in the simulation.
std::pair<ParticleType, ParticleOrigin>
classifyTruthParticle(const xAOD::TruthParticleContainer& truthParticleContainer,
                      const xAOD::TruthParticle* const thePart,
                      ParticleOutCome& outCome);

} // namespace MCTruthPartClassifier

#endif // MCTRUTHCLASSIFIER_MCTRUTHCLASSIFIERGEN_H
