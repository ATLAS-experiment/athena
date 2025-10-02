/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGBPHYSHYPO_CONSTANTS_H
#define TRIGBPHYSHYPO_CONSTANTS_H

#include "TruthUtils/ParticleConstants.h"

// PDG'2020
struct PDG20 {
  static constexpr double
    mElectron   = ParticleConstants::electronMassInMeV,
    mMuon       = ParticleConstants::muonMassInMeV,
    mPion       = ParticleConstants::chargedPionMassInMeV,
    mPion0      = ParticleConstants::piZeroMassInMeV,
    mKaon       = ParticleConstants::chargedKaonMassInMeV,
    mK_S0       = ParticleConstants::KZeroMassInMeV,
    mPhi1020    = 1019.461,
    mD0         = ParticleConstants::DZeroMassInMeV,
    mProton     = ParticleConstants::protonMassInMeV,
    mLambda0    = ParticleConstants::lambdaMassInMeV,
    mJpsi       = ParticleConstants::JpsiMassInMeV,
    mPsi2S      = 3686.097,
    mB          = ParticleConstants::BPlusMassInMeV,
    mB0         = ParticleConstants::BZeroMassInMeV,
    mB_s0       = ParticleConstants::BsMassInMeV,
    mB_c        = 6274.9,
    mLambda_b0  = 5619.60;
};

#endif
