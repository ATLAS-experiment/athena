/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s).
#include "CUDAClusterizationAlgProviderTool.h"

// Athena Acts include(s).
#include "ActsInterop/Logger.h"

// Traccc include(s).
#include "traccc/cuda/clusterization/clusterization_algorithm.hpp"
#include "traccc/cuda/clusterization/measurement_sorting_algorithm.hpp"

namespace ActsTrk {

StatusCode CUDAClusterizationAlgProviderTool::initialize()
{
  ATH_MSG_DEBUG("Initializing.");

  ATH_CHECK(m_MRs.retrieve());
  ATH_CHECK(m_copy.retrieve());
  ATH_CHECK(m_streamTool.retrieve());

  m_clusteringConfig.sort_cells = m_sortCells;

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

DeviceAlgorithmT<traccc::device::clusterization_algorithm>
CUDAClusterizationAlgProviderTool::getClusterizationAlgorithm(const EventContext& ctx) const
{

  ATH_MSG_VERBOSE("Constructing CUDA traccc clusterization algorithm");
  auto copy = m_copy->copy(ctx);

  return {copy, std::make_shared<traccc::cuda::clusterization_algorithm>(
      traccc::memory_resource{m_MRs->mainMR(), m_MRs->hostMR()},
      *copy,
      traccc::cuda::stream_wrapper{m_streamTool->stream(ctx)},
      m_clusteringConfig,
      makeActsAthenaLogger(this, "TracccClusterizationCUDA"))};

}

DeviceAlgorithmT<IDeviceClusterizationAlgProviderTool::sorting_algorithm_type>
CUDAClusterizationAlgProviderTool::getSortingAlgorithm(const EventContext& ctx) const
{

  ATH_MSG_VERBOSE("Constructing CUDA traccc measurement sorting algorithm");
  auto copy = m_copy->copy(ctx);

  return {copy, std::make_shared<traccc::cuda::measurement_sorting_algorithm>(
      traccc::memory_resource{m_MRs->mainMR(), m_MRs->hostMR()},
      *copy,
      traccc::cuda::stream_wrapper{m_streamTool->stream(ctx)},
      makeActsAthenaLogger(this, "TracccMeasurementSortingCUDA"))};

}

} // namespace ActsTrk
