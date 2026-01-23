/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKERFORWARDPROTON_H
#define DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKERFORWARDPROTON_H

// Base class
#include "DerivationFrameworkMCTruth/TruthCollectionMakerBase.h"
#include "GaudiKernel/SystemOfUnits.h"

namespace DerivationFramework {

  class TruthCollectionMakerForwardProton : public TruthCollectionMakerBase {
  public:

    using TruthCollectionMakerBase::TruthCollectionMakerBase;

  private:
    virtual std::vector<int> updateMask(const xAOD::TruthParticleContainer*) const override final;
    Gaudi::Property<double> m_beamEnergy{this, "BeamEnergy", 6.8 * Gaudi::Units::TeV, "Beam Energy"};
  };
}

#endif // DERIVATIONFRAMEWORK_TRUTHCOLLECTIONMAKERFORWARDPROTON_H
