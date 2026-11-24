/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENTCNV_TRACCCPIXELSPACEPOINTCONVERTERALG_H
#define ACTSGPUEVENTCNV_TRACCCPIXELSPACEPOINTCONVERTERALG_H
#include <vector>

#include "ActsGPUEvent/TracccSpacepointCollection.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
namespace ActsTrk {
/// Copy device Pixel SPs to xAOD and associate their input clusters.
/// @author Taisei Kato
class TracccPixelSpacepointConverterAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;

private:
  ToolHandle<AthDevice::ICopyTool> m_copy{this, "CopyProviderTool", "", "Device copy"};
  ToolHandle<AthDevice::IMemoryResourceTool> m_hostMR{this, "HostMR", "", "Host allocator"};
  SG::ReadHandleKey<traccc::edm::spacepoint_collection::buffer> m_input{this, "InputPixelSpacepoints",
                                                                        "TracccPixelSpacepoints", "GPU Pixel SPs"};
  SG::ReadHandleKey<xAOD::PixelClusterContainer> m_clusters{this, "PixelClusters", "ITkTracccPixelClusters",
                                                            "Converted GPU clusters"};
  SG::ReadHandleKey<std::vector<unsigned int>> m_mapping{this, "InputMeasToPixelCl", "ITkTracccMeasToPixelSP",
                                                         "Measurement index to Pixel cluster index"};
  SG::WriteHandleKey<xAOD::SpacePointContainer> m_output{this, "PixelSpacePoints", "ITkTracccGPUPixelSpacePoints",
                                                         "Converted GPU Pixel SPs"};
};
}  // namespace ActsTrk
#endif
