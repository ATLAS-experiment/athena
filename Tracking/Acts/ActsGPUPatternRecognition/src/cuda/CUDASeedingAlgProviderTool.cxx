/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "CUDASeedingAlgProviderTool.h"

#include "traccc/cuda/seeding/triplet_seeding_algorithm.hpp"
#include "traccc/cuda/gbts_seeding/gbts_seeding_algorithm.hpp"

#include "ActsInterop/Logger.h"

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

std::pair<std::shared_ptr<const vecmem::copy>, std::shared_ptr<const traccc::device::triplet_seeding_algorithm>>
CUDASeedingAlgProviderTool::getTripletSeedingAlgorithm(const EventContext& ctx, const traccc::seedfinder_config& seedfinder, const traccc::seedfilter_config& seedfilter) const
{

  ATH_MSG_VERBOSE("Constructing CUDA traccc pixel triplet seeding algorithm");
  traccc::memory_resource mr{m_MRs->mainMR(), m_MRs->hostMR()};
  auto copy = m_copy->copy(ctx);

  return std::make_pair(copy, std::make_shared<traccc::cuda::triplet_seeding_algorithm>(
    seedfinder,
    seedfinder,
    seedfilter,
    mr,
    *copy,
    traccc::cuda::stream_wrapper{m_streamTool->stream(ctx)},
    makeActsAthenaLogger(this, "TracccSPFormationCUDA")));

}

std::pair<std::shared_ptr<const vecmem::copy>, std::shared_ptr<const traccc::device::gbts_seeding_algorithm>>
CUDASeedingAlgProviderTool::getGBTSAlgorithm(const EventContext& ctx, const traccc::gbts_seedfinder_config& gbts_config) const
{

  ATH_MSG_VERBOSE("Constructing CUDA traccc pixel GBTS seeding algorithm");
  traccc::memory_resource mr{m_MRs->mainMR(), m_MRs->hostMR()};
  auto copy = m_copy->copy(ctx);

  return std::make_pair(copy, std::make_shared<traccc::cuda::gbts_seeding_algorithm>(
    gbts_config,
    mr,
    *copy,
    traccc::cuda::stream_wrapper{m_streamTool->stream(ctx)},
    makeActsAthenaLogger(this, "TracccGBTSSeedingCUDA")));

}

} // namespace ActsTrk