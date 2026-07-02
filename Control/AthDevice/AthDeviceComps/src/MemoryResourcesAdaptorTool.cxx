//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "MemoryResourcesAdaptorTool.h"

namespace AthDevice {

StatusCode MemoryResourcesAdaptorTool::initialize() {

  // Retrieve the sub-tool(s).
  ATH_CHECK(m_mainMRTool.retrieve());
  ATH_CHECK(
      m_hostMRTool.retrieve(DisableTool{m_hostMRTool.typeAndName().empty()}));

  // Return gracefully.
  return StatusCode::SUCCESS;
}

std::pmr::memory_resource& MemoryResourcesAdaptorTool::mainMR() const {

  return m_mainMRTool->mr();
}

std::pmr::memory_resource* MemoryResourcesAdaptorTool::hostMR() const {

  return m_hostMRTool.isEnabled() ? &(m_hostMRTool->mr()) : nullptr;
}

}  // namespace AthDevice
