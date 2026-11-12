//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

#include "MeasToPixelIndex.h"

#include <vecmem/containers/device_vector.hpp>
#include <thrust/execution_policy.h>
#include <thrust/scan.h>

#include <cstdio>
#include <exception>

/// Helper macro used for checking @c cudaError_t type return values.
#define CUDA_ERROR_CHECK(EXP)                                          \
  do {                                                                 \
    const cudaError_t errorCode = (EXP);                               \
    if (errorCode != cudaSuccess) {                                    \
      std::fprintf(stderr, "CUDA error in %s: %s (%s)\n",              \
                   __func__, #EXP, cudaGetErrorString(errorCode));     \
      return StatusCode::FAILURE;                                      \
    }                                                                  \
  } while (false)

namespace ActsTrk {

namespace kernels {

// flags[i] = 1 for a pixel (2D) measurement, 0 for a strip measurement
__global__ void markPixelMeasurements(
    const traccc::edm::measurement_collection::const_view input_view,
    vecmem::data::vector_view<unsigned int> flags_view)
{
    const unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;
    const traccc::edm::measurement_collection::const_device input(input_view);
    if (index >= input.size()) return;

    vecmem::device_vector<unsigned int> flags(flags_view);
    flags.at(index) = (input.at(index).dimensions() == 2) ? 1u : 0u;
}

// After the exclusive scan, map[i] holds the pixel index for pixels but the
// *next* pixel's index for strips: mark the strips invalid.
__global__ void invalidateStripMeasurements(
    const vecmem::data::vector_view<const unsigned int> flags_view,
    vecmem::data::vector_view<unsigned int> map_view)
{
    const unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;
    const vecmem::device_vector<const unsigned int> flags(flags_view);
    if (index >= flags.size()) return;

    vecmem::device_vector<unsigned int> map(map_view);
    if (flags.at(index) == 0u) map.at(index) = kInvalidPixelIndex;
}

} // namespace kernels

StatusCode buildMeasToPixelIndexOnGPU(
    cudaStream_t stream
  , vecmem::memory_resource & mr
  , vecmem::copy const & copy
  , unsigned int nb_measurements
  , const traccc::edm::measurement_collection::const_view & measurements
  , std::unique_ptr<vecmem::data::vector_buffer<unsigned int>> & output
  )
{
    try {
        output = std::make_unique<vecmem::data::vector_buffer<unsigned int>>(nb_measurements, mr);
        copy.setup(*output)->ignore();
    } catch (const std::exception& e) {
        std::fprintf(stderr, "buildMeasToPixelIndexOnGPU: %s\n", e.what());
        return StatusCode::FAILURE;
    }

    if (nb_measurements == 0) {
        return StatusCode::SUCCESS;
    }

    static const unsigned int block_size = 256;
    const unsigned int num_blocks = (nb_measurements + block_size - 1) / block_size;

    // Mark which measurement is what type (but only pixels)
    vecmem::data::vector_buffer<unsigned int> flags(nb_measurements, mr);
    copy.setup(flags)->ignore();
    kernels::markPixelMeasurements<<<num_blocks, block_size, 0, stream>>>(
        measurements, flags);

    // Scan pixels
    const vecmem::device_vector<const unsigned int> flags_device(flags);
    vecmem::device_vector<unsigned int> map_device(*output);
    try {
        thrust::exclusive_scan(
            thrust::cuda::par_nosync(std::pmr::polymorphic_allocator(&mr)).on(stream),
            flags_device.begin(), flags_device.end(), map_device.begin());
    } catch (const std::exception& e) {
        std::fprintf(stderr, "buildMeasToPixelIndexOnGPU: thrust::exclusive_scan failed: %s\n",
                     e.what());
        return StatusCode::FAILURE;
    }
    // 3. strips -> invalid
    kernels::invalidateStripMeasurements<<<num_blocks, block_size, 0, stream>>>(
        flags, *output);

    // Wait before returning: the local flags buffer is freed on return
    CUDA_ERROR_CHECK(cudaGetLastError());
    CUDA_ERROR_CHECK(cudaStreamSynchronize(stream));

    return StatusCode::SUCCESS;
}

} // namespace ActsTrk
