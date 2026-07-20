// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHEXDEVICE_EVENTDATACOPYEXAMPLEALG_H
#define ATHEXDEVICE_EVENTDATACOPYEXAMPLEALG_H

// Framework include(s).
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

// AthDevice include(s).
#include "AthDeviceInterfaces/ICopiesTool.h"
#include "AthDeviceInterfaces/IMemoryResourcesTool.h"

// Data model include(s).
#include "xAODTracking/TrackParticleContainer.h"

// Local include(s).
#include "DeviceObjectCollection.h"

namespace AthExDevice {

/// Example for how "device agnostic" event data copies should be done
class EventDataCopyExampleAlg : public AthReentrantAlgorithm {

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
  SG::WriteHandleKey<DeviceObjectCollection::buffer> m_outputKey{
      this, "OutputContainer", "InDetTrackParticlesCopy",
      "The output track particle container"};

  /// The copies tool to use for copying data to/from the device
  ToolHandle<AthDevice::ICopiesTool> m_copies{
      this, "CopiesTool", "",
      "The copies tool to use for copying data to/from the device"};
  /// The memory resources tool to use for allocating memory on the device
  ToolHandle<AthDevice::IMemoryResourcesTool> m_mrs{
      this, "MemoryResourcesTool", "",
      "The memory resources tool to use for allocating memory on the device"};

  /// @}

};  // class EventDataCopyExampleAlg

}  // namespace AthExDevice

#endif  // ATHEXDEVICE_EVENTDATACOPYEXAMPLEALG_H
