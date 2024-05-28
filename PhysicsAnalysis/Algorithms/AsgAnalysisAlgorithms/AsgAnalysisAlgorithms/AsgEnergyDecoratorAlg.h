/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#ifndef ASG_ENERGYDECORATOR_ALG__H
#define ASG_ENERGYDECORATOR_ALG__H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <xAODBase/IParticleContainer.h>

namespace CP {

  class AsgEnergyDecoratorAlg final : public EL::AnaAlgorithm {

  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;

  private:
    CP::SysListHandle m_systematicsList {this};
    CP::SysReadHandle<xAOD::IParticleContainer> m_particlesHandle {
      this, "particles", "SetMe", "the particle collection to run on"};
    CP::SysWriteDecorHandle<float> m_energyDecor {
      this, "energyDecoration", "e_%SYS%", "decoration for per-object energy"};

  };

} // namespace

#endif
