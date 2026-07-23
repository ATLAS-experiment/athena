/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "CUDAClusterizationAlgProviderTool.h"

#include "traccc/clusterization/clusterization_algorithm.hpp"
#include "traccc/cuda/clusterization/clusterization_algorithm.hpp"
#include "traccc/cuda/clusterization/measurement_sorting_algorithm.hpp"

#include "ActsInterop/Logger.h"

namespace ActsTrk {

StatusCode CUDAClusterizationAlgProviderTool::initialize()
{
  ATH_MSG_DEBUG("Initializing.");

  ATH_CHECK(m_MRs.retrieve());
  ATH_CHECK(m_copy.retrieve());
  ATH_CHECK(m_streamTool.retrieve());

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

std::pair<std::shared_ptr<const vecmem::copy>, std::shared_ptr<const traccc::device::clusterization_algorithm>>
CUDAClusterizationAlgProviderTool::getClusterizationAlgorithm(const EventContext& ctx) const
{

  ATH_MSG_VERBOSE("Constructing CUDA traccc clusterization algorithm");
  traccc::memory_resource mr{m_MRs->mainMR(), m_MRs->hostMR()};
  auto copy = m_copy->copy(ctx);

  return std::make_pair(copy, std::make_shared<traccc::cuda::clusterization_algorithm>(
      mr,
      *copy,
      traccc::cuda::stream_wrapper{m_streamTool->stream(ctx)},
      m_clusteringConfig,
      makeActsAthenaLogger(this, "TracccClusterizationCUDA")));

}

std::pair<std::shared_ptr<const vecmem::copy>, std::shared_ptr<const IDeviceClusterizationAlgProviderTool::sorting_algorithm_type>>
CUDAClusterizationAlgProviderTool::getSortingAlgorithm(const EventContext& ctx) const
{

  ATH_MSG_VERBOSE("Constructing CUDA traccc measurement sorting algorithm");
  traccc::memory_resource mr{m_MRs->mainMR(), m_MRs->hostMR()};
  auto copy = m_copy->copy(ctx);

  return std::make_pair(copy, std::make_shared<traccc::cuda::measurement_sorting_algorithm>(
      mr,
      *copy,
      traccc::cuda::stream_wrapper{m_streamTool->stream(ctx)},
      makeActsAthenaLogger(this, "TracccMeasurementSortingCUDA")));

}

} // namespace ActsTrk