//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "DebugMemoryResourceTool.h"

// VecMem include(s).
#include <vecmem/memory/debug_memory_resource.hpp>

namespace AthDevice {

StatusCode DebugMemoryResourceTool::initialize() {

  // Retrieve the upstream tool.
  ATH_CHECK(m_mrTool.retrieve());

  // Construct the debug resource around it.
  m_mr = std::make_unique<vecmem::debug_memory_resource>(m_mrTool->mr());

  // Return gracefully.
  return StatusCode::SUCCESS;
}

std::pmr::memory_resource& DebugMemoryResourceTool::mr() const {

  assert(m_mr);
  return *m_mr;
}

}  // namespace AthDevice
