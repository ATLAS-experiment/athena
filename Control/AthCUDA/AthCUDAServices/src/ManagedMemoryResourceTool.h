//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHCUDASERVICES_MANAGEDMEMORYRESOURCETOOL_H
#define ATHCUDASERVICES_MANAGEDMEMORYRESOURCETOOL_H

// Framework include(s).
#include "AthenaBaseComps/AthAlgTool.h"

// AthDevice include(s).
#include "AthDeviceInterfaces/IMemoryResourceTool.h"

namespace AthCUDA {

/// Tool providing memory resource for managed host/device memory
///
/// Making use of @c vecmem::cuda::managed_memory_resource.
///
class ManagedMemoryResourceTool
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

};  // class ManagedMemoryResourceTool

}  // namespace AthCUDA

#endif  // ATHCUDASERVICES_MANAGEDMEMORYRESOURCETOOL_H
