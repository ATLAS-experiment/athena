/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>
/// @author Steffen Korn <steffen.korn@cern.ch>

#include "VectorHelpers/LorentzHelper.h"

#include <AsgMessaging/MessageCheck.h>

#include "Math/Vector4D.h"
#include "xAODTruth/TruthParticle.h"

namespace CP {

ROOT::Math::PxPyPzMVector GetPxPyPzMfromTruth(
    const xAOD::TruthParticle* TruthParticle) {
  ROOT::Math::PxPyPzMVector MassSafeVector(
      TruthParticle->px(),
      TruthParticle->py(),
      TruthParticle->pz(),
      TruthParticle->m());
  return MassSafeVector;
}

ROOT::Math::PtEtaPhiMVector GetPtEtaPhiMfromTruth(
    const xAOD::TruthParticle* TruthParticle) {
  ROOT::Math::PtEtaPhiMVector MassSafeVector(
      TruthParticle->pt(),
      TruthParticle->eta(),
      TruthParticle->phi(),
      TruthParticle->m());
  return MassSafeVector;
}

ROOT::Math::PtEtaPhiEVector GetPtEtaPhiEfromTruth(
    const xAOD::TruthParticle* TruthParticle) {
  asg::msgUserCode::ANA_MSG_WARNING(
      "LorentzHelper WARNING: We can't guarantee the mass from this 4-vector "
      "is correct, consider using GetPtEtaPhiMfromTruth");
  ROOT::Math::PtEtaPhiEVector EnergySafeVector(
      TruthParticle->pt(),
      TruthParticle->eta(),
      TruthParticle->phi(),
      TruthParticle->e());
  return EnergySafeVector;
}

ROOT::Math::PxPyPzEVector GetPxPyPzEfromTruth(
    const xAOD::TruthParticle* TruthParticle) {
  asg::msgUserCode::ANA_MSG_WARNING(
      "LorentzHelper WARNING: We can't guarantee the mass from this 4-vector "
      "is correct, consider using GetPxPyPzMfromTruth");
  ROOT::Math::PxPyPzEVector EnergySafeVector(
      TruthParticle->px(),
      TruthParticle->py(),
      TruthParticle->pz(),
      TruthParticle->e());
  return EnergySafeVector;
}

}  // namespace CP
