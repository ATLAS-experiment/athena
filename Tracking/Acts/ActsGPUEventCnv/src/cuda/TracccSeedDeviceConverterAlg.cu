//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

#include "src/DeviceSeedContainers.h"

#include <vecmem/containers/device_vector.hpp>

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

// One thread per seed. For each of its 3 spacepoints:
// device SP index -> measurement index -> host pixel SP index.
// Entry i of the output belongs to traccc seed i.
__global__ void convertSeeds(
    const traccc::edm::seed_collection::const_view seeds_view,
    const traccc::edm::spacepoint_collection::const_view sps_view,
    const vecmem::data::vector_view<const unsigned int> meas_to_sp_view,
    DeviceSeedContainer::view output_view)
{
    const unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;

    const traccc::edm::seed_collection::const_device seeds(seeds_view);
    if (index >= seeds.size()) return;

    const traccc::edm::spacepoint_collection::const_device sps(sps_view);
    const vecmem::device_vector<const unsigned int> meas_to_sp(meas_to_sp_view);
    DeviceSeedContainer::device output(output_view);
    assert(seeds.size() == output.size());

    const unsigned int sp_device_index[kDeviceSeedNSp] = {
        seeds.bottom_index().at(index),
        seeds.middle_index().at(index),
        seeds.top_index().at(index)};

    auto& out_sp = output[index].sp_indices();
    for (unsigned int k = 0; k < kDeviceSeedNSp; ++k) {
        unsigned int sp_index = kDeviceSeedInvalidIndex;
        const unsigned int sp_idx = sp_device_index[k];
        if (sp_idx < sps.size()) {
            const auto sp = sps.at(sp_idx);
            const unsigned int meas_idx = sp.measurement_index_1();
            // pixel SPs only: the map is to pixel spacepoints
            if (sp.measurement_index_2() ==
                    traccc::edm::spacepoint_collection::device::INVALID_MEASUREMENT_INDEX &&
                meas_idx < meas_to_sp.size()) {
                sp_index = meas_to_sp.at(meas_idx);
            }
        }
        out_sp[k] = sp_index;
    }

    output[index].quality() = seeds.quality().at(index);
}

} // namespace kernels

StatusCode convertSeedsOnGPU(
    cudaStream_t stream
  , vecmem::memory_resource & mr
  , vecmem::copy const & copy
  , size_type nb_seeds
  , const traccc::edm::seed_collection::const_view & input_seeds
  , const traccc::edm::spacepoint_collection::const_view & input_spacepoints
  , const vecmem::data::vector_view<const unsigned int> & meas_to_host_sp
  , std::unique_ptr<DeviceSeedContainer::buffer> & output_seeds
)
{
    try {
        output_seeds = std::make_unique<DeviceSeedContainer::buffer>(nb_seeds, mr);
        copy.setup(*output_seeds)->ignore();
    } catch (const std::exception& e) {
        std::fprintf(stderr, "convertSeedsOnGPU: failed to allocate the output buffer: %s\n",
                     e.what());
        return StatusCode::FAILURE;
    }

    if (nb_seeds == 0) {
        return StatusCode::SUCCESS;
    }

    static const unsigned int block_size = 256;
    const unsigned int num_blocks = (nb_seeds + block_size - 1) / block_size;

    kernels::convertSeeds<<<num_blocks, block_size, 0, stream>>>(
        input_seeds, input_spacepoints, meas_to_host_sp, *output_seeds);

    CUDA_ERROR_CHECK(cudaGetLastError());
    CUDA_ERROR_CHECK(cudaStreamSynchronize(stream));

    return StatusCode::SUCCESS;
}

} // namespace ActsTrk