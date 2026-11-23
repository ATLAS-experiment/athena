/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENTCNV_TRACCCTRKPARAMDEVICECONVERTERKERNEL_H
#define ACTSGPUEVENTCNV_TRACCCTRKPARAMDEVICECONVERTERKERNEL_H

#include "ActsGPUEvent/TracccTrkParam.h"

#include <traccc/edm/seed_collection.hpp>
#include <traccc/edm/spacepoint_collection.hpp>
#include <traccc/edm/track_parameters.hpp>
#include <vecmem/containers/data/vector_view.hpp>

#include <cuda_runtime_api.h>

namespace ActsTrk {

// Launches the kernel filling output[i] with the packed track parameters of
// seed i and the measurement indices of its space points. The kernel is
// launched asynchronously on the given stream; the returned error only covers
// the launch itself.
cudaError_t packTrkParams(
    cudaStream_t stream
  , unsigned int nb_seeds
  , const traccc::bound_track_parameters_collection_types::const_view & parameters
  , const traccc::edm::spacepoint_collection::const_view & spacepoints
  , const traccc::edm::seed_collection::const_view & seeds
  , vecmem::data::vector_view<TracccTrkParam> output
  );

} // namespace ActsTrk

#endif // ACTSGPUEVENTCNV_TRACCCTRKPARAMDEVICECONVERTERKERNEL_H
