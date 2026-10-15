//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICECOMPS_MONITOREDMEMORYRESOURCETOOL_H
#define ATHDEVICECOMPS_MONITOREDMEMORYRESOURCETOOL_H

// Framework include(s).
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

// AthDevice include(s).
#include "AthDeviceInterfaces/IMemoryMonitorSvc.h"
#include "AthDeviceInterfaces/IMemoryResourceTool.h"

namespace AthDevice {

/// Tool providing monitoring functionality for memory resources
///
/// Making use of @c AthDevice::MonitoredMemoryResource, which would in turn
/// provide monitoring information to @c AthDevice::IMemoryMonitorSvc.
///
class MonitoredMemoryResourceTool
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

  /// Get the provided @c std::pmr::memory_resource object
  virtual std::pmr::memory_resource& mr() const override;

  /// The name of the device for which the memory resource is provided
  virtual std::string_view deviceName() const override;

  /// @}

 private:
  /// The memory resource that this tool uses
  std::unique_ptr<std::pmr::memory_resource> m_mr;

  /// @name Tool properties
  /// @{

  /// Underlying memory resource to use for allocations
  ToolHandle<IMemoryResourceTool> m_mrTool{
      this, "MRTool", "", "Memory resource tool to use for allocations"};

  /// Service collecting the memory monitoring information
  ServiceHandle<IMemoryMonitorSvc> m_monitorSvc{
      this, "MonitorSvc", "AthDevice::MemoryMonitorSvc",
      "Memory monitor service to collect the monitoring information"};

  /// @}

};  // class MonitoredMemoryResourceTool

}  // namespace AthDevice

#endif  // ATHDEVICECOMPS_MONITOREDMEMORYRESOURCETOOL_H
