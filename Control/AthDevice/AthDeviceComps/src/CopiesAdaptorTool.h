//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICECOMPS_COPIESADAPTORTOOL_H
#define ATHDEVICECOMPS_COPIESADAPTORTOOL_H

// Framework include(s).
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

// AthDevice include(s).
#include "AthDeviceInterfaces/ICopiesTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"

namespace AthDevice {

/// Tool implementing @c AthDevice::ICopiesTool using two tools
///
/// This is the simplest way of implementing the @c AthDevice::ICopiesTool
/// interface. By making use of two individual @c AthDevice::ICopyTool tools.
///
class CopiesAdaptorTool : public extends<AthAlgTool, ICopiesTool> {

 public:
  // Inherit the base class's constructor(s).
  using extends::extends;

  /// @name Function(s) inherited from @c AthAlgTool
  /// @{

  /// Initialize the tool
  virtual StatusCode initialize() override;

  /// @}

  /// @name Function(s) inherited from @c ICopiesTool
  /// @{

  /// Get the "host" copy object
  virtual std::shared_ptr<const vecmem::copy> hostCopy(
      const EventContext& ctx) const override;

  /// Get the "device" copy object
  virtual std::shared_ptr<const vecmem::copy> deviceCopy(
      const EventContext& ctx) const override;

  /// @}

 private:
  /// Handle to the "host" copy tool
  ToolHandle<ICopyTool> m_hostCopyTool{this, "HostCopyTool", "",
                                       "Tool providing the 'host' copy object"};
  /// Handle to the "device" copy tool
  ToolHandle<ICopyTool> m_deviceCopyTool{
      this, "DeviceCopyTool", "", "Tool providing the 'device' copy object"};

};  // class CopiesAdaptorTool

}  // namespace AthDevice

#endif  // ATHDEVICECOMPS_COPIESADAPTORTOOL_H
