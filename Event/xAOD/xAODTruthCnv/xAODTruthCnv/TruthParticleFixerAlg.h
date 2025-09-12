// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#ifndef XAODTRUTHCNV_TRUTHPARTICLEFIXERALG_H
#define XAODTRUTHCNV_TRUTHPARTICLEFIXERALG_H

// Framework include(s).
#include "AnaAlgorithm/AnaReentrantAlgorithm.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/WriteHandleKey.h"

// EDM include(s).
#include "xAODTruth/TruthParticleContainer.h"

namespace xAODMaker {

/// @brief Algorithm fixing @c xAOD::TruthParticle objects in (old) DAOD files
///
/// This algorithm can be used to translate legacy @c xAOD::TruthParticle
/// information read in from a DAOD file, which didn't use
/// @c xAOD::TruthParticleAuxContainer_v1 for storing the information.
///
class TruthParticleFixerAlg final : public EL::AnaReentrantAlgorithm {

 public:
  /// Inherit the base class's constructors
  using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;

  /// Function initialising the algorithm
  virtual StatusCode initialize() override;
  /// Function executing the algorithm
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  /// The keys of the input xAOD truth containers
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_inputContainerKey{
      this, "InputContainer", "InputFileTruthParticles",
      "Input TruthParticles container"};

  /// The keys for the output xAOD truth containers
  SG::WriteHandleKey<xAOD::TruthParticleContainer> m_outputContainerKey{
      this, "OutputContainer", "TruthParticles",
      "Output TruthParticles container"};

};  // class TruthParticleFixerAlg

}  // namespace xAODMaker

#endif  // XAODTRUTHCNV_TRUTHPARTICLEFIXERALG_H
