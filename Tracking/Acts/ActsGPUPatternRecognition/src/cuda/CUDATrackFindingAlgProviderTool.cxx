/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "CUDATrackFindingAlgProviderTool.h"

#include "traccc/cuda/finding/combinatorial_kalman_filter_algorithm.hpp"

#include "ActsInterop/Logger.h"

namespace ActsTrk {

StatusCode CUDATrackFindingAlgProviderTool::initialize()
{
  ATH_MSG_DEBUG("Initializing.");

  ATH_CHECK(m_MRs.retrieve());
  ATH_CHECK(m_copy.retrieve());
  ATH_CHECK(m_streamTool.retrieve());

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

DeviceAlgorithmT<traccc::device::combinatorial_kalman_filter_algorithm>
CUDATrackFindingAlgProviderTool::getAlgorithm(const EventContext& ctx, const traccc::finding_config& trkfinding_config) const
{

  ATH_MSG_VERBOSE("Constructing CUDA traccc track finding algorithm");
  
  auto copy = m_copy->copy(ctx);

  return {copy, std::make_shared<traccc::cuda::combinatorial_kalman_filter_algorithm>(
    trkfinding_config,
    traccc::memory_resource{m_MRs->mainMR(), m_MRs->hostMR()},
    *copy,
    traccc::cuda::stream_wrapper{m_streamTool->stream(ctx)},
    makeActsAthenaLogger(this, "TracccTrackFindingCUDA"),
    nullptr)};

}

} // namespace ActsTrk