//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHCUDASERVICES_DEVICEMEMORYRESOURCETOOL_H
#define ATHCUDASERVICES_DEVICEMEMORYRESOURCETOOL_H

// Framework include(s).
#include "AthenaBaseComps/AthAlgTool.h"

// AthDevice include(s).
#include "AthDeviceInterfaces/IMemoryResourceTool.h"

// VecMem include(s).
#include <vecmem/memory/cuda/device_memory_resource.hpp>

namespace AthCUDA {

/// Tool providing memory resource for device memory
///
/// Making use of @c vecmem::cuda::device_memory_resource.
///
class DeviceMemoryResourceTool
    : public extends<AthAlgTool, AthDevice::IMemoryResourceTool> {

 public:
  // Inherit the base class's constructor(s).
  using extends::extends;

  /// @name Function(s) inherited from @c AthAlgTool
  /// @{

  /// Initialize the tool
  virtual StatusCode initialize() override;

  /// @}

  /// @name Function(s) inherited from @c AthDevice::IMemoryResourceTool
  /// @{

  virtual std::pmr::memory_resource& mr() const override;

  /// @}

 private:
  /// The memory resource that this tool uses
  std::unique_ptr<std::pmr::memory_resource> m_mr;

  /// @name Tool properties
  /// @{

  /// Device to allocate memory on
  Gaudi::Property<int> m_deviceID{
      this, "DeviceID", vecmem::cuda::device_memory_resource::INVALID_DEVICE,
      "ID of the device to allocate memory on (-1 to use the default device)"};

  /// @}

};  // class DeviceMemoryResourceTool

}  // namespace AthCUDA

#endif  // ATHCUDASERVICES_DEVICEMEMORYRESOURCETOOL_H
