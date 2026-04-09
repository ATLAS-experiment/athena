/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>
/// @author Steffen Korn <steffen.korn@cern.ch>

#include "PartonHistory/PartonHistoryUtils.h"

#include <xAODEventInfo/EventInfo.h>

#include "VectorHelpers/LorentzHelper.h"

namespace CP {
namespace PartonHistoryUtils {

//////////////////////////////////////////////
////////// Parent - child relations //////////
//////////////////////////////////////////////

bool hasParentPdgId(const xAOD::TruthParticle* p, int pdgId) {
  // Checks if the particle or its parent are nullptr.
  if (!p || !p->parent(0)) {
    return false;
  }
  // Checks if the parent of the given particle has a specific PDG ID.
  return p->parent(0)->pdgId() == pdgId;
}

bool hasParentPdgId(const xAOD::TruthParticle* p) {
  // Checks if the parent of the given particle has a specific PDG ID.
  return hasParentPdgId(p, p->pdgId());
}

bool hasIdenticalChild(const xAOD::TruthParticle* p) {
  // Checks if the given particle has at least one child with an identical PDG
  // ID.
  bool check = false;
  for (size_t i = 0; i < p->nChildren(); i++) {
    if (p->child(i) && p->child(i)->pdgId() == p->pdgId())
      check = true;
  }
  return check;
}

bool hasParentAbsPdgId(const xAOD::TruthParticle* p, int absPdgId) {
  // Checks if the parent of the given particle has a specific absolute PDG ID.
  return p->parent(0) && p->parent(0)->absPdgId() == absPdgId;
}

bool hasParticleIdenticalParent(const xAOD::TruthParticle* p) {
  // Checks if particle and any of ity parents are identical.
  bool skipit(false);
  for (size_t i = 0; i < p->nParents(); i++) {
    const xAOD::TruthParticle* parent = p->parent(i);
    if (parent && parent->pdgId() == p->pdgId()) {
      skipit = true;
      break;
    }  // if
  }  // for
  return skipit;
}

bool isChildOf(const xAOD::TruthParticle* parent,
               const xAOD::TruthParticle* child) {
  // Checks if child is child of parent.
  if (!(parent && child))
    return false;
  if (parent->uid() == child->uid()) {
    return true;
  }
  // Loop through all children of the parent
  for (size_t i = 0; i < parent->nChildren(); ++i) {
    // Recursively check if child is a child of the current child
    if (isChildOf(parent->child(i), child)) {
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

bool isAfterFSR(const xAOD::TruthParticle* p) {
  bool isAfter = true;
  for (size_t j = 0; j < p->nChildren(); j++) {
    if (p->child(j) && p->child(j)->pdgId() == p->pdgId())
      isAfter = false;
  }
  return isAfter;
}

const xAOD::TruthParticle* findAfterFSR(const xAOD::TruthParticle* p) {
  bool isAfter(false);
  const int pdgId = p->pdgId();
  int i = 0;
  while (!isAfter) {
    i = 0;
    for (size_t j = 0; j < p->nChildren(); j++) {
      const xAOD::TruthParticle* tmp_children = p->child(j);
      if (tmp_children && tmp_children->pdgId() == pdgId) {
        p = p->child(j);
        i++;
        break;
      }  // if
    }  // for
    if (i == 0)
      isAfter = true;
  }  // while
  return p;
}

///////////////////////////
////////// Other //////////
///////////////////////////

bool isBrokenTop(const xAOD::TruthParticle* p) {
  // check if particle is a top without children.
  return (p->pdgId() == 6 && p->nChildren() == 0);
}

std::string getretrievalstring(const std::string& parent,
                               const std::string& suffix,
                               const std::string& postfix) {
  std::string baseprefix = "MC_";
  return baseprefix + (parent.empty() ? "" : parent + "_") + suffix + postfix;
}

}  // namespace PartonHistoryUtils
}  // namespace CP
