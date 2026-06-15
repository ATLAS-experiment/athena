//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Framework include(s).
#include "AthenaKernel/errorcheck.h"
#include "GaudiKernel/StatusCode.h"

// Traccc include(s).
#include <traccc/edm/track_collection.hpp>

/// Helper macro used for checking @c cudaError_t type return values.
#define CUDA_ERROR_CHECK(EXP)                                      \
  do {                                                             \
    cudaError_t errorCode = EXP;                                   \
    if (errorCode != cudaSuccess) {                                \
      errorcheck::ReportMessage(MSG::ERROR, ERRORCHECK_ARGS,       \
                                "AthCUDAExamples::calibrateOnGPU", \
                                StatusCode::FAILURE)               \
              .msgstream()                                         \
          << "Failed to execute: " << #EXP << " ("                 \
          << cudaGetErrorString(errorCode) << ")";                 \
      return StatusCode::FAILURE;                                  \
    }                                                              \
  } while (false)

namespace AthCUDAExamples {

/// Separate namespace for the example CUDA kernel(s).
namespace kernels {

/// Dummy kernel performing a trivial transformation on the track particle
/// parameters.
__global__ void trackParticleCalibrate(
    const traccc::edm::track_collection<traccc::default_algebra>::const_view
        input_view,
    traccc::edm::track_collection<traccc::default_algebra>::view output_view) {

  // Get the current thread's index.
  const unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;

  // Create the device containers.
  traccc::edm::track_collection<traccc::default_algebra>::const_device input(
      input_view);
  traccc::edm::track_collection<traccc::default_algebra>::device output(
      output_view);
  assert(input.size() == output.size());

  // Check that the index is in range.
  if (index < input.size()) {
    // Copy the angle parameters as they are.
    output[index].params().set_theta(input.params()[index].theta());
    output.at(index).params().set_phi(input.params().at(index).phi());

    // Transform the momentum in some silly way.
    output[index].params().set_qop(
        input[index].params().qop() *
        std::abs((input[index].params().theta() - input[index].params().phi()) /
                 input[index].params().phi()));
  }

  return;
}

}  // namespace kernels

StatusCode calibrateOnGPU(
    cudaStream_t stream,
    const traccc::edm::track_collection<traccc::default_algebra>::const_view&
        input,
    traccc::edm::track_collection<traccc::default_algebra>::view& output) {

  // Launch the kernel.
  static const unsigned int block_size = 256;
  const unsigned int num_blocks =
      (input.capacity() + block_size - 1) / block_size;
  kernels::trackParticleCalibrate<<<num_blocks, block_size, 0, stream>>>(
      input, output);

  // Check for errors, and wait for the kernel to finish.
  CUDA_ERROR_CHECK(cudaGetLastError());
  CUDA_ERROR_CHECK(cudaDeviceSynchronize());

  // Return gracefully.
  return StatusCode::SUCCESS;
}

}  // namespace AthCUDAExamples
