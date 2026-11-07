//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "MonitoredCopy.h"

// System include(s).
#include <cassert>

namespace AthDevice {

MonitoredCopy::MonitoredCopy(std::shared_ptr<const vecmem::copy> upstream,
                             IMemoryMonitorSvc& svc,
                             const std::string& component,
                             const std::string& flavor)
    : m_upstream(std::move(upstream)),
      m_svc(svc),
      m_component(component),
      m_flavor(flavor) {}

MonitoredCopy::~MonitoredCopy() = default;

void MonitoredCopy::do_copy(std::size_t size, const void* from, void* to,
                            type::copy_type cptype) const {

  // Forward the copy operation to the upstream copy object.
  assert(m_upstream);
  m_upstream->typeless_copy(size, from, to, cptype);

  // Notify the service that a memory copy has been made.
  m_svc.get().memoryCopied(m_component, m_flavor, cptype, size);
}

void MonitoredCopy::do_memset(std::size_t size, void* ptr, int value) const {

  // Forward the memset operation to the upstream copy object.
  assert(m_upstream);
  m_upstream->typeless_memset(size, ptr, value);

  // Notify the service that a memory fill has been made.
  m_svc.get().memorySet(m_component, m_flavor, size);
}

}  // namespace AthDevice
