//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "PerComponentStreamTool.h"

// System include(s).
#include <cassert>
#include <iterator>

namespace AthCUDA {

StatusCode PerComponentStreamTool::initialize() {

  // Create the stream object.
  m_stream = std::make_unique<const Details::Stream>();

  // Tell the user what happened.
  ATH_MSG_DEBUG("Initialized CUDA stream on device: " << m_stream->name());

  // Return gracefully.
  return StatusCode::SUCCESS;
}

cudaStream_t PerComponentStreamTool::stream(const EventContext&) const {

  // Get the stream corresponding to the current slot.
  assert(m_stream);
  assert(m_stream->stream());
  return m_stream->stream();
}

}  // namespace AthCUDA
