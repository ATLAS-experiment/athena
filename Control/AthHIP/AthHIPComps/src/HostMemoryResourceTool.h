//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHHIPCOMPS_HOSTMEMORYRESOURCETOOL_H
#define ATHHIPCOMPS_HOSTMEMORYRESOURCETOOL_H

// Framework include(s).
#include "AthenaBaseComps/AthAlgTool.h"

// AthDevice include(s).
#include "AthDeviceInterfaces/IMemoryResourceTool.h"

namespace AthHIP {

/// Tool providing memory resource for pinned host memory
///
/// Making use of @c vecmem::hip::host_memory_resource.
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

  /// Get the provided @c std::pmr::memory_resource object
  virtual std::pmr::memory_resource& mr() const override;

  /// @}

 private:
  /// The memory resource that this tool uses
  std::unique_ptr<std::pmr::memory_resource> m_mr;

};  // class HostMemoryResourceTool

}  // namespace AthHIP

#endif  // ATHHIPCOMPS_HOSTMEMORYRESOURCETOOL_H
