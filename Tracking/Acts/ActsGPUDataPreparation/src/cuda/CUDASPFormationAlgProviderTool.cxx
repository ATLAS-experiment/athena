/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "CUDASPFormationAlgProviderTool.h"

#include "traccc/cuda/seeding/silicon_pixel_spacepoint_formation_algorithm.hpp"

#include "ActsInterop/Logger.h"

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

std::pair<std::shared_ptr<const vecmem::copy>, std::shared_ptr<const traccc::device::silicon_pixel_spacepoint_formation_algorithm>>
CUDASPFormationAlgProviderTool::getPixelSPFormationAlgorithm(const EventContext& ctx) const
{

  ATH_MSG_VERBOSE("Constructing CUDA traccc pixel spacepoint formation algorithm");
  traccc::memory_resource mr{m_MRs->mainMR(), m_MRs->hostMR()};
  auto copy = m_copy->copy(ctx);

  return std::make_pair(copy, std::make_shared<traccc::cuda::silicon_pixel_spacepoint_formation_algorithm>(
    mr,
    *copy,
    traccc::cuda::stream_wrapper{m_streamTool->stream(ctx)},
    makeActsAthenaLogger(this, "TracccSPFormationCUDA")));

}

} // namespace ActsTrk