// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#ifndef XAODTRUTHCNV_TRUTHVERTEXPRINTERALG_H
#define XAODTRUTHCNV_TRUTHVERTEXPRINTERALG_H

// Framework include(s).
#include "AnaAlgorithm/AnaReentrantAlgorithm.h"
#include "AsgDataHandles/ReadHandleKey.h"

// EDM include(s).
#include "xAODTruth/TruthVertexContainer.h"

namespace xAODReader {

/// @brief Algorithm printing @c xAOD::TruthVertex in some simple way
///
/// It is meant only for debugging purposes. To be able to test reading truth
/// vertices from a file.
///
class TruthVertexPrinterAlg final : public EL::AnaReentrantAlgorithm {

 public:
  /// Inherit the base class's constructors
  using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;

  /// Function initialising the algorithm
  virtual StatusCode initialize() override;
  /// Function executing the algorithm
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  /// The key of the truth vertex container to print
  SG::ReadHandleKey<xAOD::TruthVertexContainer> m_key{
      this, "Container", "TruthVertices", "Truth vertex container"};

};  // class TruthVertexPrinterAlg

}  // namespace xAODReader

#endif  // XAODTRUTHCNV_TRUTHVERTEXPRINTERALG_H
