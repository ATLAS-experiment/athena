/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKERELECTRON_H
#define DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKERELECTRON_H

// Base class
#include "DerivationFrameworkMCTruth/TruthCollectionMakerBase.h"

namespace DerivationFramework {

  class TruthCollectionMakerElectron : public TruthCollectionMakerBase {
  public:

    using TruthCollectionMakerBase::TruthCollectionMakerBase;

  private:
    virtual std::vector<int> updateMask(const xAOD::TruthParticleContainer*) const override final;
  };
}

#endif // DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKERELECTRON_H
