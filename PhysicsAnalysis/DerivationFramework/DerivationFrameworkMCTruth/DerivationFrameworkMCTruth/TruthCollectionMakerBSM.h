/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKERBSM_H
#define DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKERBSM_H

// Base class
#include "DerivationFrameworkMCTruth/TruthCollectionMakerBase.h"

namespace DerivationFramework {

  class TruthCollectionMakerBSM : public TruthCollectionMakerBase {
  public:

    using TruthCollectionMakerBase::TruthCollectionMakerBase;

  private:
    virtual std::vector<int> updateMask(const xAOD::TruthParticleContainer*) const override final;
  };
}

#endif // DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKERBSM_H
