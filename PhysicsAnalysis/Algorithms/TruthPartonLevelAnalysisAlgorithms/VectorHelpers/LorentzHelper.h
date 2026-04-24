/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>
/// @author Steffen Korn <steffen.korn@cern.ch>

#ifndef LORENTZHELPER_H
#define LORENTZHELPER_H

// Include necessary header files
#include "Math/Vector4D.h"
#include "Math/VectorUtil.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/xAODTruthHelpers.h"

// Using directives for convenience with ROOT::Math vector types
using ROOT::Math::PtEtaPhiEVector;
using ROOT::Math::PtEtaPhiMVector;
using ROOT::Math::PxPyPzEVector;
using ROOT::Math::PxPyPzMVector;

// Template function to create a PtEtaPhiMVector
template <typename object>
PtEtaPhiMVector GetPtEtaPhiM(const object* obj) {
  return PtEtaPhiMVector(
      obj->pt(),   // float px and py cast to double to build pT
      obj->eta(),  // double
      obj->phi(),  // double
      obj->m());   // double
}

// Template function to create a PtEtaPhiEVector
template <typename object>
PtEtaPhiEVector GetPtEtaPhiE(const object* obj) {
  return PtEtaPhiEVector(
      obj->pt(),   // float px and py cast to double to build pT
      obj->eta(),  // double
      obj->phi(),  // double
      obj->e());   // double
}

// Template function to create a PxPyPzMVector
template <typename object>
PxPyPzMVector GetPxPyPzM(const object* obj) {
  return PxPyPzMVector(obj->px(),  // float
                       obj->py(),  // float
                       obj->pz(),  // float
                       obj->m());  // double
}

// Template function to create a PxPyPzEVector
template <typename object>
PxPyPzEVector GetPxPyPzE(const object* obj) {
  return PxPyPzEVector(obj->px(),  // float
                       obj->py(),  // float
                       obj->pz(),  // float
                       obj->e());  // double
}

// We don't template these as some will include warnings and differ slightly!
// This shall highlight the difference w.r.t. truth particle information
PtEtaPhiMVector GetPtEtaPhiMfromTruth(const xAOD::TruthParticle* TruthParticle);
PxPyPzMVector GetPxPyPzMfromTruth(const xAOD::TruthParticle* TruthParticle);
PtEtaPhiEVector GetPtEtaPhiEfromTruth(const xAOD::TruthParticle* TruthParticle);
PxPyPzEVector GetPxPyPzEfromTruth(const xAOD::TruthParticle* TruthParticle);

#endif
