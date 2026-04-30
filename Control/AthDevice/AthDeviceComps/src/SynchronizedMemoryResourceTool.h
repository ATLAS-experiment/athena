//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICECOMPS_SYNCHRONIZEDMEMORYRESOURCETOOL_H
#define ATHDEVICECOMPS_SYNCHRONIZEDMEMORYRESOURCETOOL_H

// Framework include(s).
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

// AthDevice include(s).
#include "AthDeviceInterfaces/IMemoryResourceTool.h"

namespace AthDevice {

/// Tool providing thread-safety for an upstream memory resource
///
/// This tool can be used to take a thread-unsafe memory resource, and add the
/// necessary synchronization around it to make it thread safe.
///
class SynchronizedMemoryResourceTool
    : public extends<AthAlgTool, IMemoryResourceTool> {

 public:
  // Inherit the base class's constructor(s).
  using extends::extends;

  /// @name Function(s) inherited from @c AthAlgTool
  /// @{

  /// Initialize the tool
  virtual StatusCode initialize() override;

  /// @}

  /// @name Function(s) inherited from @c IMemoryResourceTool
  /// @{

  virtual std::pmr::memory_resource& mr() const override;

  /// @}

 private:
  /// Handle to the tool providing the underlying memory resource
  ToolHandle<IMemoryResourceTool> m_mrTool{
      this, "MRTool", "",
      "Tool providing the memory resource to be synchronized"};
  /// The memory resource that this tool uses for synchronization
  std::unique_ptr<std::pmr::memory_resource> m_syncedMR;

};  // class SynchronizedMemoryResourceTool

}  // namespace AthDevice

#endif  // ATHDEVICECOMPS_SYNCHRONIZEDMEMORYRESOURCETOOL_H
