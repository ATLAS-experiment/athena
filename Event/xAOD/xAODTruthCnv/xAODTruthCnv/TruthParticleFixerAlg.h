// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#ifndef XAODTRUTHCNV_TRUTHPARTICLEFIXERALG_H
#define XAODTRUTHCNV_TRUTHPARTICLEFIXERALG_H

// Framework include(s).
#include "AnaAlgorithm/AnaReentrantAlgorithm.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/WriteHandleKey.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"
#include "AsgTools/PropertyWrapper.h"

// EDM include(s).
#include "xAODTruth/TruthParticleContainer.h"

// System include(s).
#include <string>
#include <vector>

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

  SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_uidKey{
    this, "UIDKey", m_inputContainerKey, "uid"};

  /// Names of the truth particle links to fix
  Gaudi::Property<std::vector<std::string>> m_particleLinks{
      this, "ParticleLinks", {}, "Names of the truth particle links to fix"};

  /// Names of the truth vertex links to fix
  Gaudi::Property<std::vector<std::string>> m_vertexLinks{
      this, "VertexLinks", {}, "Names of the truth vertex links to fix"};

  /// Prefix to remove from the link names
  Gaudi::Property<std::string> m_linkPrefixToRemove{
      this, "LinkPrefixToRemove", "", "Prefix to remove from the link names"};

};  // class TruthParticleFixerAlg

}  // namespace xAODMaker

#endif  // XAODTRUTHCNV_TRUTHPARTICLEFIXERALG_H
