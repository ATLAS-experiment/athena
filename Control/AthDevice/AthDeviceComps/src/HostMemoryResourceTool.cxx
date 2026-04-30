//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "HostMemoryResourceTool.h"

// System include(s).
#include <cassert>
#include <memory_resource>

namespace AthDevice {

std::pmr::memory_resource& HostMemoryResourceTool::mr() const {

  assert(std::pmr::new_delete_resource() != nullptr);
  return *(std::pmr::new_delete_resource());
}

}  // namespace AthDevice
