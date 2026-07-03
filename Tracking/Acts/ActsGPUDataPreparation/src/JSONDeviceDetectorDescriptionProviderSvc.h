/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUDATAPREPARATION_JSONDEVICEDETECTORDESCRIPTIONPROVIDERSVC_H
#define ACTSGPUDATAPREPARATION_JSONDEVICEDETECTORDESCRIPTIONPROVIDERSVC_H

#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/ToolHandle.h"
#include "PathResolver/PathResolver.h"
#include "ActsGPUInterfaces/IDeviceDetectorDescriptionProviderSvc.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"

#include "traccc/geometry/detector_design_description.hpp"
#include "traccc/geometry/detector_conditions_description.hpp"

#include "vecmem/utils/cuda/copy.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace ActsTrk {

class JSONDeviceDetectorDescriptionProviderSvc
    : public extends<AthService, ActsTrk::IDeviceDetectorDescriptionProviderSvc>
{
public:

  using extends::extends;

    virtual StatusCode initialize() override;

    virtual const std::unordered_map<uint64_t, Identifier>&
        detrayToAthenaMap() const override;
    virtual const std::unordered_map<Identifier, uint64_t>&
        athenaToDetrayMap() const override;

private:

  ServiceHandle<StoreGateSvc> m_detStore{this, "DetectorStore", "StoreGateSvc/DetectorStore"};

  ToolHandle<AthDevice::IMemoryResourceTool> m_hostMR{
      this, "HostMR", "", "Host memory resource tool"};
  ToolHandle<AthDevice::IMemoryResourceTool> m_deviceMR{
      this, "DeviceMR", "", "Device memory resource tool"};
  ToolHandle<AthDevice::ICopyTool> m_copy{
      this, "CopyProviderTool", "", "Vecmem copy provider tool"};

  // Read from files (temporary solution)
  Gaudi::Property<std::string> m_geometryFile{
      this, "GeometryFile", "",
      "Detray geometry JSON file (required when PopulateFromFile=true)"};
  Gaudi::Property<std::string> m_digitizationFile{
      this, "DigitizationFile", "",
      "Traccc digitization config JSON file (required when PopulateFromFile=true)"};
  Gaudi::Property<std::string> m_conditionsFile{
      this, "ConditionsFile", "",
      "Traccc conditions config JSON file (required when PopulateFromFile=true)"};
  Gaudi::Property<std::string> m_mapFile{
    this, "MapFile", "",
    "Path to the athena<->detray ID map CSV file"};

  Gaudi::Property<std::string> m_deviceDesignObjectName{
      this, "DeviceDigitizationObjectName", "",
      "Traccc device digitization object"};
  Gaudi::Property<std::string> m_deviceCondObjectName{
      this, "DeviceConditionsObjectName", "",
      "Traccc device conditions object"};
  Gaudi::Property<std::string> m_hostDesignObjectName{
      this, "HostDigitizationObjectName", "",
      "Traccc host digitization object"};
  Gaudi::Property<std::string> m_hostCondObjectName{
      this, "HostConditionsObjectName", "",
      "Traccc host conditions object"};

  StatusCode loadIdMaps();
  StatusCode buildFromFile(std::pmr::memory_resource& hostMR,
                            std::pmr::memory_resource& deviceMR,
                            const vecmem::copy& copy,
                            std::unique_ptr<traccc::detector_design_description::host>& hostDesign,
                            std::unique_ptr<traccc::detector_conditions_description::host>& hostCond,
                            std::unique_ptr<traccc::detector_design_description::buffer>& deviceDesign,
                            std::unique_ptr<traccc::detector_conditions_description::buffer>& deviceCond);

  std::unordered_map<uint64_t, Identifier> m_detrayToAthena;
  std::unordered_map<Identifier, uint64_t> m_athenaToDetray;

};

} // namespace ActsTrk

#endif // ACTSGPUDATAPREPARATION_JSONDEVICEDETECTORDESCRIPTIONPROVIDERSVC_H