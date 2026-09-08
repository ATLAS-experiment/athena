/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUDATAPREPARATION_CUDACLUSTERIZATIONALGPROVIDERTOOL_H
#define ACTSGPUDATAPREPARATION_CUDACLUSTERIZATIONALGPROVIDERTOOL_H

// Local include(s).
#include "../IDeviceClusterizationAlgProviderTool.h"

// Framework include(s).
#include "AthCUDAInterfaces/IStreamTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "AthDeviceInterfaces/IMemoryResourcesTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/ToolHandle.h"

namespace ActsTrk {

/**
 * @class CUDAClusterizationAlgProviderTool
 *
 * @brief Tool providing CUDA-based traccc clusterization and
 *        measurement sorting algorithms.
 *
 * This tool constructs the traccc device clusterization
 * algorithm, configured to run on with CUDA backend. It relies on
 * separate tools to provide the host/device memory resources, the
 * vecmem copy object, and the CUDA stream used for asynchronous
 * execution.
 *
 * @author Neža Ribarič <neza.ribaric@cern.ch>
 */
class CUDAClusterizationAlgProviderTool
    : public extends<AthAlgTool, IDeviceClusterizationAlgProviderTool> {
 public:
  using extends::extends;
  /// Function initializing the algorithm
  virtual StatusCode initialize() override;

  /// Function constructing the traccc cuda clusterization algorithm
  /// @return cuda clusterization algorithm and vecmem copy object
  virtual DeviceAlgorithmT<traccc::device::clusterization_algorithm>
  getClusterizationAlgorithm(const EventContext& ctx) const override;

  /// Function constructing the traccc cuda measurement sorting algorithm
  /// Neccesary because the CKF requires measurements to be sorted by module ID
  /// @return cuda measurement sorting algorithm and vecmem copy object
  virtual DeviceAlgorithmT<sorting_algorithm_type> getSortingAlgorithm(
      const EventContext& ctx) const override;

 private:
  traccc::clustering_config m_clusteringConfig{};

  /// @name Whether to sort traccc cells on GPU prior to clusterization
  Gaudi::Property<bool> m_sortCells{
      this, "CellSorting", true,
      "Whether to sort traccc cells on GPU prior to clusterization"};
  /// @name The host and device memory resources tool to use for memory
  /// allocations
  ToolHandle<AthDevice::IMemoryResourcesTool> m_MRs{
      this, "MemoryResourcesTool", "",
      "The memory resources tool to use for allocating memory on the device"};

  /// @name The device copy tool to use
  ToolHandle<AthDevice::ICopyTool> m_copy{this, "CopyProviderTool", "",
                                          "Vecmem copy provider tool"};
  /// @name The cuda stream provider tool
  ToolHandle<AthCUDA::IStreamTool> m_streamTool{this, "StreamTool", "",
                                                "CUDA stream provider tool"};
};

}  // namespace ActsTrk

#endif  // ACTSGPUDATAPREPARATION_CUDACLUSTERIZATIONALGPROVIDERTOOL_H
