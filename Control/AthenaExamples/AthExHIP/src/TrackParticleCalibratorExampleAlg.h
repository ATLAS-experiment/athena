// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHEXHIP_TRACKPARTICLECALIBRATOREXAMPLEALG_H
#define ATHEXHIP_TRACKPARTICLECALIBRATOREXAMPLEALG_H

// Framework include(s).
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

// Device include(s).
#include "AthDeviceInterfaces/ICopiesTool.h"
#include "AthDeviceInterfaces/IMemoryResourcesTool.h"
#include "AthHIPInterfaces/IStreamTool.h"

// xAOD include(s).
#include "xAODTracking/TrackParticleContainer.h"

// Traccc include(s).
#include <traccc/edm/track_collection.hpp>

namespace AthHIPExamples {

/// Example algorithm performing "track particle calibration"
///
/// It performs a super naive conversion between @c xAOD::TrackParticleContainer
/// and @c traccc::edm::track_collection. Something not to be taken seriously.
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
  ToolHandle<AthDevice::ICopiesTool> m_copyTool{
      this, "CopiesTool", "",
      "Tool providing the vecmem::copy object(s) to use"};

  /// Stream tool to use
  ToolHandle<AthHIP::IStreamTool> m_streamTool{this, "StreamTool", "",
                                               "The stream tool to use"};

  /// @}

};  // class LinearTransformTaskExampleAlg

}  // namespace AthHIPExamples

#endif  // ATHEXHIP_TRACKPARTICLECALIBRATOREXAMPLEALG_H
