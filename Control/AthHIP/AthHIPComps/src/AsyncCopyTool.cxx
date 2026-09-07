//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "AsyncCopyTool.h"

// VecMem include(s).
#include <vecmem/utils/hip/async_copy.hpp>

namespace AthHIP {

StatusCode AsyncCopyTool::initialize() {

  // Retrieve the stream tool.
  ATH_CHECK(m_streamTool.retrieve());

  // Return gracefully.
  return StatusCode::SUCCESS;
}

std::shared_ptr<const vecmem::copy> AsyncCopyTool::copy(
    const EventContext& ctx) const {

  // Create an asynchronous copy object, specific to this slot's HIP stream.
  return std::make_shared<const vecmem::hip::async_copy>(
      m_streamTool->stream(ctx));
}

}  // namespace AthHIP
