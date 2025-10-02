// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#ifndef XAODTRUTHCNV_TRUTHVERTEXFIXERALG_H
#define XAODTRUTHCNV_TRUTHVERTEXFIXERALG_H

// Framework include(s).
#include "AnaAlgorithm/AnaReentrantAlgorithm.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/WriteHandleKey.h"
#include "AsgTools/PropertyWrapper.h"

// EDM include(s).
#include "xAODTruth/TruthVertexContainer.h"

// System include(s).
#include <string>
#include <vector>

namespace xAODMaker {

/// @brief Algorithm fixing @c xAOD::TruthVertex objects in (old) DAOD files
///
/// This algorithm can be used to translate legacy @c xAOD::TruthVertex
/// information read in from a DAOD file, which didn't use
/// @c xAOD::TruthVertexAuxContainer_v1 for storing the information.
///
class TruthVertexFixerAlg final : public EL::AnaReentrantAlgorithm {

 public:
  /// Inherit the base class's constructors
  using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;

  /// Function initialising the algorithm
  virtual StatusCode initialize() override;
  /// Function executing the algorithm
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  /// The keys of the input xAOD truth containers
  SG::ReadHandleKey<xAOD::TruthVertexContainer> m_inputContainerKey{
      this, "InputContainer", "InputFileTruthVertices",
      "Input TruthVertices container"};

  /// The keys for the output xAOD truth containers
  SG::WriteHandleKey<xAOD::TruthVertexContainer> m_outputContainerKey{
      this, "OutputContainer", "TruthVertices",
      "Output TruthVertices container"};

  /// Names of the truth particle links to fix
  Gaudi::Property<std::vector<std::string>> m_particleLinks{
      this, "ParticleLinks", {}, "Names of the truth particle links to fix"};

  /// Names of the truth vertex links to fix
  Gaudi::Property<std::vector<std::string>> m_vertexLinks{
      this, "VertexLinks", {}, "Names of the truth vertex links to fix"};

  /// Prefix to remove from the link names
  Gaudi::Property<std::string> m_linkPrefixToRemove{
      this, "LinkPrefixToRemove", "", "Prefix to remove from the link names"};

};  // class TruthVertexFixerAlg

}  // namespace xAODMaker

#endif  // XAODTRUTHCNV_TRUTHVERTEXFIXERALG_H
