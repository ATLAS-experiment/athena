// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#ifndef XAODTRUTHCNV_TRUTHPARTICLEPRINTERALG_H
#define XAODTRUTHCNV_TRUTHPARTICLEPRINTERALG_H

// Framework include(s).
#include "AnaAlgorithm/AnaReentrantAlgorithm.h"
#include "AsgDataHandles/ReadHandleKey.h"

// EDM include(s).
#include "xAODTruth/TruthParticleContainer.h"

namespace xAODReader {

/// @brief Algorithm printing @c xAOD::TruthParticle in some simple way
///
/// It is meant only for debugging purposes. To be able to test reading truth
/// particles from a file.
///
class TruthParticlePrinterAlg final : public EL::AnaReentrantAlgorithm {

 public:
  /// Inherit the base class's constructors
  using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;

  /// Function initialising the algorithm
  virtual StatusCode initialize() override;
  /// Function executing the algorithm
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  /// The key of the truth particle container to print
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_key{
      this, "Container", "TruthParticles", "Truth particle container"};

};  // class TruthParticlePrinterAlg

}  // namespace xAODReader

#endif  // XAODTRUTHCNV_TRUTHPARTICLEPRINTERALG_H
