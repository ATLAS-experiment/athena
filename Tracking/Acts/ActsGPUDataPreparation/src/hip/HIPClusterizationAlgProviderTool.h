/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUDATAPREPARATION_HIPCLUSTERIZATIONALGPROVIDERTOOL_H
#define ACTSGPUDATAPREPARATION_HIPCLUSTERIZATIONALGPROVIDERTOOL_H

// Local include(s).
#include "../IDeviceClusterizationAlgProviderTool.h"

// Framework include(s).
#include "AthDeviceInterfaces/ICopyTool.h"
#include "AthDeviceInterfaces/IMemoryResourcesTool.h"
#include "AthHIPInterfaces/IStreamTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/ToolHandle.h"

namespace ActsTrk {

/**
 * @class HIPClusterizationAlgProviderTool
 *
 * @brief Tool providing HIP-based traccc clusterization and
 *        measurement sorting algorithms.
 *
 * This tool constructs the traccc device clusterization
 * algorithm, configured to run on with HIP backend. It relies on
 * separate tools to provide the host/device memory resources, the
 * vecmem copy object, and the HIP stream used for asynchronous
 * execution.
 */
class HIPClusterizationAlgProviderTool
    : public extends<AthAlgTool, IDeviceClusterizationAlgProviderTool> {
 public:
  using extends::extends;
  /// Function initializing the algorithm
  virtual StatusCode initialize() override;

  /// Function constructing the traccc hip clusterization algorithm
  /// @return hip clusterization algorithm and vecmem copy object
  virtual DeviceAlgorithmT<traccc::device::clusterization_algorithm>
  getClusterizationAlgorithm(const EventContext& ctx) const override;

  /// Function constructing the traccc hip measurement sorting algorithm
  /// Neccesary because the CKF requires measurements to be sorted by module ID
  /// @return hip measurement sorting algorithm and vecmem copy object
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
  /// @name The hip stream provider tool
  ToolHandle<AthHIP::IStreamTool> m_streamTool{this, "StreamTool", "",
                                               "HIP stream provider tool"};
};

}  // namespace ActsTrk

#endif  // ACTSGPUDATAPREPARATION_HIPCLUSTERIZATIONALGPROVIDERTOOL_H
