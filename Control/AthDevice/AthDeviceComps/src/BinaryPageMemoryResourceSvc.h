//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICECOMPS_BINARYPAGEMEMORYRESOURCESVC_H
#define ATHDEVICECOMPS_BINARYPAGEMEMORYRESOURCESVC_H

// Framework include(s).
#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/ToolHandle.h"

// AthDevice include(s).
#include "AthDeviceInterfaces/IMemoryResourceSvc.h"
#include "AthDeviceInterfaces/IMemoryResourceTool.h"

namespace AthDevice {

/// Service implementing "binary page" caching on top of another memory resource
///
/// Note that the memory resource provided by this service **is** thread safe.
///
class BinaryPageMemoryResourceSvc
    : public extends<AthService, IMemoryResourceSvc> {

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
      this, "MRTool", "", "Tool providing the memory resource to be cached"};
  /// The memory resource that this tool uses for caching
  std::unique_ptr<std::pmr::memory_resource> m_cachedMR;
  /// The memory resource that this tool uses for synchronization
  std::unique_ptr<std::pmr::memory_resource> m_syncedMR;

};  // class BinaryPageMemoryResourceSvc

}  // namespace AthDevice

#endif  // ATHDEVICECOMPS_BINARYPAGEMEMORYRESOURCESVC_H
