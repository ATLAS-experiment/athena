//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "PerEventStreamSvc.h"

// System include(s).
#include <cassert>
#include <iterator>

namespace AthHIP {

StatusCode PerEventStreamSvc::initialize() {

  // Create the streams object.
  m_streams = std::make_unique<
      const SG::SlotSpecificObj<Details::Stream, SG::InvalidSlot::Enabled>>();

  // Tell the user what happened.
  ATH_MSG_INFO("Initialized "
               << std::distance(m_streams->begin(), m_streams->end())
               << " HIP stream(s):");
  for (const auto& stream : *m_streams) {
    ATH_MSG_INFO("  - " << stream.name());
  }

  // Return gracefully.
  return StatusCode::SUCCESS;
}

hipStream_t PerEventStreamSvc::stream(const EventContext& ctx) const {

  // Get the stream corresponding to the current slot.
  assert(m_streams);
  return m_streams->get(ctx)->stream();
}

}  // namespace AthHIP
