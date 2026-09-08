//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "DeviceMemoryResourceTool.h"

// System include(s).
#include <cassert>

namespace AthHIP {

StatusCode DeviceMemoryResourceTool::initialize() {

  // Construct the appropriate memory resource.
  m_mr = std::make_unique<vecmem::hip::device_memory_resource>(m_deviceID);

  // Return gracefully.
  return StatusCode::SUCCESS;
}

std::pmr::memory_resource& DeviceMemoryResourceTool::mr() const {

  assert(m_mr);
  return *m_mr;
}

}  // namespace AthHIP
