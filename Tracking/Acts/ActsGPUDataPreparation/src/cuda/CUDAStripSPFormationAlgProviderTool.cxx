/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s).
#include "CUDAStripSPFormationAlgProviderTool.h"

#include <vecmem/memory/cuda/device_memory_resource.hpp>

#include "../StripSpacepointAdapter.h"

// Athena Acts include(s).
#include "ActsInterop/Logger.h"

// Traccc include(s).
#include "traccc/cuda/seeding/silicon_strip_spacepoint_formation_algorithm.hpp"

namespace ActsTrk {

StatusCode CUDAStripSPFormationAlgProviderTool::initialize() {
  ATH_MSG_DEBUG("Initializing.");

  ATH_CHECK(m_MRs.retrieve());
  ATH_CHECK(m_copy.retrieve());
  ATH_CHECK(m_streamTool.retrieve());

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

DeviceAlgorithmT<traccc::device::silicon_strip_spacepoint_formation_algorithm>
CUDAStripSPFormationAlgProviderTool::getAlgorithm(
    const EventContext& ctx) const {

  ATH_MSG_VERBOSE(
      "Constructing CUDA traccc strip spacepoint formation algorithm");

  auto copy = m_copy->copy(ctx);

  return {copy,
          std::make_shared<
              traccc::cuda::silicon_strip_spacepoint_formation_algorithm>(
              traccc::memory_resource{m_MRs->mainMR(), m_MRs->hostMR()}, *copy,
              traccc::cuda::stream_wrapper{m_streamTool->stream(ctx)},
              makeActsAthenaLogger(this, "TracccStripSPFormationCUDA"))};
}

std::unique_ptr<StripSpacepointConditions>
CUDAStripSPFormationAlgProviderTool::uploadInputs(
    const StripSpacepointInputs& inputs, const EventContext& ctx) const {
  // Conditions can outlive this Tool. Own the device allocator together with
  // the buffers instead of borrowing the event allocator's service lifetime.
  auto output = std::make_unique<StripSpacepointConditions>(
      std::make_unique<vecmem::cuda::device_memory_resource>(),
      inputs.surface_infos.size(), inputs.pairing_rules.size());
  auto copy = m_copy->copy(ctx);
  copy->setup(output->surfaces)->wait();
  copy->setup(output->rules)->wait();
  (*copy)(vecmem::get_data(inputs.surface_infos), output->surfaces)->wait();
  (*copy)(vecmem::get_data(inputs.pairing_rules), output->rules)->wait();
  return output;
}

}  // namespace ActsTrk
