//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "PoolMemoryResourceSvc.h"

// VecMem include(s).
#include <vecmem/memory/pool_memory_resource.hpp>
#include <vecmem/memory/synchronized_memory_resource.hpp>

// System include(s).
#include <cassert>

namespace AthDevice {

StatusCode PoolMemoryResourceSvc::initialize() {

  // Retrieve the upstream memory resource tool.
  ATH_CHECK(m_mrTool.retrieve());

  // Construct the cached and synchronized memory resources around this upstream
  // memory resource.
  m_cachedMR =
      std::make_unique<vecmem::pool_memory_resource>(m_mrTool->mr(), m_opts);
  m_syncedMR =
      std::make_unique<vecmem::synchronized_memory_resource>(*m_cachedMR);

  // Return gracefully.
  return StatusCode::SUCCESS;
}

std::pmr::memory_resource& PoolMemoryResourceSvc::mr() const {

  assert(m_cachedMR);
  assert(m_syncedMR);
  return *m_syncedMR;
}

}  // namespace AthDevice
