//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "AsyncCopyTool.h"

// VecMem include(s).
#include <vecmem/utils/cuda/async_copy.hpp>

// System include(s).
#include <format>
#include <stdexcept>

/// Helper macro used for checking @c cudaError_t type return values.
#define CUDA_ERROR_CHECK(EXP)                                                  \
  do {                                                                         \
    cudaError_t errorCode = EXP;                                               \
    if (errorCode != cudaSuccess) {                                            \
      throw std::runtime_error(std::format("{}:{} Failed to execute: {} ({})", \
                                           __FILE__, __LINE__, #EXP,           \
                                           cudaGetErrorString(errorCode)));    \
    }                                                                          \
  } while (false)

namespace AthCUDA {

AsyncCopyTool::Stream::Stream() {
  CUDA_ERROR_CHECK(cudaStreamCreate(&m_stream));
}

/// Let's not check for errors here. Since that would introduce a struct that
/// may throw in its destructor. Which we don't want.
AsyncCopyTool::Stream::~Stream() {
  cudaStreamDestroy(m_stream);
}

StatusCode AsyncCopyTool::initialize() {

  // Create the slot specific stream.
  m_streams = std::make_unique<const SG::SlotSpecificObj<Stream>>();

  // Return gracefully.
  return StatusCode::SUCCESS;
}

std::shared_ptr<const vecmem::copy> AsyncCopyTool::copy(
    const EventContext& ctx) const {

  // Create an asynchronous copy object, specific to this slot's CUDA stream.
  cudaStream_t stream = m_streams->get(ctx)->m_stream;
  assert(stream != nullptr);
  return std::make_shared<const vecmem::cuda::async_copy>(stream);
}

}  // namespace AthCUDA
