/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s).
#include "CUDATrkParamAlgProviderTool.h"

// Athena Acts include(s).
#include "ActsInterop/Logger.h"

// Traccc include(s).
#include "traccc/cuda/seeding/seed_parameter_estimation_algorithm.hpp"

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

DeviceAlgorithmT<traccc::device::seed_parameter_estimation_algorithm>
CUDATrkParamAlgProviderTool::getAlgorithm(const EventContext& ctx, const traccc::track_params_estimation_config& trkparam_config) const
{

  ATH_MSG_VERBOSE("Constructing CUDA traccc track parameter estimation algorithm");
  auto copy = m_copy->copy(ctx);

  return {copy, std::make_shared<traccc::cuda::seed_parameter_estimation_algorithm>(
    trkparam_config,
    traccc::memory_resource{m_MRs->mainMR(), m_MRs->hostMR()},
    *copy,
    traccc::cuda::stream_wrapper{m_streamTool->stream(ctx)},
    makeActsAthenaLogger(this, "TracccSPFormationCUDA"))};

}

} // namespace ActsTrk
