//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICECOMPS_MEMORYRESOURCESADAPTORTOOL_H
#define ATHDEVICECOMPS_MEMORYRESOURCESADAPTORTOOL_H

// Framework include(s).
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

// AthDevice include(s).
#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthDeviceInterfaces/IMemoryResourcesTool.h"

namespace AthDevice {

/// Tool implementing @c AthDevice::IMemoryResourcesTool using two tools
///
/// This is the simplest way of implementing the
/// @c AthDevice::IMemoryResourcesTool interface. By making use of two
/// individual @c AthDevice::IMemoryResourceTool tools.
///
class MemoryResourcesAdaptorTool
    : public extends<AthAlgTool, IMemoryResourcesTool> {

 public:
  // Inherit the base class's constructor(s).
  using extends::extends;

  /// @name Function(s) inherited from @c AthAlgTool
  /// @{

  /// Initialize the tool
  virtual StatusCode initialize() override;

  /// @}

  /// @name Function(s) inherited from @c IMemoryResourcesTool
  /// @{

  /// Get the "main" / "device" @c std::pmr::memory_resource object
  virtual std::pmr::memory_resource& mainMR() const override;

  /// Get the "host" (accessible) @c std::pmr::memory_resource object
  virtual std::pmr::memory_resource* hostMR() const override;

  /// @}

 private:
  /// Handle to the "main" / "device" memory resource tool
  ToolHandle<IMemoryResourceTool> m_mainMRTool{
      this, "MainMRTool", "",
      "Tool providing the 'main' / 'device' memory resource"};
  /// Handle to the "host" (accessible) memory resource tool
  ToolHandle<IMemoryResourceTool> m_hostMRTool{
      this, "HostMRTool", "",
      "Tool providing the 'host' (accessible) memory resource"};

};  // class MemoryResourcesAdaptorTool

}  // namespace AthDevice

#endif  // ATHDEVICECOMPS_MEMORYRESOURCESADAPTORTOOL_H
