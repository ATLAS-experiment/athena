//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "MemoryResourceSvcAdaptorTool.h"

// System include(s).
#include <cassert>

namespace AthDevice {

StatusCode MemoryResourceSvcAdaptorTool::initialize() {

  // Retrieve the "wrapped" service.
  ATH_CHECK(m_mrSvc.retrieve());

  // Return gracefully.
  return StatusCode::SUCCESS;
}

std::pmr::memory_resource& MemoryResourceSvcAdaptorTool::mr() const {

  // Just return the memory resource provided by the service.
  assert(m_mrSvc.isValid());
  return m_mrSvc->mr();
}

}  // namespace AthDevice
