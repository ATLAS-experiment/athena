/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#ifndef TRUTH_PARTICLELEVEL_CHARGEDECORATOR_ALG_H
#define TRUTH_PARTICLELEVEL_CHARGEDECORATOR_ALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgTools/PropertyWrapper.h>

// Framework includes
#include <xAODTruth/TruthParticleContainer.h>

namespace CP {
class ParticleLevelChargeDecoratorAlg : public EL::AnaReentrantAlgorithm {
 public:
  using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;
  virtual StatusCode initialize() final;
  virtual StatusCode execute(const EventContext &ctx) const final;

 private:
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_particlesKey{
      this, "particles", "", "the name of the input truth particles container"};
  SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decChargeKey{
      this, "chargeDecoration", m_particlesKey, "charge", "the charge decoration"};
};

}  // namespace CP

#endif
