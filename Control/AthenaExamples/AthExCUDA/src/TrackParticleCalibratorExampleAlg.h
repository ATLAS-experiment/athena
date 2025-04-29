// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHEXCUDA_TRACKPARTICLECALIBRATOREXAMPLEALG_H
#define ATHEXCUDA_TRACKPARTICLECALIBRATOREXAMPLEALG_H

// Framework include(s).
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

// xAOD include(s).
#include "xAODTracking/TrackParticleContainer.h"

namespace AthCUDAExamples {

/// Example algorithm performing "track particle calibration"
///
/// It uses the VecMem based @c AthExCUDA::TrackParticleContainer to offload
/// information about @c xAOD::TrackParticle-s to the GPU, and to get the
/// results of the calibration back. Converting the results back into an
/// @c xAOD::TrackParticleContainer in the end.
///
/// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
///
class TrackParticleCalibratorExampleAlg : public AthReentrantAlgorithm {

 public:
  // Inherit the base class's constructor(s).
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  /// @name Function(s) inherited from @c AthReentrantAlgorithm
  /// @{

  /// Function initialising the algorithm
  virtual StatusCode initialize() override;

  /// Function executing the algorithm
  virtual StatusCode execute(const EventContext& ctx) const override;

  /// @}

 private:
  /// The input container
  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_inputKey{
      this, "InputContainer", "InDetTrackParticles",
      "The input track particle container"};
  /// The output container
  SG::WriteHandleKey<xAOD::TrackParticleContainer> m_outputKey{
      this, "OutputContainer", "CalibratedInDetTrackParticles",
      "The output track particle container"};

};  // class LinearTransformTaskExampleAlg

}  // namespace AthCUDAExamples

#endif  // ATHEXCUDA_TRACKPARTICLECALIBRATOREXAMPLEALG_H
