//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICECOMPS_MONITOREDMEMORYRESOURCE_H
#define ATHDEVICECOMPS_MONITOREDMEMORYRESOURCE_H

// AthDevice include(s).
#include "AthDeviceInterfaces/IMemoryMonitorSvc.h"

// System include(s).
#include <functional>
#include <memory_resource>

namespace AthDevice {

/// Memory resource that monitors memory allocations and deallocations
///
/// It's an Athena specific wrapper around an upstream memory resource,
/// which reports each of its memory (de)allocations to an
/// @c IMemoryMonitorSvc service.
///
class MonitoredMemoryResource : public std::pmr::memory_resource {

 public:
  /// Constructor
  MonitoredMemoryResource(std::pmr::memory_resource& upstream,
                          IMemoryMonitorSvc& svc, const std::string& component,
                          const std::string& device);
  /// Destructor
  virtual ~MonitoredMemoryResource();

 private:
  /// @name Functions inherited from @c std::pmr::memory_resource
  /// @{

  /// Allocate memory from the resource
  virtual void* do_allocate(std::size_t bytes, std::size_t alignment) override;

  /// Deallocate memory from the resource
  virtual void do_deallocate(void* p, std::size_t bytes,
                             std::size_t alignment) override;

  /// Check if the resource is equal to another resource
  virtual bool do_is_equal(
      const std::pmr::memory_resource& other) const noexcept override;

  /// @}

  /// @name Variables used to monitor the memory allocations/deallocations
  /// @{

  /// The upstream memory resource to which the allocations are forwarded
  std::reference_wrapper<std::pmr::memory_resource> m_upstream;
  /// The service used to monitor the memory allocations/deallocations
  std::reference_wrapper<IMemoryMonitorSvc> m_svc;
  /// The name of the component that is using this memory resource
  std::string m_component;
  /// The name of the device for which the memory resource is provided
  std::string m_device;

  /// @}

};  // class MonitoredMemoryResource

}  // namespace AthDevice

#endif  // ATHDEVICECOMPS_MONITOREDMEMORYRESOURCE_H
