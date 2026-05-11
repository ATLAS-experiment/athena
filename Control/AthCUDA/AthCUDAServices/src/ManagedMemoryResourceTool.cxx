//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "ManagedMemoryResourceTool.h"

// VecMem include(s).
#include <vecmem/memory/cuda/managed_memory_resource.hpp>

// System include(s).
#include <cassert>

namespace AthCUDA {

StatusCode ManagedMemoryResourceTool::initialize() {

  // Construct the appropriate memory resource.
  m_mr = std::make_unique<vecmem::cuda::managed_memory_resource>();

  // Return gracefully.
  return StatusCode::SUCCESS;
}

std::pmr::memory_resource& ManagedMemoryResourceTool::mr() const {

  assert(m_mr);
  return *m_mr;
}

}  // namespace AthCUDA
