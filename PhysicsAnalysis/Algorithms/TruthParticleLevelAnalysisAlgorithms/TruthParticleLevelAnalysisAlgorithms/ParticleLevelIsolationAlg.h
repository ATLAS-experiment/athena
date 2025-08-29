/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#ifndef TRUTH__PARTICLELEVEL_ISOLATION__ALG_H
#define TRUTH__PARTICLELEVEL_ISOLATION__ALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgTools/PropertyWrapper.h>

// Framework includes
#include <MCTruthClassifier/IMCTruthClassifier.h>
#include <xAODTruth/TruthParticleContainer.h>

namespace CP {
class ParticleLevelIsolationAlg : public EL::AnaReentrantAlgorithm {
 public:
  using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;
  virtual StatusCode initialize() final;
  virtual StatusCode execute(const EventContext &ctx) const final;

 private:
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_particlesKey{
      this, "particles", "", "the name of the input truth particles container"};
  Gaudi::Property<std::string> m_isolated{
      this, "isolation", "", "decoration for isolated truth particles"};
  Gaudi::Property<std::string> m_notTauOrigin{
      this, "notTauOrigin", "",
      "decoration for truth particles not coming from a tau-decay"};
  Gaudi::Property<std::string> m_checkTypeName{
      this, "checkType", "",
      "the MCTruthPartClassifier::ParticleType string to check against"};
  Gaudi::Property<std::string> m_isolationVariable{
      this, "isoVar", "",
      "variable to use in isolation cuts of the form 'var/pT < cut'"};
  Gaudi::Property<float> m_isolationCut{
      this, "isoCut", -1,
      "threshold to use in isolation cuts of the form 'var/pT < cut'"};
  MCTruthPartClassifier::ParticleType m_checkType;
  std::unique_ptr<const SG::Decorator<char>> m_dec_isolated{};
  std::unique_ptr<const SG::Decorator<char>> m_dec_notTauOrigin{};
  std::unique_ptr<const SG::ConstAccessor<float>> m_acc_isoVar{};
};

}  // namespace CP

#endif
