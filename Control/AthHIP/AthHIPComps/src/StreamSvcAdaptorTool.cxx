//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "StreamSvcAdaptorTool.h"

// System include(s).
#include <cassert>

namespace AthHIP {

StatusCode StreamSvcAdaptorTool::initialize() {

  // Retrieve the "wrapped" service.
  ATH_CHECK(m_svc.retrieve());

  // Return gracefully.
  return StatusCode::SUCCESS;
}

hipStream_t StreamSvcAdaptorTool::stream(const EventContext& ctx) const {

  // Just return the stream provided by the service.
  assert(m_svc.isValid());
  return m_svc->stream(ctx);
}

}  // namespace AthHIP
