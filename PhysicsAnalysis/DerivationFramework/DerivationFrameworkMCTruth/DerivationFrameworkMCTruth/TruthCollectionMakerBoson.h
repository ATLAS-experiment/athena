/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKERBOSON_H
#define DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKERBOSON_H

// Base class
#include "DerivationFrameworkMCTruth/TruthCollectionMakerBase.h"

namespace DerivationFramework {

  class TruthCollectionMakerBoson : public TruthCollectionMakerBase {
  public:

    using TruthCollectionMakerBase::TruthCollectionMakerBase;

  private:
    virtual std::vector<int> updateMask(const xAOD::TruthParticleContainer*) const override final;
  };
}

#endif // DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKERBOSON_H
