//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "HostCopySvc.h"

// VecMem include(s).
#include <vecmem/utils/copy.hpp>

// System include(s).
#include <cassert>

namespace AthDevice {

StatusCode HostCopySvc::initialize() {

  // Construct the copy object.
  m_copy = std::make_unique<vecmem::copy>();

  // Return gracefully.
  return StatusCode::SUCCESS;
}

vecmem::copy& HostCopySvc::copy() const {

  assert(m_copy);
  return *m_copy;
}

}  // namespace AthDevice
