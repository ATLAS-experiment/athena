/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUDATAPREPARATION_DEVICEDETECTORDESCRIPTIONSVC_H
#define ACTSGPUDATAPREPARATION_DEVICEDETECTORDESCRIPTIONSVC_H

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "GaudiKernel/ToolHandle.h"
#include "PathResolver/PathResolver.h"

#include "ActsGPUInterfaces/IDeviceDetectorDescriptionProviderSvc.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "InDetCondTools/ISiLorentzAngleTool.h"

#include "AthDeviceInterfaces/IMemoryResourcesTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"
#include "InDetReadoutGeometry/SiDetectorDesign.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "SCT_ReadoutGeometry/SCT_BarrelModuleSideDesign.h"
#include "SCT_ReadoutGeometry/SCT_ForwardModuleSideDesign.h"
#include "SCT_ReadoutGeometry/StripStereoAnnulusDesign.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetReadoutGeometry/SiDetectorManager.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetIdentifier/SCT_ID.h"

#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGPUEvent/TracccDetectorConditionsDescription.h"
#include "ActsGPUEvent/TracccDetectorDesignDescription.h"
#include "ActsGPUEvent/TracccDetectorGeometryDescription.h"
#include "ActsGPUEvent/GeometryIdMapping.h"

#include "traccc/io/read_detector.hpp"
#include "traccc/geometry/detector_design_description.hpp"
#include "traccc/geometry/detector_conditions_description.hpp"

#include "detray/geometry/tracking_surface.hpp"

#include "vecmem/utils/cuda/copy.hpp"

#include <memory>
#include <string>
#include <unordered_map>

struct moduleInfo {
    bool pixel;
    bool isAnnulus;
    bool equidistant_binning;
    std::vector<traccc::scalar> row_centres;
    std::vector<traccc::scalar> column_centres;
    float module_width;
    float module_length;
    int columns;
    int rows;
    int side;
    float lorentz_shift_x = 0.f;
    float lorentz_shift_y = 0.f;
};

struct designKey {
    bool pixel;
    bool isAnnulus;
    std::vector<traccc::scalar> edgesX;
    std::vector<traccc::scalar> edgesY;
    float width;
    float length;

    bool operator<(const designKey& other) const {
    return std::tie(pixel, isAnnulus, edgesX, edgesY, width, length) <
           std::tie(other.pixel, other.isAnnulus, other.edgesX, other.edgesY, other.width,
                        other.length);
    }
};

struct StaticCondEntry {
    unsigned int designId = 0;
    detray::geometry::identifier detrayGeometryId{};
    Acts::GeometryIdentifier::Value actsGeometryId = 0;
    Identifier athenaId;       // only meaningful if hasAthenaModule
    bool hasAthenaModule = false;
};


namespace ActsTrk {

/**
 * @class DeviceDetectorDescriptionProviderSvc
 *
 * @brief Service providing device detector description
 *
 * This service populates detector description data,
 * which is needed for executing track reconstruction on GPU.
 * The service provides the following data:
 * traccc digitization config: module segmnentation info used in clustering
 * traccc conditions config: per-module information (eg. identifier maps, lorentz shift, backside module index for strips)
 *
 *
 * All objects are recorded to detector store as pointers to device objects, apart from the athena<->detray ID map.
 * Additionally the digitization and conditions objects are stored in detector store as host objects,
 * which are needed for EDM conversions.
 *
 * @author Neža Ribarič <neza.ribaric@cern.ch>
 */
class DeviceDetectorDescriptionCondAlg : public AthCondAlgorithm
{
public:

    using AthCondAlgorithm::AthCondAlgorithm;

    /// Function initializing and executing the file loading
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

private:

    ServiceHandle<StoreGateSvc> m_detStore{this, "DetectorStore", "StoreGateSvc/DetectorStore"};

    /// @name The host and device memory resources tools to use for memory allocations
    /// @{
    ToolHandle<AthDevice::IMemoryResourcesTool> m_MRs{
        this, "MemoryResourcesTool", "",
        "The memory resources tool to use for allocating memory on the device"};
    /// @}

    /// The copy tool used for copying data to device
    ToolHandle<AthDevice::ICopyTool> m_copy{
        this, "CopyProviderTool", "", "Vecmem copy provider tool"};

    Gaudi::Property<std::string> m_hostDetectorName{
        this, "HostDetectorName", "",
        "Detray host detector object"};    

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
    /// @}

    SG::WriteCondHandleKey<traccc::detector_conditions_description::host> m_writeHostCondKey{
        this, "HostConditionsObjectName", "",
        "Key for writing the per-IOV traccc host conditions object"};

    SG::WriteCondHandleKey<traccc::detector_conditions_description::buffer> m_writeDeviceCondKey{
        this, "DeviceConditionsObjectName", "",
        "Key for writing the per-IOV traccc device conditions object"};

    Gaudi::Property<std::string> m_geoIdMappingObjectName{
        this, "GeoIdMappingObjectName", "",
        "StoreGate name for the detray/acts/athena geo id mapping"};

    /// Conversion helpers (to retrieve module design, hash, etc.)
    /// {@
    const PixelID* m_pixelID{nullptr};
    const SCT_ID*  m_stripID{nullptr};
    Gaudi::Property<std::string> m_pixelIdHelperName {this, "PixelIDHelperName", "PixelID",
        "Pixel-like ID helper name to retrieve from DetectorStore"};
    Gaudi::Property<std::string> m_stripIdHelperName{this, "StripIDHelperName", "SCT_ID",
        "Strip-like ID helper name to retrieve from DetectorStore"};

    const InDetDD::PixelDetectorManager* m_pixelManager{nullptr};
    const InDetDD::SCT_DetectorManager*  m_stripManager{nullptr};
    
    /// @}

    std::vector<StaticCondEntry> m_staticCondEntries;
    std::vector<std::size_t> m_designSizes;  // for device buffer construction, reused every execute()
    // Athena Identifier -> row index in the conditions table (stable across
    // events, since row order mirrors itkDetector.surfaces()). Currently
    // unused outside initialize() but kept for the future strip
    // neighbour/backside_id fill (see commented-out block in execute()).
    std::unordered_map<Identifier, std::size_t> m_athenaToCondIndex;
        
    std::shared_ptr<const Acts::TrackingGeometry> m_trackingGeometry;
    std::vector<std::shared_ptr<Acts::Surface>> m_allsurfaces;
    std::vector<std::shared_ptr<Acts::Surface>> m_surfaces;

    std::map<Acts::GeometryIdentifier, Identifier> m_actsToAthena;
    std::unordered_map<uint64_t, Identifier> m_detrayToAthenaMap;
    std::unordered_map<Identifier, uint64_t> m_athenaToDetrayMap;

    std::map<Identifier, moduleInfo> m_atlasModuleInfo;

    ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};
    ToolHandle<ISiLorentzAngleTool> m_stripLorentzAngleTool{
        this, "StripLorentzAngleTool", "SiLorentzAngleTool",
        "Tool to retrieve Lorentz angle"};
    ToolHandle<ISiLorentzAngleTool> m_pixelLorentzAngleTool{
        this, "PixelLorentzAngleTool", "",
        "Tool to retreive Lorentz angle of Pixel"};



};

} // namespace ActsTrk

#endif // ACTSGPUDATAPREPARATION_DEVICEDETECTORDESCRIPTIONSVC_H