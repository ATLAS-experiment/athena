/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUDATAPREPARATION_DEVICECLUSTERIZATIONALG_H
#define ACTSGPUDATAPREPARATION_DEVICECLUSTERIZATIONALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "IDeviceClusterizationAlgProviderTool.h"
#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "ActsGPUEvent/TracccSiliconCellCollection.h"
#include "ActsGPUEvent/TracccMeasurementCollection.h"
#include "ActsGPUEvent/TracccSiliconClusterCollection.h"
#include "ActsGPUEvent/TracccDetectorConditionsDescription.h"
#include "ActsGPUEvent/TracccDetectorDesignDescription.h"

#include "vecmem/utils/cuda/copy.hpp"

namespace ActsTrk {

class DeviceClusterizationAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:

   // ---------- configuration ----------
   Gaudi::Property<bool> m_retrieveClusterCells{
      this, "RetrieveClusterCells", true,
      "Whether to retrieve cell indices associated to the clusters."};
  Gaudi::Property<std::string> m_deviceDesignObjectName{
      this, "DeviceDigitizationObjectName", "TracccDeviceDigitizationConfig",
      "Traccc device digitization object"};
  Gaudi::Property<std::string> m_deviceCondObjectName{
      this, "DeviceConditionsObjectName", "TracccDeviceConditionsConfig",
      "Traccc device conditions object"};

  // ---------- tools ----------
  ToolHandle<IDeviceClusterizationAlgProviderTool> m_clusteringAlgProviderTool{
      this, "ClusteringAlgProviderTool", "",
      "Tool providing the appropriate backend device clusterization algorithm"};

  ToolHandle<AthDevice::IMemoryResourceTool> m_deviceMR{
      this, "DeviceMR", "",
      "Device memory resource tool"};
  // for debugging prints only
  ToolHandle<AthDevice::ICopyTool> m_copy{
      this, "CopyProviderTool", "", "Vecmem copy provider tool"};

  // ---------- data handles ----------
  SG::ReadHandleKey<traccc::edm::silicon_cell_collection::const_view> m_inputCellsKey{
      this, "InputTracccCells", "",
      "Input traccc cell collection buffer"};
  SG::WriteHandleKey<traccc::edm::measurement_collection::buffer> m_outputMeasKey{
      this, "OutputTracccMeasurements", "",
      "Output uncalibrated traccc measurement collection buffer"};
  SG::WriteHandleKey<traccc::edm::silicon_cluster_collection::buffer> m_outputClusterKey{
      this, "OutputTracccClusters", "",
      "Output uncalibrated traccc cluster collection buffer"};

  // Device buffers — retrieved from detStore
  const traccc::detector_design_description::const_view* m_deviceDesign;
  const traccc::detector_conditions_description::const_view* m_deviceCond;

};

} // namespace ActsTrk

#endif // ACTSGPUDATAPREPARATION_DEVICECLUSTERIZATIONALG_H
