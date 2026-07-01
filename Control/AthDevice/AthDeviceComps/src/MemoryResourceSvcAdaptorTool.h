//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICECOMPS_MEMORYRESOURCESVCADAPTORTOOL_H
#define ATHDEVICECOMPS_MEMORYRESOURCESVCADAPTORTOOL_H

// Framework include(s).
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"

// AthDevice include(s).
#include "AthDeviceInterfaces/IMemoryResourceSvc.h"
#include "AthDeviceInterfaces/IMemoryResourceTool.h"

namespace AthDevice {

/// Tool exposing a memory resource service as a memory resource tool
///
/// Client components should ideally only use memory resources through the
/// @c AthDevice::IMemoryResourceTool interface. But especially caching memory
/// resources are best implemented as a service.
///
/// This tool allows us to expose such services with a tool interface.
///
class MemoryResourceSvcAdaptorTool
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

  /// Get the provided @c std::pmr::memory_resource object
  virtual std::pmr::memory_resource& mr() const override;

  /// @}

 private:
  /// Handle to the service providing the underlying memory resource
  ServiceHandle<IMemoryResourceSvc> m_mrSvc{
      this, "MRSvc", "", "Service providing the 'adapted' memory resource"};

};  // class MemoryResourceSvcAdaptorTool

}  // namespace AthDevice

#endif  // ATHDEVICECOMPS_MEMORYRESOURCESVCADAPTORTOOL_H