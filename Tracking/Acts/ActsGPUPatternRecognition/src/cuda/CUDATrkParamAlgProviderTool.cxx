/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "CUDATrkParamAlgProviderTool.h"

#include "traccc/cuda/seeding/seed_parameter_estimation_algorithm.hpp"

#include "ActsInterop/Logger.h"

namespace ActsTrk {

StatusCode CUDATrkParamAlgProviderTool::initialize()
{
  ATH_MSG_DEBUG("Initializing.");

  ATH_CHECK(m_MRs.retrieve());
  ATH_CHECK(m_copy.retrieve());
  ATH_CHECK(m_streamTool.retrieve());

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

std::pair<std::shared_ptr<const vecmem::copy>, std::shared_ptr<const traccc::device::seed_parameter_estimation_algorithm>>
CUDATrkParamAlgProviderTool::getTrkParamAlgorithm(const EventContext& ctx, const traccc::track_params_estimation_config& trkparam_config) const
{

  ATH_MSG_VERBOSE("Constructing CUDA traccc track parameter estimation algorithm");
  traccc::memory_resource mr{m_MRs->mainMR(), m_MRs->hostMR()};
  auto copy = m_copy->copy(ctx);

  return std::make_pair(copy, std::make_shared<traccc::cuda::seed_parameter_estimation_algorithm>(
    trkparam_config,
    mr,
    *copy,
    traccc::cuda::stream_wrapper{m_streamTool->stream(ctx)},
    makeActsAthenaLogger(this, "TracccSPFormationCUDA")));

}

} // namespace ActsTrk