//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "PerEventAndComponentStreamTool.h"

// System include(s).
#include <cassert>
#include <iterator>

namespace AthCUDA {

StatusCode PerEventAndComponentStreamTool::initialize() {

  // Create the stream object.
  m_streams = std::make_unique<
      const SG::SlotSpecificObj<Details::Stream, SG::InvalidSlot::Enabled>>();

  // Tell the user what happened.
  ATH_MSG_DEBUG("Initialized "
                << std::distance(m_streams->begin(), m_streams->end())
                << " CUDA stream(s):");
  for (const auto& stream : *m_streams) {
    ATH_MSG_DEBUG("  - " << stream.name());
  }

  // Return gracefully.
  return StatusCode::SUCCESS;
}

cudaStream_t PerEventAndComponentStreamTool::stream(
    const EventContext& ctx) const {

  // Get the stream corresponding to the current slot.
  assert(m_streams);
  return m_streams->get(ctx)->stream();
}

}  // namespace AthCUDA
