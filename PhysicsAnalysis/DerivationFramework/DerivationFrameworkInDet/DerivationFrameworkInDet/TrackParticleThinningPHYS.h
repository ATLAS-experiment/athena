/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRACKPARTICLETHINNINGPHYS_H
#define DERIVATIONFRAMEWORK_TRACKPARTICLETHINNINGPHYS_H

#include "DerivationFrameworkInDet/TrackParticleThinningBase.h"
#include "xAODTracking/TrackParticleContainer.h"
#include <vector>

namespace DerivationFramework {

  class TrackParticleThinningPHYS : public TrackParticleThinningBase {
  public:
    using TrackParticleThinningBase::TrackParticleThinningBase;

    virtual StatusCode initialize() override final;

  private:
    virtual std::vector<int> updateMask(const xAOD::TrackParticleContainer* trackParticles) const override final;
    SG::ReadHandleKey< std::vector<float> > m_trackZ0PVKey{ this, "Z0SGEntryName", "", "Collection of floats corresponding to z0 wrt PV for tracks" };
  };
}

#endif // DERIVATIONFRAMEWORK_TRACKPARTICLETHINNINGPHYS_H
