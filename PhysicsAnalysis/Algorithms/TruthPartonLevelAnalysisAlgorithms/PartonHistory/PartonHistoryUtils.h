/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>
/// @author Steffen Korn <steffen.korn@cern.ch>

#ifndef PARTONS_TOPPARTONUTILS_H
#define PARTONS_TOPPARTONUTILS_H

#include "xAODTruth/TruthParticleContainer.h"

namespace CP {
using ROOT::Math::PtEtaPhiMVector;

namespace PartonHistoryUtils {
/// Return particle after FSR (before the decay vertex)
const xAOD::TruthParticle* findAfterFSR(const xAOD::TruthParticle* particle);

// determine whether potentialChild is child of parent
bool isChildOf(const xAOD::TruthParticle* parent,
               const xAOD::TruthParticle* potentialChild);

/// Looking for tops without children -> must be broken
bool isBrokenTop(const xAOD::TruthParticle* particle);

/// Determine whether particle is afterFSR
bool isAfterFSR(const xAOD::TruthParticle* particle);

/// Return true when particle is a top before FSR
bool hasParticleIdenticalParent(const xAOD::TruthParticle* particle);

// Checking whether a particle has the same pdgId as its parent
bool hasParentPdgId(const xAOD::TruthParticle* particle, int PdgId);
bool hasParentPdgId(const xAOD::TruthParticle* particle);

// Checking whether a particle has the same absolute pdgId as absPdgId
bool hasParentAbsPdgId(const xAOD::TruthParticle* particle, int absPdgId);

// Checking whether a particle has an identical child
bool hasIdenticalChild(const xAOD::TruthParticle* particle);

// Checking whether a particle is a quark from the PDF (massless)
bool isQuarkFromPDF(const xAOD::TruthParticle* particle);

}  // namespace PartonHistoryUtils
}  // namespace CP

#endif
