/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Daniel Werner

#ifndef TRACKING_ANALYSIS_ALGORITHMS__EXTRAVARDECORATOR_ALG__H
#define TRACKING_ANALYSIS_ALGORITHMS__EXTRAVARDECORATOR_ALG__H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <xAODTracking/TrackParticleContainer.h>

namespace CP {

  class InDetTrackExtraVarDecoratorAlg final : public EL::AnaAlgorithm {

  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;

  private:
    CP::SysListHandle m_systematicsList {this};
    CP::SysReadHandle<xAOD::TrackParticleContainer> m_tracksHandle {
      this, "inDetTracks", "", "the track collection to run on"};
    CP::SysWriteDecorHandle<float> m_momentumDecor {
      this, "momentumDecoration", "pt_%SYS%", "decoration for per-object transverse momentum"};
    CP::SysWriteDecorHandle<float> m_etaDecor {
      this, "etaDecoration", "eta_%SYS%", "decoration for per-object pseudorapidity"};
    CP::SysWriteDecorHandle<float> m_chargeDecor {
      this, "chargeDecoration", "charge_%SYS%", "decoration for per-object charge"};

  };

} // namespace

#endif
