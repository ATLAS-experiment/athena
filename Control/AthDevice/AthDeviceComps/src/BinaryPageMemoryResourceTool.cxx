//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "BinaryPageMemoryResourceTool.h"

// VecMem include(s).
#include <vecmem/memory/binary_page_memory_resource.hpp>

// System include(s).
#include <cassert>

namespace AthDevice {

StatusCode BinaryPageMemoryResourceTool::initialize() {

  // Retrieve the upstream memory resource tool.
  ATH_CHECK(m_mrTool.retrieve());

  // Construct the cached memory resource around this upstream memory resource.
  m_cachedMR =
      std::make_unique<vecmem::binary_page_memory_resource>(m_mrTool->mr());

  // Return gracefully.
  return StatusCode::SUCCESS;
}

std::pmr::memory_resource& BinaryPageMemoryResourceTool::mr() const {

  assert(m_cachedMR);
  return *m_cachedMR;
}

}  // namespace AthDevice
