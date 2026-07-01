//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "HostMemoryResourceTool.h"

// VecMem include(s).
#include <vecmem/memory/cuda/host_memory_resource.hpp>

// System include(s).
#include <cassert>

namespace AthCUDA {

StatusCode HostMemoryResourceTool::initialize() {

  // Construct the appropriate memory resource.
  m_mr = std::make_unique<vecmem::cuda::host_memory_resource>();

  // Return gracefully.
  return StatusCode::SUCCESS;
}

std::pmr::memory_resource& HostMemoryResourceTool::mr() const {

  assert(m_mr);
  return *m_mr;
}

}  // namespace AthCUDA
