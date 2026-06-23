/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_TRACCCMEASUREMENTCONVERTERALG_H
#define ACTSGPUEVENT_TRACCCMEASUREMENTCONVERTERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "ActsGPUEvent/TracccMeasurementCollection.h"
#include "traccc/edm/measurement_collection.hpp"
#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"

// vecmem
#include "vecmem/memory/memory_resource.hpp"
#include "vecmem/utils/copy.hpp"
#include "vecmem/utils/cuda/async_copy.hpp"
#include "traccc/cuda/utils/stream.hpp"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"

#include "ActsGPUInterfaces/IActsDeviceDetectorDescriptionProviderSvc.h"

#include "GaudiKernel/ToolHandle.h"

namespace ActsTrk {

class TracccMeasurementConverterAlg : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual StatusCode finalize() override;

private:
  // ---- Input ----
  SG::ReadHandleKey<traccc::edm::measurement_collection::buffer> m_inputMeasKey{
      this, "InputMeasurements", "TracccMeasurements",
      "Input traccc measurement collection buffer"};

  // ---- Output ----
  SG::WriteHandleKey<xAOD::PixelClusterContainer> m_outputPixelKey{
      this, "OutputPixelClusters", "ITkTracccPixelClusters",
      "Output xAOD pixel cluster container"};

  SG::WriteHandleKey<xAOD::StripClusterContainer> m_outputStripKey{
      this, "OutputStripClusters", "ITkTracccStripClusters",
      "Output xAOD strip cluster container"};

  // ---- Tool ----
  ToolHandle<AthDevice::IMemoryResourceTool> m_hostMR{
    this, "HostMR", "", "Host memory resource tool"};
  ToolHandle<AthDevice::ICopyTool> m_copy{
      this, "CopyProviderTool", "AthCUDA::CopyTool/CopyProviderTool", "Vecmem copy provider tool"};
  ServiceHandle<ActsTrk::IActsDeviceDetectorDescriptionProviderSvc> m_detDescSvc{
    this, "DetectorDescriptionSvc", "ActsDeviceDetectorDescriptionProviderSvc"};

  // ---- object counters ----
  mutable std::atomic<int> m_nPix = 0;
  mutable std::atomic<int> m_nStrip = 0;
  mutable std::atomic<int> m_nMeas = 0;


  const std::unordered_map<uint64_t, Identifier>* m_detrayToAthena;

  const PixelID* m_pixelID{nullptr};
  const SCT_ID*  m_stripID{nullptr};
  const InDetDD::PixelDetectorManager* m_pixelManager{nullptr};
  const InDetDD::SCT_DetectorManager*  m_stripManager{nullptr};
};

} // namespace ActsTrk

#endif // ACTSGPUEVENT_TRACCCMEASUREMENTCONVERTERALG_H