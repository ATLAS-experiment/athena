/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>
/// @author Steffen Korn <steffen.korn@cern.ch>

#ifndef PARTONS_PARTONHISTORYUTILS_H
#define PARTONS_PARTONHISTORYUTILS_H

#include "xAODTruth/TruthParticleContainer.h"

namespace CP {
using ROOT::Math::PtEtaPhiMVector;

namespace PartonHistoryUtils {
/// Return particle after FSR (before the decay vertex)
const xAOD::TruthParticle& findAfterFSR(const xAOD::TruthParticle& particle);

// determine whether potentialChild is child of parent
//
// A broken child truth link within nChildren() is treated as "not this child".
bool isChildOf(const xAOD::TruthParticle& parent,
               const xAOD::TruthParticle& potentialChild);

/// Looking for tops without children -> must be broken
bool isBrokenTop(const xAOD::TruthParticle& particle);

/// Determine whether particle is afterFSR
bool isAfterFSR(const xAOD::TruthParticle& particle);

/// Return true when any parent of the particle has the same pdgId as the particle
///
/// A broken parent truth link within nParents() is treated as "not identical".
bool hasParticleIdenticalParent(const xAOD::TruthParticle& particle);

// Checking whether a particle has the same pdgId as its parent
//
// A particle with no parent, or a broken truth link to it, is treated as "no parent".
bool hasParentPdgId(const xAOD::TruthParticle& particle, int PdgId);
bool hasParentPdgId(const xAOD::TruthParticle& particle);

// Checking whether a particle has the same absolute pdgId as absPdgId
//
// A particle with no parent, or a broken truth link to it, is treated as "no parent".
bool hasParentAbsPdgId(const xAOD::TruthParticle& particle, int absPdgId);

// Checking whether a particle has an identical child
//
// A broken child truth link within nChildren() is treated as "no such child".
bool hasIdenticalChild(const xAOD::TruthParticle& particle);

// Checking whether a particle is a quark from the PDF (massless)
bool isQuarkFromPDF(const xAOD::TruthParticle& particle);

}  // namespace PartonHistoryUtils
}  // namespace CP

#endif
