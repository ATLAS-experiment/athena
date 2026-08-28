/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUPATTERNRECOGNITION_CUDATRKPARAMALGPROVIDERTOOL_H
#define ACTSGPUPATTERNRECOGNITION_CUDATRKPARAMALGPROVIDERTOOL_H

// Local include(s).
#include "../IDeviceTrkParamAlgProviderTool.h"

// Framework include(s).
#include "AthCUDAInterfaces/IStreamTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "AthDeviceInterfaces/IMemoryResourcesTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

namespace ActsTrk {

/**
 * @class CUDATrkParamAlgProviderTool
 *
 * @brief Tool providing CUDA-based traccc track parameter estimation algorithm.
 *
 * This tool constructs the traccc device track parameter estimation
 * algorithm, configured to run on with CUDA backend. It relies on
 * separate tools to provide the host/device memory resources and the
 * vecmem copy object.
 *
 * Currently only pixel seeding is implemented in Traccc,
 * so strip seeding and param est. should be appropriately configured.
 *
 * @author Neža Ribarič <neza.ribaric@cern.ch>
 */
class CUDATrkParamAlgProviderTool
    : public extends<AthAlgTool, IDeviceTrkParamAlgProviderTool> {
 public:
  using extends::extends;
  /// Function initializing the algorithm
  virtual StatusCode initialize() override;

  /// Function constructing the traccc cuda track parameter estimation algorithm
  /// @return cuda track parameter estimation algorithm and vecmem copy object
  virtual DeviceAlgorithmT<traccc::device::seed_parameter_estimation_algorithm>
  getAlgorithm(const EventContext& ctx,
               const traccc::track_params_estimation_config& trkparam_config)
      const override;

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

};  // class CUDATrkParamAlgProviderTool

}  // namespace ActsTrk

#endif  // ACTSGPUPATTERNRECOGNITION_CUDATRKPARAMALGPROVIDERTOOL_H
