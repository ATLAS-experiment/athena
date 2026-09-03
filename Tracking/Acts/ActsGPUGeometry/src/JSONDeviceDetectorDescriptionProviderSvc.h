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
#include "AthDeviceInterfaces/IMemoryResourcesTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"

#include "traccc/geometry/detector_design_description.hpp"
#include "traccc/geometry/detector_conditions_description.hpp"

#include "detray/core/detail/container_views.hpp"
#include "detray/core/detector.hpp"
#include "detray/detectors/itk_metadata.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace ActsTrk {

/**
 * @class JSONDeviceDetectorDescriptionProviderSvc
 *
 * @brief Service providing device detector description from JSON files
 *
 * This service loads detector description data from JSON files,
 * which are needed for executing track reconstruction on GPU.
 * The service provides the following data:
 * - JSON geometry file -> detray geometry (required)
 * - JSON digitization file -> traccc digitization config (required)
 * - JSON conditions file -> traccc conditions config (required)
 * - CSV map file -> athena<->detray ID map (required)
 *
 * To be added in the future (only needed in track reco on device):
 * - JSON material file -> detray material (optional)
 * - JSON surface grid file -> detray surface grid (optional)
 * - CVF magnetic field -> covfie magnetic field (required)
 *
 * All objects are recorded to detector store as pointers to device objects, apart from the athena<->detray ID map.
 * Additionally the digitization and conditions objects are stored in detector store as host objects,
 * which are needed for EDM conversions.
 *
 * @author Neža Ribarič <neza.ribaric@cern.ch>
 */
class JSONDeviceDetectorDescriptionProviderSvc
    : public extends<AthService, ActsTrk::IDeviceDetectorDescriptionProviderSvc>
{
public:

    using extends::extends;

    /// Function initializing and executing the file loading
    virtual StatusCode initialize() override;

    /// @name Athena<->detray ID map accessors for EDM converters
    /// @{
    virtual const std::unordered_map<uint64_t, Identifier>&
        detrayToAthenaMap() const override;
    virtual const std::unordered_map<Identifier, uint64_t>&
        athenaToDetrayMap() const override;
    ///@}

private:

    ServiceHandle<StoreGateSvc> m_detStore{this, "DetectorStore", "StoreGateSvc/DetectorStore"};
    /// @name The host and device memory resources tool to use for memory allocations
    ToolHandle<AthDevice::IMemoryResourcesTool> m_MRs{
        this, "MemoryResourcesTool", "",
        "The memory resources tool to use for allocating memory on the device"};

    /// The copy tool used for copying data to device
    ToolHandle<AthDevice::ICopyTool> m_copy{
        this, "CopyProviderTool", "", "Vecmem copy provider tool"};

    /// @name The input JSON file names, path resolved with PathResolver
    /// @{
    Gaudi::Property<std::string> m_geometryFile{
        this, "GeometryFile", "",
        "Detray geometry JSON file"};
    Gaudi::Property<std::string> m_digitizationFile{
        this, "DigitizationFile", "",
        "Traccc digitization config JSON file"};
    Gaudi::Property<std::string> m_conditionsFile{
        this, "ConditionsFile", "",
        "Traccc conditions config JSON file"};
    Gaudi::Property<std::string> m_mapFile{
        this, "MapFile", "",
        "Path to the athena<->detray ID map CSV file"};
    /// @}

    /// @name The output object names
    /// @{
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
    Gaudi::Property<std::string> m_deviceDetectorName{
        this, "DeviceDetectorName", "",
        "Detray device detector object"};
    Gaudi::Property<std::string> m_hostDetectorName{
        this, "HostDetectorName", "",
        "Detray host detector object"};
    /// @}

    /// Helper function to load Athena<->detray ID maps from csv
    StatusCode loadIdMaps();

    std::unordered_map<uint64_t, Identifier> m_detrayToAthena;
    std::unordered_map<Identifier, uint64_t> m_athenaToDetray;

};

} // namespace ActsTrk

#endif // ACTSGPUDATAPREPARATION_JSONDEVICEDETECTORDESCRIPTIONPROVIDERSVC_H