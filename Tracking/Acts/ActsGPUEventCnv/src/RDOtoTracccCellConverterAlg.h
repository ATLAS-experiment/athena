/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_RDOTOTRACCCCELLCONVERTERALG_H
#define ACTSGPUEVENT_RDOTOTRACCCCELLCONVERTERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "InDetRawData/PixelRDO_Container.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include "ActsGPUEvent/TracccSiliconCellCollection.h"
#include "ActsGPUEvent/TracccDetectorConditionsDescription.h"
#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"

#include "Identifier/Identifier.h"

#include "ActsGPUInterfaces/IDeviceDetectorDescriptionProviderSvc.h"

#include <unordered_map>
#include <cstdint>
#include <atomic>

class PixelID;
class SCT_ID;
namespace InDetDD{
  class PixelDetectorManager;
  class SCT_DetectorManager;
}

namespace ActsTrk {

class RDOtoTracccCellConverterAlg : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual StatusCode finalize() override;


private:
  SG::ReadHandleKey<PixelRDO_Container> m_pixelRDOKey{
      this, "PixelRDO", "ITkPixelRDOs"};
  SG::ReadHandleKey<SCT_RDO_Container> m_stripRDOKey{
      this, "StripRDO", "ITkStripRDOs"};
  SG::WriteHandleKey<traccc::edm::silicon_cell_collection::buffer> m_tracccCellsKey{
      this, "TracccCells", "", "Output traccc cell collection buffer"};

  // object counters
  mutable std::atomic<int> m_nPix = 0;
  mutable std::atomic<int> m_nStrip = 0;
  mutable std::atomic<int> m_nCells = 0;

  const PixelID* m_pixelID{nullptr};
  const SCT_ID*  m_stripID{nullptr};
  const InDetDD::PixelDetectorManager* m_pixelManager{nullptr};
  const InDetDD::SCT_DetectorManager*  m_stripManager{nullptr};

  ToolHandle<AthDevice::IMemoryResourceTool> m_hostMR{
      this, "HostMR", "", "The host memory resource tool to use"};
  ToolHandle<AthDevice::IMemoryResourceTool> m_deviceMR{
      this, "DeviceMR", "", "The device memory resource tool to use"};
  ToolHandle<AthDevice::ICopyTool> m_copy{
      this, "CopyProviderTool", "", "Vecmem copy provider tool"};

  ServiceHandle<ActsTrk::IDeviceDetectorDescriptionProviderSvc> m_detDescSvc{
    this, "DetectorDescriptionSvc", "ActsTrk::JSONDeviceDetectorDescriptionProviderSvc"};
  Gaudi::Property<std::string> m_hostCondObjectName{
      this, "HostConditionsObjectName", "",
      "Traccc host conditions object"};

  const traccc::detector_conditions_description::host* m_hostCond{nullptr};

  // Geometry conversion maps
  const std::unordered_map<Identifier, uint64_t>* m_athenaToDetray{};
  std::unordered_map<uint64_t, unsigned int> m_DetrayIdToDetDescrIndexMap;
};

} // namespace ActsTrk

#endif // ACTSGPUEVENT_RDOTOTRACCCCELLCONVERTERALG_H