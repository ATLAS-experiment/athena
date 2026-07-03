/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUDATAPREPARATION_CUDACLUSTERIZATIONALGPROVIDERTOOL_H
#define ACTSGPUDATAPREPARATION_CUDACLUSTERIZATIONALGPROVIDERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "AthCUDAInterfaces/IStreamTool.h"
#include "../IDeviceClusterizationAlgProviderTool.h"

#include "vecmem/utils/cuda/copy.hpp"

#include <memory>
#include <string>

namespace ActsTrk {

class CUDAClusterizationAlgProviderTool
    : public extends<AthAlgTool, IDeviceClusterizationAlgProviderTool>
{
public:
  using extends::extends;
  virtual StatusCode initialize() override;
  virtual StatusCode finalize()   override;

  virtual std::pair<std::shared_ptr<const vecmem::copy>, std::shared_ptr<const traccc::device::clusterization_algorithm>> getClusterizationAlgorithm(const EventContext& ctx) const override;

  virtual std::pair<std::shared_ptr<const vecmem::copy>, std::shared_ptr<const IDeviceClusterizationAlgProviderTool::sorting_algorithm_type>>
    getSortingAlgorithm(const EventContext& ctx) const override;


private:

  traccc::clustering_config m_clusteringConfig{};

  ToolHandle<AthDevice::IMemoryResourceTool> m_hostMR{
    this, "HostMR", "", "Host memory resource tool"};
  ToolHandle<AthDevice::IMemoryResourceTool> m_deviceMR{
    this, "DeviceMR", "", "Device memory resource tool"};
  ToolHandle<AthDevice::ICopyTool> m_copy{
      this, "CopyProviderTool", "", "Vecmem copy provider tool"};
  ToolHandle<AthCUDA::IStreamTool> m_streamTool{
      this, "StreamTool", "", "CUDA stream provider tool"};

};

} // namespace ActsTrk

#endif // ACTSGPUDATAPREPARATION_CUDACLUSTERIZATIONALGPROVIDERTOOL_H