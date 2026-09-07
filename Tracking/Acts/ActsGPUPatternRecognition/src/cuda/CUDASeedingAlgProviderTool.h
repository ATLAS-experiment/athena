/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUPATTERNRECOGNITION_CUDASEEDINGALGPROVIDERTOOL_H
#define ACTSGPUPATTERNRECOGNITION_CUDASEEDINGALGPROVIDERTOOL_H

// Local include(s).
#include "../IDeviceSeedingAlgProviderTool.h"

// Framework include(s).
#include "AthCUDAInterfaces/IStreamTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "AthDeviceInterfaces/IMemoryResourcesTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

namespace ActsTrk {

/**
 * @class CUDASeedingAlgProviderTool
 *
 * @brief Tool providing CUDA-based traccc seeding algorithm.
 *
 * This tool constructs the traccc device seeding
 * algorithm, configured to run on with CUDA backend. It relies on
 * separate tools to provide the host/device memory resources and the
 * vecmem copy object.
 *
 * Currently only pixel seeding is implemented in Traccc
 *
 * @author Neža Ribarič <neza.ribaric@cern.ch>
 */
class CUDASeedingAlgProviderTool
    : public extends<AthAlgTool, IDeviceSeedingAlgProviderTool> {
 public:
  using extends::extends;
  /// Function initializing the algorithm
  virtual StatusCode initialize() override;

  /// Function constructing the traccc cuda seeding algorithm
  /// @return cuda triplet seeding algorithm and vecmem copy object
  virtual DeviceAlgorithmT<traccc::device::triplet_seeding_algorithm>
  getTripletSeedingAlgorithm(
      const EventContext& ctx, const traccc::seedfinder_config& seedfinder,
      const traccc::seedfilter_config& seedfilter) const override;
  /// Get the device specific GBTS seeding algorithm.
  virtual DeviceAlgorithmT<traccc::device::gbts_seeding_algorithm>
  getGBTSAlgorithm(
      const EventContext& ctx,
      const traccc::gbts_seedfinder_config& gbts_config) const override;

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

#endif  // ACTSGPUPATTERNRECOGNITION_CUDASEEDINGALGPROVIDERTOOL_H
