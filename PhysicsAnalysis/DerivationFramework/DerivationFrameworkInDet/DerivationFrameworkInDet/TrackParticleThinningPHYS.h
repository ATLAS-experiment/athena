/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRACKPARTICLETHINNINGPHYS_H
#define DERIVATIONFRAMEWORK_TRACKPARTICLETHINNINGPHYS_H

#include "DerivationFrameworkInDet/TrackParticleThinningBase.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include <vector>

namespace DerivationFramework {

  class TrackParticleThinningPHYS : public TrackParticleThinningBase {
  public:
    using TrackParticleThinningBase::TrackParticleThinningBase;

    virtual StatusCode initialize() override final;

  private:
    virtual std::vector<int> updateMask(const EventContext& ctx, const xAOD::TrackParticleContainer* trackParticles) const override final;
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_trackZ0PVKey{
      this, "Z0SGEntryName", m_inDetSGKey, "DFCommonInDetTrackZ0AtPV",
        "Decoration key for z0 wrt PV"};
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_tightPrimaryKey{
      this, "TightPrimaryKey", m_inDetSGKey, "DFCommonTightPrimary",
        "Decoration key for tight primary selection"};
  };
}

#endif // DERIVATIONFRAMEWORK_TRACKPARTICLETHINNINGPHYS_H
