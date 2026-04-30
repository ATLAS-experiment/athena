//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "SynchronizedMemoryResourceTool.h"

// VecMem include(s).
#include <vecmem/memory/synchronized_memory_resource.hpp>

// System include(s).
#include <cassert>

namespace AthDevice {

StatusCode SynchronizedMemoryResourceTool::initialize() {

  // Retrieve the upstream memory resource tool.
  ATH_CHECK(m_mrTool.retrieve());

  // Construct the synchronized memory resource around this upstream memory
  // resource.
  m_syncedMR =
      std::make_unique<vecmem::synchronized_memory_resource>(m_mrTool->mr());

  // Return gracefully.
  return StatusCode::SUCCESS;
}

std::pmr::memory_resource& SynchronizedMemoryResourceTool::mr() const {

  assert(m_syncedMR);
  return *m_syncedMR;
}

}  // namespace AthDevice
