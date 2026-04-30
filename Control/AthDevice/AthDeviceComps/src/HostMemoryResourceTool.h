//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICECOMPS_HOSTMEMORYRESOURCETOOL_H
#define ATHDEVICECOMPS_HOSTMEMORYRESOURCETOOL_H

// Framework include(s).
#include "AthenaBaseComps/AthAlgTool.h"

// AthDevice include(s).
#include "AthDeviceInterfaces/IMemoryResourceTool.h"

namespace AthDevice {

/// Tool providing a basic, host-side memory resource
///
/// The memory resource provided by this tool **is** thread safe.
///
class HostMemoryResourceTool : public extends<AthAlgTool, IMemoryResourceTool> {

 public:
  // Inherit the base class's constructor(s).
  using extends::extends;

  /// @name Function(s) inherited from @c IMemoryResourceTool
  /// @{

  virtual std::pmr::memory_resource& mr() const override;

  /// @}

 private:
  /// The memory resource that this tool uses
  std::unique_ptr<std::pmr::memory_resource> m_mr;

};  // class HostMemoryResourceTool

}  // namespace AthDevice

#endif  // ATHDEVICECOMPS_HOSTMEMORYRESOURCETOOL_H
