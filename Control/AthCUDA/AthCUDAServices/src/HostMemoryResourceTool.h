//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHCUDASERVICES_HOSTMEMORYRESOURCETOOL_H
#define ATHCUDASERVICES_HOSTMEMORYRESOURCETOOL_H

// Framework include(s).
#include "AthenaBaseComps/AthAlgTool.h"

// AthDevice include(s).
#include "AthDeviceInterfaces/IMemoryResourceTool.h"

namespace AthCUDA {

/// Tool providing memory resource for pinned host memory
///
/// Making use of @c vecmem::cuda::host_memory_resource.
///
class HostMemoryResourceTool
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

};  // class HostMemoryResourceTool

}  // namespace AthCUDA

#endif  // ATHCUDASERVICES_HOSTMEMORYRESOURCETOOL_H
