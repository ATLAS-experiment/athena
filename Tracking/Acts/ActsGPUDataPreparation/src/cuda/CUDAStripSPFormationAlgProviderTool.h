/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUDATAPREPARATION_CUDASTRIPSPFORMATIONALGPROVIDERTOOL_H
#define ACTSGPUDATAPREPARATION_CUDASTRIPSPFORMATIONALGPROVIDERTOOL_H

// Local include(s).
#include "../IDeviceStripSPFormationAlgProviderTool.h"

// Framework include(s).
#include "AthCUDAInterfaces/IStreamTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "AthDeviceInterfaces/IMemoryResourcesTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

namespace ActsTrk {

/**
 * @class CUDAStripSPFormationAlgProviderTool
 *
 * @brief Tool providing CUDA-based traccc spacepoint formation algorithm.
 *
 * This tool constructs the traccc device spacepoint formation
 * algorithm, configured to run with the CUDA backend. It relies on
 * separate tools to provide the host/device memory resources and the
 * vecmem copy object.
 *
 * Returns standard and overlap strip spacepoints separately.
 *
 * @author Taisei Kato
 */
class CUDAStripSPFormationAlgProviderTool
    : public extends<AthAlgTool, IDeviceStripSPFormationAlgProviderTool> {
 public:
  using extends::extends;
  /// Function initializing the algorithm
  virtual StatusCode initialize() override;

  /// Function constructing the traccc CUDA strip spacepoint algorithm
  /// @return cuda spacepoint formation algorithm and vecmem copy object
  virtual DeviceAlgorithmT<
      traccc::device::silicon_strip_spacepoint_formation_algorithm>
  getAlgorithm(const EventContext& ctx) const override;

  std::unique_ptr<StripSpacepointConditions> uploadInputs(
      const StripSpacepointInputs& inputs,
      const EventContext& ctx) const override;

 private:
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

#endif  // ACTSGPUDATAPREPARATION_CUDASTRIPSPFORMATIONALGPROVIDERTOOL_H
