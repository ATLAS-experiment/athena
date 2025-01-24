/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak


#ifndef ASG_ANALYSIS_ALGORITHMS__ASG_ORIGINAL_OBJECT_LINK_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__ASG_ORIGINAL_OBJECT_LINK_ALG_H

#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <SystematicsHandles/SysCopyHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <xAODBase/IParticleContainer.h>
#include <AsgTools/PropertyWrapper.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/ReadHandle.h>

namespace CP
{
  /// \brief an algorithm for relinking a shallow copy with it's base container
  /// when this was not done originally
  ///
  /// In some cases we work with shallow copy containers that originally did
  /// not have proper original object linking done when created.
  /// Currently the client are b-tagging calibration shallow copies.

  class AsgOriginalObjectLinkAlg final : public EL::AnaReentrantAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;
    StatusCode initialize () override;
    StatusCode execute (const EventContext &ctx) const override;

    /// \brief base container name
  private:
    SG::ReadHandleKey<xAOD::IParticleContainer> m_baseContainerName {this, "baseContainerName", "", "base particle container name"};

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the particle container we run on
  private:
    SysCopyHandle<xAOD::IParticleContainer> m_particleHandle {
      this, "particles", "", "the particle container to run on"};
  };
}

#endif
