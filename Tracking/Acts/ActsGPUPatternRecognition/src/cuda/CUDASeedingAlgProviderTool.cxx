/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s).
#include "CUDASeedingAlgProviderTool.h"

// Athena Acts include(s).
#include "ActsInterop/Logger.h"

// Traccc include(s).
#include "traccc/cuda/seeding/triplet_seeding_algorithm.hpp"
#include "traccc/cuda/gbts_seeding/gbts_seeding_algorithm.hpp"

namespace ActsTrk {

StatusCode CUDASeedingAlgProviderTool::initialize()
{
  ATH_MSG_DEBUG("Initializing.");

  ATH_CHECK(m_MRs.retrieve());
  ATH_CHECK(m_copy.retrieve());
  ATH_CHECK(m_streamTool.retrieve());

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

DeviceAlgorithmT<traccc::device::triplet_seeding_algorithm>
CUDASeedingAlgProviderTool::getTripletSeedingAlgorithm(const EventContext& ctx, const traccc::seedfinder_config& seedfinder, const traccc::seedfilter_config& seedfilter) const
{

  ATH_MSG_VERBOSE("Constructing CUDA traccc pixel triplet seeding algorithm");
  auto copy = m_copy->copy(ctx);

  return {copy, std::make_shared<traccc::cuda::triplet_seeding_algorithm>(
    seedfinder,
    seedfinder,
    seedfilter,
    traccc::memory_resource{m_MRs->mainMR(), m_MRs->hostMR()},
    *copy,
    traccc::cuda::stream_wrapper{m_streamTool->stream(ctx)},
    makeActsAthenaLogger(this, "TracccSPFormationCUDA"))};
}

DeviceAlgorithmT<traccc::device::gbts_seeding_algorithm>
CUDASeedingAlgProviderTool::getGBTSAlgorithm(const EventContext& ctx, const traccc::gbts_seedfinder_config& gbts_config) const
{

  ATH_MSG_VERBOSE("Constructing CUDA traccc pixel GBTS seeding algorithm");
  auto copy = m_copy->copy(ctx);

  return {copy, std::make_shared<traccc::cuda::gbts_seeding_algorithm>(
    gbts_config,
    traccc::memory_resource{m_MRs->mainMR(), m_MRs->hostMR()},
    *copy,
    traccc::cuda::stream_wrapper{m_streamTool->stream(ctx)},
    makeActsAthenaLogger(this, "TracccGBTSSeedingCUDA"))};

}

} // namespace ActsTrk
