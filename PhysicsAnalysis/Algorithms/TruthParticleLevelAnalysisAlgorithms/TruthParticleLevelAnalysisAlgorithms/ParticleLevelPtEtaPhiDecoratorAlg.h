/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#ifndef TRUTH__PARTICLELEVEL_PTETAPHIDECORATOR__ALG_H
#define TRUTH__PARTICLELEVEL_PTETAPHIDECORATOR__ALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgTools/PropertyWrapper.h>

// Framework includes
#include <xAODTruth/TruthParticleContainer.h>

namespace CP {
class ParticleLevelPtEtaPhiDecoratorAlg : public EL::AnaAlgorithm {
 public:
  using EL::AnaAlgorithm::AnaAlgorithm;
  virtual StatusCode initialize() final;
  virtual StatusCode execute() final;

 private:
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_particlesKey{
      this, "particles", "", "the name of the input truth particles container"};
};

}  // namespace CP

#endif
