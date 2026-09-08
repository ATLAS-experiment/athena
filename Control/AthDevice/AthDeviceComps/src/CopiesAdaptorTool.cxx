//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "CopiesAdaptorTool.h"

namespace AthDevice {

StatusCode CopiesAdaptorTool::initialize() {

  // Retrieve the sub-tool(s).
  ATH_CHECK(m_hostCopyTool.retrieve());
  ATH_CHECK(m_deviceCopyTool.retrieve());

  // Return gracefully.
  return StatusCode::SUCCESS;
}

std::shared_ptr<const vecmem::copy> CopiesAdaptorTool::hostCopy(
    const EventContext& ctx) const {

  return m_hostCopyTool->copy(ctx);
}

std::shared_ptr<const vecmem::copy> CopiesAdaptorTool::deviceCopy(
    const EventContext& ctx) const {

  return m_deviceCopyTool->copy(ctx);
}

}  // namespace AthDevice
