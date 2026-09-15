/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUPATTERNRECOGNITION_CUDATRACKFINDINGALGPROVIDERTOOL_H
#define ACTSGPUPATTERNRECOGNITION_CUDATRACKFINDINGALGPROVIDERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthDeviceInterfaces/IMemoryResourcesTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "AthCUDAInterfaces/IStreamTool.h"
#include "../IDeviceTrackFindingAlgProviderTool.h"

#include <Gaudi/Property.h>
#include <memory>
#include <string>

namespace ActsTrk {

/**
 * @class CUDATrackFindingAlgProviderTool
 *
 * @brief Tool providing CUDA-based traccc track findng algorithm.
 *
 * This tool constructs the traccc device track reconstruction
 * algorithm, configured to run on with CUDA backend. It relies on
 * separate tools to provide the host/device memory resources and the
 * vecmem copy object.
 * It can accept a traccc device fitter as well, but currently MBF is emplyed
 * ie we are fitting through finding
 *
 * @author Neža Ribarič <neza.ribaric@cern.ch>
 */
class CUDATrackFindingAlgProviderTool
    : public extends<AthAlgTool, IDeviceTrackFindingAlgProviderTool>
{
public:
  using extends::extends;
  /// Function initializing the algorithm
  virtual StatusCode initialize() override;

  /// Function constructing the traccc cuda track finding algorithm
  /// @return cuda track finding algorithm and vecmem copy object

  virtual DeviceAlgorithmT<traccc::device::combinatorial_kalman_filter_algorithm> getAlgorithm(const EventContext& ctx, const traccc::finding_config& trkfinding_config) const override;


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

#endif // ACTSGPUPATTERNRECOGNITION_CUDATRACKFINDINGALGPROVIDERTOOL_H