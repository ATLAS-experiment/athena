//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "MonitoredMemoryResource.h"

namespace AthDevice {

MonitoredMemoryResource::MonitoredMemoryResource(
    std::pmr::memory_resource& upstream, IMemoryMonitorSvc& svc,
    const std::string& component, const std::string& device)
    : m_upstream(upstream),
      m_svc(svc),
      m_component(component),
      m_device(device) {}

MonitoredMemoryResource::~MonitoredMemoryResource() = default;

void* MonitoredMemoryResource::do_allocate(std::size_t bytes,
                                           std::size_t alignment) {

  // Forward the allocation to the upstream memory resource.
  void* result = m_upstream.get().allocate(bytes, alignment);

  // Notify the service that a memory allocation has been made.
  m_svc.get().memoryAllocated(m_component, m_device, bytes);

  // Return the pointer to the allocated memory.
  return result;
}

void MonitoredMemoryResource::do_deallocate(void* p, std::size_t bytes,
                                            std::size_t alignment) {

  // Forward the deallocation to the upstream memory resource.
  m_upstream.get().deallocate(p, bytes, alignment);

  // Notify the service that a memory deallocation has been made.
  m_svc.get().memoryDeallocated(m_component, m_device, bytes);
}

bool MonitoredMemoryResource::do_is_equal(
    const std::pmr::memory_resource& other) const noexcept {

  // Simple "object identity" check.
  return this == &other;
}

}  // namespace AthDevice
