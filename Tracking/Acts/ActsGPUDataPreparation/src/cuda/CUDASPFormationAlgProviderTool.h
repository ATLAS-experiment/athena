/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUDATAPREPARATION_CUDASPFORMATIONALGPROVIDERTOOL_H
#define ACTSGPUDATAPREPARATION_CUDASPFORMATIONALGPROVIDERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthDeviceInterfaces/IMemoryResourcesTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "AthCUDAInterfaces/IStreamTool.h"
#include "../IDeviceSPFormationAlgProviderTool.h"

#include "vecmem/utils/cuda/copy.hpp"

#include <Gaudi/Property.h>
#include <memory>
#include <string>

namespace ActsTrk {

/**
 * @class CUDASPFormationAlgProviderTool
 *
 * @brief Tool providing CUDA-based traccc spacepoint formation algorithm.
 *
 * This tool constructs the traccc device spacepoint formation
 * algorithm, configured to run on with CUDA backend. It relies on
 * separate tools to provide the host/device memory resources and the
 * vecmem copy object.
 * 
 * Currently only pixel spacepoint formation is implemented in Traccc
 *
 * @author Neža Ribarič <neza.ribaric@cern.ch>
 */
class CUDASPFormationAlgProviderTool
    : public extends<AthAlgTool, IDeviceSPFormationAlgProviderTool>
{
public:
  using extends::extends;
  /// Function initializing the algorithm
  virtual StatusCode initialize() override;

  /// Function constructing the traccc cuda clusterization algorithm
  /// @return cuda spacepoint formation algorithm and vecmem copy object
  virtual std::pair<std::shared_ptr<const vecmem::copy>, std::shared_ptr<const traccc::device::silicon_pixel_spacepoint_formation_algorithm>> getPixelSPFormationAlgorithm(const EventContext& ctx) const override;


private:

  /// @name The host and device memory resources tool to use for memory allocations
  ToolHandle<AthDevice::IMemoryResourcesTool> m_MRs{
        this, "MemoryResourcesTool", "",
        "The memory resources tool to use for allocating memory on the device"};

  /// @name The device copy tool to use
  ToolHandle<AthDevice::ICopyTool> m_copy{
      this, "CopyProviderTool", "", "Vecmem copy provider tool"};
  /// @name The cuda stream provider tool
  ToolHandle<AthCUDA::IStreamTool> m_streamTool{
      this, "StreamTool", "", "CUDA stream provider tool"};    

};

} // namespace ActsTrk

#endif // ACTSGPUDATAPREPARATION_CUDASPFORMATIONALGPROVIDERTOOL_H