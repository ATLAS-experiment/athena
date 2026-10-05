/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUGEOMETRY_DEVICEDETECTORDESCRIPTIONSVC_H
#define ACTSGPUGEOMETRY_DEVICEDETECTORDESCRIPTIONSVC_H

#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/StoreGateSvc.h"

#include "IDeviceDetectorDescriptionSvc.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"

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
#include "ActsGPUEvent/TracccDetectorDesignDescription.h"
#include "ActsGPUEvent/TracccDetectorGeometryDescription.h"
#include "ActsGPUEvent/GeometryIdMapping.h"

#include "traccc/io/read_detector.hpp"
#include "traccc/geometry/detector_design_description.hpp"

#include "detray/geometry/tracking_surface.hpp"

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

struct moduleInfo {
    bool pixel{false};
    bool isAnnulus{false};
    bool equidistant_binning{false};
    std::vector<traccc::scalar> row_centres;
    std::vector<traccc::scalar> column_centres;
    float module_width{0.f};
    float module_length{0.f};
    int columns{0};
    int rows{0};
    int side{0};
    float lorentz_shift_x = 0.f;
    float lorentz_shift_y = 0.f;
};

struct designKey {
    bool pixel{false};
    bool isAnnulus{false};
    std::vector<traccc::scalar> edgesX;
    std::vector<traccc::scalar> edgesY;
    float width{0.f};
    float length{0.f};

    bool operator<(const designKey& other) const {
    return std::tie(pixel, isAnnulus, edgesX, edgesY, width, length) <
           std::tie(other.pixel, other.isAnnulus, other.edgesX, other.edgesY, other.width,
                        other.length);
    }
};

namespace ActsTrk {

/**
 * @class DeviceDetectorDescriptionSvc
 *
 * @brief Service providing the static device detector description
 *
 * This service populates the detector description data which does not
 * change during the run and is needed for executing track reconstruction on GPU:
 * traccc digitization config: module segmnentation info used in clustering
 * detray detector: the tracking geometry on the device
 * athena<->acts<->detray ID map
 *
 * All objects are recorded to detector store as pointers to device objects, apart from the athena<->detray ID map.
 * Additionally the digitization object is stored in detector store as host object,
 * which is needed for EDM conversions.
 *
 * Being a service, it is initialized before any algorithm in both serial and
 * multi-threaded jobs, so that the objects can be retrieved from the detector
 * store during the initialization of the algorithms. The per-module conditions
 * (eg. lorentz shift) are written by DeviceDetectorDescriptionCondAlg.
 *
 * @author Neža Ribarič <neza.ribaric@cern.ch>
 */
class DeviceDetectorDescriptionSvc
    : public extends<AthService, IDeviceDetectorDescriptionSvc>
{
public:

    using extends::extends;

    /// Function initializing and executing the geometry conversion
    virtual StatusCode initialize() override;

    /// Static per-module entries, indexed like the detector conditions description
    virtual const std::vector<StaticCondEntry>& staticCondEntries() const override {
        return m_staticCondEntries;
    }

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

    Gaudi::Property<std::string> m_deviceDetectorName{
        this, "DeviceDetectorName", "",
        "StoreGate name for the device (buffer) detector object"};

    /// @name The output object names
    /// @{
    Gaudi::Property<std::string> m_deviceDesignObjectName{
        this, "DeviceDigitizationObjectName", "",
        "Traccc device digitization object"};
    Gaudi::Property<std::string> m_hostDesignObjectName{
        this, "HostDigitizationObjectName", "",
        "Traccc host digitization object"};
    /// @}

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

    std::shared_ptr<const Acts::TrackingGeometry> m_trackingGeometry;
    std::map<Identifier, moduleInfo> m_atlasModuleInfo;

    ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};

};

} // namespace ActsTrk

#endif // ACTSGPUGEOMETRY_DEVICEDETECTORDESCRIPTIONSVC_H
