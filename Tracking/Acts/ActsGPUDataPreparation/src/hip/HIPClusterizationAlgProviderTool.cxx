/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s).
#include "HIPClusterizationAlgProviderTool.h"

// Athena Acts include(s).
#include "ActsInterop/Logger.h"

// Traccc include(s).
#include "traccc/hip/clusterization/clusterization_algorithm.hpp"
#include "traccc/hip/clusterization/measurement_sorting_algorithm.hpp"

namespace ActsTrk {

StatusCode HIPClusterizationAlgProviderTool::initialize() {
  ATH_MSG_DEBUG("Initializing.");

  ATH_CHECK(m_MRs.retrieve());
  ATH_CHECK(m_copy.retrieve());
  ATH_CHECK(m_streamTool.retrieve());

  m_clusteringConfig.sort_cells = m_sortCells;

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

DeviceAlgorithmT<traccc::device::clusterization_algorithm>
HIPClusterizationAlgProviderTool::getClusterizationAlgorithm(
    const EventContext& ctx) const {

  ATH_MSG_VERBOSE("Constructing HIP traccc clusterization algorithm");
  auto copy = m_copy->copy(ctx);

  return {copy,
          std::make_shared<traccc::hip::clusterization_algorithm>(
              traccc::memory_resource{m_MRs->mainMR(), m_MRs->hostMR()}, *copy,
              traccc::hip::stream_wrapper{m_streamTool->stream(ctx)},
              m_clusteringConfig,
              makeActsAthenaLogger(this, "TracccClusterizationHIP"))};
}

DeviceAlgorithmT<IDeviceClusterizationAlgProviderTool::sorting_algorithm_type>
HIPClusterizationAlgProviderTool::getSortingAlgorithm(
    const EventContext& ctx) const {

  ATH_MSG_VERBOSE("Constructing HIP traccc measurement sorting algorithm");
  auto copy = m_copy->copy(ctx);

  return {copy,
          std::make_shared<traccc::hip::measurement_sorting_algorithm>(
              traccc::memory_resource{m_MRs->mainMR(), m_MRs->hostMR()}, *copy,
              traccc::hip::stream_wrapper{m_streamTool->stream(ctx)},
              makeActsAthenaLogger(this, "TracccMeasurementSortingHIP"))};
}

}  // namespace ActsTrk
