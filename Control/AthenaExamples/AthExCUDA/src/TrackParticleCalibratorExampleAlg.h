// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHEXCUDA_TRACKPARTICLECALIBRATOREXAMPLEALG_H
#define ATHEXCUDA_TRACKPARTICLECALIBRATOREXAMPLEALG_H

// Framework include(s).
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

// Device include(s).
#include "AthCUDAInterfaces/IStreamTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "AthDeviceInterfaces/IMemoryResourcesTool.h"

// xAOD include(s).
#include "xAODTracking/TrackParticleContainer.h"

// Traccc include(s).
#include <traccc/edm/track_collection.hpp>

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
  /// @name Algorithm properties
  /// @{

  /// The input container
  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_inputKey{
      this, "InputContainer", "InDetTrackParticles",
      "The input track particle container"};
  /// The output container
  SG::WriteHandleKey<xAOD::TrackParticleContainer> m_outputKey{
      this, "OutputContainer", "CalibratedInDetTrackParticles",
      "The output track particle container"};

  /// Host memory resource tool to use
  ToolHandle<AthDevice::IMemoryResourcesTool> m_mrTool{
      this, "MemoryResourcesTool", "",
      "Tool providing the memory resource(s) to use"};

  /// Host copy tool to use
  ToolHandle<AthDevice::ICopyTool> m_hostCopyTool{this, "HostCopyTool", "",
                                                  "The host copy tool to use"};
  /// Device copy tool to use
  ToolHandle<AthDevice::ICopyTool> m_deviceCopyTool{
      this, "DeviceCopyTool", "", "The device copy tool to use"};

  /// Stream tool to use
  ToolHandle<AthCUDA::IStreamTool> m_streamTool{this, "StreamTool", "",
                                                "The stream tool to use"};

  /// @}

};  // class LinearTransformTaskExampleAlg

/// Perform the transformation on an NVIDIA GPU
StatusCode calibrateOnGPU(
    cudaStream_t stream,
    const traccc::edm::track_collection<traccc::default_algebra>::const_view&
        input,
    traccc::edm::track_collection<traccc::default_algebra>::view& output);

}  // namespace AthCUDAExamples

#endif  // ATHEXCUDA_TRACKPARTICLECALIBRATOREXAMPLEALG_H
