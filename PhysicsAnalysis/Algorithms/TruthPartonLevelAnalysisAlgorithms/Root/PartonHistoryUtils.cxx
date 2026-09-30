/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>
/// @author Steffen Korn <steffen.korn@cern.ch>

#include "PartonHistory/PartonHistoryUtils.h"

#include <cmath>

namespace {
// Returns the first child with the same PDG ID as p, or nullptr if none.
// A broken truth link within nChildren() is treated like a missing child.
const xAOD::TruthParticle* findIdenticalChild(const xAOD::TruthParticle& p) {
  for (size_t i = 0; i < p.nChildren(); i++) {
    const xAOD::TruthParticle* child = p.child(i);
    if (child && child->pdgId() == p.pdgId())
      return child;
  }
  return nullptr;
}
}  // namespace

namespace CP {
namespace PartonHistoryUtils {

//////////////////////////////////////////////
////////// Parent - child relations //////////
//////////////////////////////////////////////

bool hasParentPdgId(const xAOD::TruthParticle& p, int pdgId) {
  // No parent recorded, or a broken truth link to it, is treated as "no
  // parent".
  if (p.nParents() == 0)
    return false;
  const xAOD::TruthParticle* parent = p.parent(0);
  // Checks if the parent of the given particle has a specific PDG ID.
  return parent && parent->pdgId() == pdgId;
}

bool hasParentPdgId(const xAOD::TruthParticle& p) {
  // Checks if the parent of the given particle has a specific PDG ID.
  return hasParentPdgId(p, p.pdgId());
}

bool hasIdenticalChild(const xAOD::TruthParticle& p) {
  // Checks if the given particle has at least one child with an identical PDG
  // ID.
  return findIdenticalChild(p) != nullptr;
}

bool hasParentAbsPdgId(const xAOD::TruthParticle& p, int absPdgId) {
  // No parent recorded, or a broken truth link to it, is treated as "no
  // parent".
  if (p.nParents() == 0)
    return false;
  const xAOD::TruthParticle* parent = p.parent(0);
  // Checks if the parent of the given particle has a specific absolute PDG ID.
  return parent && parent->absPdgId() == absPdgId;
}

bool hasParticleIdenticalParent(const xAOD::TruthParticle& p) {
  // Checks if particle and any of its parents are identical. A broken
  // parent truth link is treated as "not identical".
  for (size_t i = 0; i < p.nParents(); i++) {
    const xAOD::TruthParticle* parent = p.parent(i);
    if (parent && parent->pdgId() == p.pdgId())
      return true;
  }  // for
  return false;
}

bool isChildOf(const xAOD::TruthParticle& parent,
               const xAOD::TruthParticle& child) {
  // Checks if child is child of parent.
  if (parent.uid() == child.uid()) {
    return true;
  }
  // Loop through all children of the parent
  for (size_t i = 0; i < parent.nChildren(); ++i) {
    const xAOD::TruthParticle* c = parent.child(i);
    // A broken child truth link is treated as "not this child" and skipped.
    // Recursively check if child is a child of the current child
    if (c && isChildOf(*c, child)) {
      return true;
    }
  }
  // If child is not found in the children or their descendants, return
  // false
  return false;
}

////////////////////////////////////////////////////
////////// Before and after FSR functions //////////
////////////////////////////////////////////////////

bool isAfterFSR(const xAOD::TruthParticle& p) {
  return !hasIdenticalChild(p);
}

const xAOD::TruthParticle& findAfterFSR(const xAOD::TruthParticle& p) {
  // Follow the chain of identical children down to the last copy.
  const xAOD::TruthParticle* current = &p;
  while (const xAOD::TruthParticle* child = findIdenticalChild(*current))
    current = child;
  return *current;
}

///////////////////////////
////////// Other //////////
///////////////////////////

bool isBrokenTop(const xAOD::TruthParticle& p) {
  // check if particle is a top without children.
  return (p.absPdgId() == 6 && p.nChildren() == 0);
}

bool isQuarkFromPDF(const xAOD::TruthParticle& particle) {
  // In principle we could use some status codes here, e.g. 31/41/42/53/61 for
  // Pythia 8. But that is not guaranteed to be compatible across generators,
  // so instead we just check if it's a massless quark.
  bool isQuark = 1 <= particle.absPdgId() && particle.absPdgId() <= 5;
  bool isMassless = particle.m() == 0;
  return isQuark && isMassless;
}


}  // namespace PartonHistoryUtils
}  // namespace CP
