/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_DEVICESEEDCONTAINERS_H
#define ACTSGPUEVENT_DEVICESEEDCONTAINERS_H

// Device-side interface of the seed conversion. Included by the .cu (nvcc)
// and the algorithm header: keep Athena algorithm/StoreGate headers OUT.

#include "GaudiKernel/StatusCode.h"

#include "traccc/definitions/qualifiers.hpp"
#include <traccc/edm/seed_collection.hpp>
#include <traccc/edm/spacepoint_collection.hpp>
#include <vecmem/containers/data/vector_view.hpp>
#include <vecmem/edm/container.hpp>
#include <vecmem/memory/memory_resource.hpp>
#include <vecmem/utils/copy.hpp>

#include <cuda_runtime_api.h>

#include <array>
#include <memory>

namespace ActsTrk {

// vecmem objects use this type for size
using size_type = unsigned int;

/// Spacepoints per seed (bottom, middle, top)
inline constexpr unsigned int kDeviceSeedNSp = 3u;
/// No host index marker (as in measMap)
inline constexpr unsigned int kDeviceSeedInvalidIndex = 0xFFFFFFFFu;

/// One entry per traccc seed, in seed order
template <typename BASE>
class DeviceSeed : public BASE {
    public:
    using BASE::BASE;
    using BASE::operator=;

    /// @return array of 3 host xAOD SP indices (bottom, middle, top),
    ///         kDeviceSeedInvalidIndex where the lookup failed
    TRACCC_HOST_DEVICE
    auto& sp_indices() { return BASE::template get<0>(); }
    TRACCC_HOST_DEVICE
    const auto& sp_indices() const { return BASE::template get<0>(); }

    /// @return float, seed quality from traccc
    TRACCC_HOST_DEVICE
    auto& quality() { return BASE::template get<1>(); }
    TRACCC_HOST_DEVICE
    const auto& quality() const { return BASE::template get<1>(); }
};

using DeviceSeedContainer = vecmem::edm::container<
    DeviceSeed
    // sp_indices
    , vecmem::edm::type::vector<std::array<unsigned int, kDeviceSeedNSp>>
    // quality
    , vecmem::edm::type::vector<float>
    >;

/// Seeds -> host SP indices + quality. The output buffer is always allocated,
/// also for nb_seeds == 0. Defined in TracccSeedDeviceConverterAlg.cu.
StatusCode convertSeedsOnGPU(
    cudaStream_t stream
  , vecmem::memory_resource & mr
  , vecmem::copy const & copy
  , size_type nb_seeds
  , const traccc::edm::seed_collection::const_view & input_seeds
  , const traccc::edm::spacepoint_collection::const_view & input_spacepoints
  , const vecmem::data::vector_view<const unsigned int> & meas_to_host_sp
  , std::unique_ptr<DeviceSeedContainer::buffer> & output_seeds
  );

}  // namespace ActsTrk

#endif // ACTSGPUEVENT_DEVICESEEDCONTAINERS_H