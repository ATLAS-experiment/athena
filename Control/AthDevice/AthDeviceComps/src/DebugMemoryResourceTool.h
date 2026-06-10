//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICECOMPS_DEBUGMEMORYRESOURCETOOL_H
#define ATHDEVICECOMPS_DEBUGMEMORYRESOURCETOOL_H

// Framework include(s).
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

// AthDevice include(s).
#include "AthDeviceInterfaces/IMemoryResourceTool.h"

namespace AthDevice {

/// Tool providing debugging functionality for memory resources
///
/// Making use of @c vecmem::debug_memory_resource.
///
class DebugMemoryResourceTool
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

  /// @}

 private:
  /// The memory resource that this tool uses
  std::unique_ptr<std::pmr::memory_resource> m_mr;

  /// @name Tool properties
  /// @{

  /// Underlying memory resource to use for allocations
  ToolHandle<IMemoryResourceTool> m_mrTool{
      this, "MRTool", "", "Memory resource tool to use for allocations"};

  /// @}

};  // class DebugMemoryResourceTool

}  // namespace AthDevice

#endif  // ATHDEVICECOMPS_DEBUGMEMORYRESOURCETOOL_H
