//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "BinaryPageMemoryResourceSvc.h"

// VecMem include(s).
#include <vecmem/memory/binary_page_memory_resource.hpp>
#include <vecmem/memory/synchronized_memory_resource.hpp>

// System include(s).
#include <cassert>

namespace AthDevice {

StatusCode BinaryPageMemoryResourceSvc::initialize() {

  // Retrieve the upstream memory resource tool.
  ATH_CHECK(m_mrTool.retrieve());

  // Construct the cached and synchronized memory resources around this upstream
  // memory resource.
  m_cachedMR =
      std::make_unique<vecmem::binary_page_memory_resource>(m_mrTool->mr());
  m_syncedMR =
      std::make_unique<vecmem::synchronized_memory_resource>(*m_cachedMR);

  // Return gracefully.
  return StatusCode::SUCCESS;
}

std::pmr::memory_resource& BinaryPageMemoryResourceSvc::mr() const {

  assert(m_cachedMR);
  assert(m_syncedMR);
  return *m_syncedMR;
}

}  // namespace AthDevice
