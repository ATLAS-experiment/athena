//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "TrackParticleContainer.h"

// Framework include(s).
#include "AthenaKernel/errorcheck.h"
#include "GaudiKernel/StatusCode.h"

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
    const TrackParticleContainer::const_view input_view,
    TrackParticleContainer::view output_view) {

  // Get the current thread's index.
  const unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;

  // Create the device containers.
  TrackParticleContainer::const_device input(input_view);
  TrackParticleContainer::device output(output_view);
  assert(input.size() == output.size());

  // Check that the index is in range.
  if (index < input.size()) {
    // Copy the angle parameters as they are.
    output.theta()[index] = input.theta()[index];
    output.phi()[index] = input.phi()[index];

    // Transform the momentum in some silly way.
    output.qOverP()[index] =
        input.qOverP()[index] *
        std::abs((input.theta()[index] - input.phi()[index]) /
                 input.phi()[index]);
  }

  return;
}

}  // namespace kernels

StatusCode calibrateOnGPU(const TrackParticleContainer::const_view& input,
                          TrackParticleContainer::view& output) {

  // Launch the kernel.
  static const unsigned int block_size = 256;
  const unsigned int num_blocks =
      (input.capacity() + block_size - 1) / block_size;
  kernels::trackParticleCalibrate<<<num_blocks, block_size>>>(input, output);

  // Check for errors, and wait for the kernel to finish.
  CUDA_ERROR_CHECK(cudaGetLastError());
  CUDA_ERROR_CHECK(cudaDeviceSynchronize());

  // Return gracefully.
  return StatusCode::SUCCESS;
}

}  // namespace AthCUDAExamples
