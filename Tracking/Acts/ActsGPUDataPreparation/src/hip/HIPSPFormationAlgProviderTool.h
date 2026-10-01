/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUDATAPREPARATION_HIPSPFORMATIONALGPROVIDERTOOL_H
#define ACTSGPUDATAPREPARATION_HIPSPFORMATIONALGPROVIDERTOOL_H

// Local include(s).
#include "../IDeviceSPFormationAlgProviderTool.h"

// Framework include(s).
#include "AthDeviceInterfaces/ICopyTool.h"
#include "AthDeviceInterfaces/IMemoryResourcesTool.h"
#include "AthHIPInterfaces/IStreamTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

namespace ActsTrk {

/**
 * @class HIPSPFormationAlgProviderTool
 *
 * @brief Tool providing HIP-based traccc spacepoint formation algorithm.
 *
 * This tool constructs the traccc device spacepoint formation
 * algorithm, configured to run on with HIP backend. It relies on
 * separate tools to provide the host/device memory resources and the
 * vecmem copy object.
 *
 * Currently only pixel spacepoint formation is implemented in Traccc
 */
class HIPSPFormationAlgProviderTool
    : public extends<AthAlgTool, IDeviceSPFormationAlgProviderTool> {
 public:
  using extends::extends;
  /// Function initializing the algorithm
  virtual StatusCode initialize() override;

  /// Function constructing the traccc hip clusterization algorithm
  /// @return hip spacepoint formation algorithm and vecmem copy object
  virtual DeviceAlgorithmT<
      traccc::device::silicon_pixel_spacepoint_formation_algorithm>
  getAlgorithm(const EventContext& ctx) const override;

 private:
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

#endif  // ACTSGPUDATAPREPARATION_HIPSPFORMATIONALGPROVIDERTOOL_H
