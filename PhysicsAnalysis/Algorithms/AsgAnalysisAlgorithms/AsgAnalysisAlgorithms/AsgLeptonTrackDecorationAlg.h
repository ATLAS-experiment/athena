/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef ASG_ANALYSIS_ALGORITHMS__ASG_LEPTON_TRACK_DECORATION_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__ASG_LEPTON_TRACK_DECORATION_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <xAODBase/IParticleContainer.h>

#include "AsgDataHandles/ReadHandleKey.h"
#include <xAODTracking/VertexContainer.h>
#include <xAODEventInfo/EventInfo.h>

namespace CP
{
  /// \brief an algorithm for decorating track variables on leptons

  class AsgLeptonTrackDecorationAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;

  private:
    /// \brief the systematics list we run
    SysListHandle m_systematicsList {this};

    /// \brief the EventInfo key
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {
      this, "eventInfo", "EventInfo", "the name of the EventInfo object to retrieve"};

    /// \brief the PrimaryVertex key
    SG::ReadHandleKey<xAOD::VertexContainer> m_primaryVerticesKey {
      this, "primaryVertices", "PrimaryVertices",
	"the name of the PrimaryVertex container to retrieve"};

    /// \brief the particle container we run on
    SysReadHandle<xAOD::IParticleContainer> m_particlesHandle {
      this, "particles", "", "the asg collection to run on"};

    SysWriteDecorHandle<float> m_d0sigHandle {
      this, "d0sig", "d0sig_%SYS%", "decoration name for d0 significance" };

    SysWriteDecorHandle<float> m_z0sinthetaHandle {
      this, "z0sintheta", "z0sintheta_%SYS%", "decoration name for z0*sin(theta)" };

  };

}

#endif
