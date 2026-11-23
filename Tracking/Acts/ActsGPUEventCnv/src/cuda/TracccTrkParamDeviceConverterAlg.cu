//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

#include "TracccTrkParamDeviceConverterKernel.h"

#include <traccc/definitions/primitives.hpp>
#include <traccc/edm/seed_collection.hpp>
#include <traccc/edm/spacepoint_collection.hpp>
#include <traccc/edm/track_parameters.hpp>
#include <vecmem/containers/device_vector.hpp>

namespace ActsTrk {

/// Separate namespace kernel(s).
namespace kernels {

// One thread per seed: pack the track parameters estimated for the seed and
// gather the measurement indices of its bottom, middle and top space points.
__global__ void packTrkParams(
    const traccc::bound_track_parameters_collection_types::const_view parameters_view
  , const traccc::edm::spacepoint_collection::const_view spacepoints_view
  , const traccc::edm::seed_collection::const_view seeds_view
  , vecmem::data::vector_view<TracccTrkParam> output_view
  )
{
  const unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;

  const traccc::edm::seed_collection::const_device seeds(seeds_view);
  if (index >= seeds.size()) return;

  const traccc::bound_track_parameters_collection_types::const_device parameters(parameters_view);
  const traccc::edm::spacepoint_collection::const_device spacepoints(spacepoints_view);
  vecmem::device_vector<TracccTrkParam> output(output_view);

  TracccTrkParam& out = output.at(index);

  // ---- Track parameters
  const traccc::bound_track_parameters<>& params = parameters.at(index);
  const auto& vec = params.vector();
  for (unsigned int i = 0; i < 6u; ++i) {
    out.parameters[i] = traccc::getter::element(vec, i, 0u);
  }
  // A failed estimate is not handed to the CKF: traccc reports degenerate
  // states with all-zero parameters, and a failed estimate can also leave
  // non-finite values (is_invalid() checks both)
  bool valid = !params.is_invalid();

  const auto& cov = params.covariance();
  unsigned int k = 0;
  for (unsigned int i = 0; i < 5u; ++i) {
    for (unsigned int j = 0; j <= i; ++j) {
      const traccc::scalar value = traccc::getter::element(cov, i, j);
      out.covariance[k++] = value;
      // the CKF needs finite, positive variances
      if (i == j) {
        valid = valid && isfinite(value) && (value > 0);
      }
    }
  }
  out.timeVariance = traccc::getter::element(cov, 5u, 5u);
  valid = valid && isfinite(out.timeVariance) && (out.timeVariance > 0);
  out.hasParameters = valid;
  out.surfaceLink = params.surface_link();

  // ---- Measurement indices of the seed space points
  // PIXEL SEEDS ONLY: a pixel space point is made from a single measurement,
  // so only measurement_index_1 is kept. Strip seeds would also need
  // measurement_index_2 of each space point.
  const auto seed = seeds.at(index);
  const unsigned int sp_indices[3]{
    seed.bottom_index(), seed.middle_index(), seed.top_index()};
  for (unsigned int i = 0; i < 3u; ++i) {
    out.measurementIndices[i] = spacepoints.at(sp_indices[i]).measurement_index_1();
  }
}

} //namespace kernels

cudaError_t packTrkParams(
    cudaStream_t stream
  , unsigned int nb_seeds
  , const traccc::bound_track_parameters_collection_types::const_view & parameters
  , const traccc::edm::spacepoint_collection::const_view & spacepoints
  , const traccc::edm::seed_collection::const_view & seeds
  , vecmem::data::vector_view<TracccTrkParam> output
  )
{
  if (nb_seeds == 0) {
    return cudaSuccess;
  }

  static const unsigned int block_size = 256;
  const unsigned int num_blocks = (nb_seeds + block_size - 1) / block_size;

  kernels::packTrkParams<<<num_blocks, block_size, 0, stream>>>(
    parameters, spacepoints, seeds, output);

  return cudaGetLastError();
}

} // namespace ActsTrk
