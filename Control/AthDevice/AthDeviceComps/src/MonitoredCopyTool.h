//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICECOMPS_MONITOREDCOPYTOOL_H
#define ATHDEVICECOMPS_MONITOREDCOPYTOOL_H

// Framework include(s).
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

// AthDevice include(s).
#include "AthDeviceInterfaces/ICopyTool.h"
#include "AthDeviceInterfaces/IMemoryMonitorSvc.h"

namespace AthDevice {

/// Tool providing monitoring functionality for @c vecmem::copy objects
///
/// Making use of @c AthDevice::MonitoredCopy, which would in turn
/// provide monitoring information to @c AthDevice::IMemoryMonitorSvc.
///
class MonitoredCopyTool : public extends<AthAlgTool, AthDevice::ICopyTool> {

 public:
  // Inherit the base class's constructor(s).
  using extends::extends;

  /// @name Function(s) inherited from @c AthAlgTool
  /// @{

  /// Initialize the tool
  virtual StatusCode initialize() override;

  /// @}

  /// @name Function(s) inherited from @c AthDevice::ICopyTool
  /// @{

  /// Get the provided @c vecmem::copy object
  ///
  /// @param ctx The event context for which the copy object is requested
  /// @returns The @c vecmem::copy object to use for the current event context
  ///
  virtual std::shared_ptr<const vecmem::copy> copy(
      const EventContext& ctx) const override;

  /// Name/identifier for the copy type/flavour
  virtual std::string_view flavor() const override;

  /// @}

 private:
  /// @name Tool properties
  /// @{

  /// Tool providing the underlying copy object
  ToolHandle<ICopyTool> m_copyTool{this, "CopyTool", "",
                                   "Copy tool to use for memory operations"};

  /// Service collecting the memory monitoring information
  ServiceHandle<IMemoryMonitorSvc> m_monitorSvc{
      this, "MonitorSvc", "AthDevice::MemoryMonitorSvc",
      "Memory monitor service to collect the monitoring information"};

  /// @}

  /// The name of the parent component
  std::string m_component;

};  // class MonitoredCopyTool

}  // namespace AthDevice

#endif  // ATHDEVICECOMPS_MONITOREDCOPYTOOL_H
