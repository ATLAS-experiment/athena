/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s).
#include "CUDASPFormationAlgProviderTool.h"

// Athena Acts include(s).
#include "ActsInterop/Logger.h"

// Traccc include(s).
#include "traccc/cuda/seeding/silicon_pixel_spacepoint_formation_algorithm.hpp"

namespace ActsTrk {

StatusCode CUDASPFormationAlgProviderTool::initialize()
{
  ATH_MSG_DEBUG("Initializing.");

  ATH_CHECK(m_MRs.retrieve());
  ATH_CHECK(m_copy.retrieve());
  ATH_CHECK(m_streamTool.retrieve());

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

DeviceAlgorithmT<traccc::device::silicon_pixel_spacepoint_formation_algorithm>
CUDASPFormationAlgProviderTool::getAlgorithm(const EventContext& ctx) const
{

  ATH_MSG_VERBOSE("Constructing CUDA traccc pixel spacepoint formation algorithm");

  auto copy = m_copy->copy(ctx);

  return {copy, std::make_shared<traccc::cuda::silicon_pixel_spacepoint_formation_algorithm>(
    traccc::memory_resource{m_MRs->mainMR(), m_MRs->hostMR()},
    *copy,
    traccc::cuda::stream_wrapper{m_streamTool->stream(ctx)},
    makeActsAthenaLogger(this, "TracccSPFormationCUDA"))};

}

} // namespace ActsTrk
