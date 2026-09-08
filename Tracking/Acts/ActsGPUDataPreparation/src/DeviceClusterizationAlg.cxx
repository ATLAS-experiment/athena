/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "DeviceClusterizationAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

// traccc EDM
#include "traccc/edm/silicon_cell_collection.hpp"
#include "traccc/edm/measurement_collection.hpp"

// vecmem
#include "vecmem/memory/memory_resource.hpp"

namespace ActsTrk {

// -----------------------------------------------------------------------
StatusCode DeviceClusterizationAlg::initialize()
{
  ATH_MSG_DEBUG("Initializing " << name());

  ATH_CHECK(m_clusteringAlgProviderTool.retrieve());
  ATH_CHECK(m_deviceMR.retrieve());
  ATH_CHECK(m_inputCellsKey.initialize());
  ATH_CHECK(m_outputMeasKey.initialize());
  ATH_CHECK(m_outputClusterKey.initialize());

  ATH_CHECK(detStore()->retrieve(m_deviceDesign, m_deviceDesignObjectName.value()));
  ATH_CHECK(detStore()->retrieve(m_deviceCond, m_deviceCondObjectName.value()));

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

StatusCode DeviceClusterizationAlg::execute(const EventContext& ctx) const
{
  ATH_MSG_DEBUG("Executing device clusterization.");

  // ---- 1. Read input traccc cells from StoreGate --------------------------------
  auto inputTracccCells = SG::makeHandle(m_inputCellsKey, ctx);
  ATH_CHECK(inputTracccCells.isValid());
  ATH_MSG_DEBUG("Read traccc cells from '"
                         << m_inputCellsKey.key() << "'");

  // ---- 2. Get traccc clusterization alg ---------------------------------------------
  auto clustering_alg = m_clusteringAlgProviderTool->getClusterizationAlgorithm(ctx);

  // ---- 2.5 Retrieve the sorting algorithm ---------------------------------------------
  auto sorting_alg = m_clusteringAlgProviderTool->getSortingAlgorithm(ctx);

  // ---- 3. Run traccc clusterization ---------------------------------------------
  traccc::edm::silicon_cluster_collection::buffer cluster_gpu_buffer;
  traccc::edm::measurement_collection::buffer measurements_gpu_buffer;

  if(m_retrieveClusterCells){
    ATH_MSG_DEBUG("Running clusterization with returning cell info");
    std::tie(measurements_gpu_buffer, cluster_gpu_buffer) =
                (*clustering_alg)(
                    *inputTracccCells, *m_deviceDesign, *m_deviceCond,
                    traccc::device::clustering_keep_disjoint_set{});
  } else {
    ATH_MSG_DEBUG("Running clusterization without returning cell info");
    measurements_gpu_buffer = (*clustering_alg)(*inputTracccCells, *m_deviceDesign, *m_deviceCond);
  }

  // ---- 3.5 Run measurement sorting ---------------------------------------------
  auto sortedTracccMeasurements =
      (*sorting_alg)(measurements_gpu_buffer);

  ATH_MSG_DEBUG("Reconstructed " << clustering_alg.copy().get_size(measurements_gpu_buffer) << " measurements.");

  // ---- 4. Write output traccc measurements to StoreGate -------------------------
  auto outputTracccMeas = SG::makeHandle(m_outputMeasKey, ctx);
  ATH_CHECK(outputTracccMeas.record(
    std::make_unique<traccc::edm::measurement_collection::buffer>(
        std::move(sortedTracccMeasurements))));
  ATH_MSG_DEBUG("Wrote measurement buffer to '" << m_outputMeasKey.key() << "'");

  auto outputTracccClusters = SG::makeHandle(m_outputClusterKey, ctx);
  ATH_CHECK(outputTracccClusters.record(
    std::make_unique<traccc::edm::silicon_cluster_collection::buffer>(
        std::move(cluster_gpu_buffer))));
  ATH_MSG_DEBUG("Wrote cluster buffer to '" << m_outputClusterKey.key() << "'");

  return StatusCode::SUCCESS;
}

} // namespace ActsTrk
