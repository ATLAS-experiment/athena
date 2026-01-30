/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKERCHARM_H
#define DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKERCHARM_H

// Base class
#include "DerivationFrameworkMCTruth/TruthCollectionMakerBase.h"

namespace DerivationFramework {

  class TruthCollectionMakerCharm : public TruthCollectionMakerBase {
  public:

    using TruthCollectionMakerBase::TruthCollectionMakerBase;

  private:
    virtual std::vector<int> updateMask(const xAOD::TruthParticleContainer*) const override final;
  };
}

#endif // DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKERCHARM_H
