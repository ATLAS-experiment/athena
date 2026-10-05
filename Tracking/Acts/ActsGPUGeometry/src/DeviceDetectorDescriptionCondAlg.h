/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUGEOMETRY_DEVICEDETECTORDESCRIPTIONCONDALG_H
#define ACTSGPUGEOMETRY_DEVICEDETECTORDESCRIPTIONCONDALG_H

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

#include "IDeviceDetectorDescriptionSvc.h"
#include "InDetCondTools/ISiLorentzAngleTool.h"

#include "AthDeviceInterfaces/IMemoryResourcesTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"

#include "ActsGPUEvent/TracccDetectorConditionsDescription.h"

#include "traccc/geometry/detector_conditions_description.hpp"

#include <string>
#include <vector>

namespace ActsTrk {

/**
 * @class DeviceDetectorDescriptionCondAlg
 *
 * @brief Conditions algorithm providing the device detector conditions description
 *
 * This algorithm populates the traccc conditions config: per-module information
 * (eg. identifier maps, lorentz shift, backside module index for strips),
 * which is needed for executing track reconstruction on GPU.
 *
 * The static part of the detector description (digitization config, detray
 * detector, athena<->detray ID map) is provided by DeviceDetectorDescriptionSvc.
 *
 * The conditions object is recorded to the conditions store both as a device
 * object and as a host object, which is needed for EDM conversions.
 *
 * @author Neža Ribarič <neza.ribaric@cern.ch>
 */
class DeviceDetectorDescriptionCondAlg : public AthCondAlgorithm
{
public:

    using AthCondAlgorithm::AthCondAlgorithm;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

private:

    /// The service providing the static part of the detector description
    ServiceHandle<ActsTrk::IDeviceDetectorDescriptionSvc> m_detDescSvc{
        this, "DeviceDetectorDescriptionSvc", "",
        "Service building the static device detector description"};

    /// @name The host and device memory resources tools to use for memory allocations
    /// @{
    ToolHandle<AthDevice::IMemoryResourcesTool> m_MRs{
        this, "MemoryResourcesTool", "",
        "The memory resources tool to use for allocating memory on the device"};
    /// @}

    /// The copy tool used for copying data to device
    ToolHandle<AthDevice::ICopyTool> m_copy{
        this, "CopyProviderTool", "", "Vecmem copy provider tool"};

    SG::WriteCondHandleKey<traccc::detector_conditions_description::host> m_writeHostCondKey{
        this, "HostConditionsObjectName", "",
        "Key for writing the per-IOV traccc host conditions object"};

    SG::WriteCondHandleKey<traccc::detector_conditions_description::buffer> m_writeDeviceCondKey{
        this, "DeviceConditionsObjectName", "",
        "Key for writing the per-IOV traccc device conditions object"};

    /// Conversion helpers (to retrieve module design, hash, etc.)
    /// {@
    const PixelID* m_pixelID{nullptr};
    const SCT_ID*  m_stripID{nullptr};
    Gaudi::Property<std::string> m_pixelIdHelperName {this, "PixelIDHelperName", "PixelID",
        "Pixel-like ID helper name to retrieve from DetectorStore"};
    Gaudi::Property<std::string> m_stripIdHelperName{this, "StripIDHelperName", "SCT_ID",
        "Strip-like ID helper name to retrieve from DetectorStore"};

    /// @}

    ToolHandle<ISiLorentzAngleTool> m_stripLorentzAngleTool{
        this, "StripLorentzAngleTool", "SiLorentzAngleTool",
        "Tool to retrieve Lorentz angle"};
    ToolHandle<ISiLorentzAngleTool> m_pixelLorentzAngleTool{
        this, "PixelLorentzAngleTool", "",
        "Tool to retreive Lorentz angle of Pixel"};

};

} // namespace ActsTrk

#endif // ACTSGPUGEOMETRY_DEVICEDETECTORDESCRIPTIONCONDALG_H
