/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUDATAPREPARATION_DEVICESPFORMATIONALG_H
#define ACTSGPUDATAPREPARATION_DEVICESPFORMATIONALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "IDeviceSPFormationAlgProviderTool.h"
#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "ActsGPUEvent/TracccDetectorGeometryDescription.h"
#include "ActsGPUEvent/TracccMeasurementCollection.h"
#include "ActsGPUEvent/TracccSpacepointCollection.h"



#include "vecmem/utils/cuda/copy.hpp"

namespace ActsTrk {
/**
 * @class DeviceSPFormationAlg
 *
 * @brief Algorithm executing traccc (pixel) spacepoint formation on the GPU.
 *
 * The backend-specific spacepoint formation algorithm is provided by
 * a dedicated tool, together with the device memory resource.
 *
 * The algorithm retrieves the device resident traccc measurement collection from the event
 * store and the detector geometry from the detector store, passes
 * them to the device spacepoint formation algorithm, and records the
 * resulting device resident traccc spacepoint collection back into the event store.
 *
 *
 * @author Neža Ribarič <neza.ribaric@cern.ch>
 */
class DeviceSPFormationAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  /// Function initializing the algorithm
  virtual StatusCode initialize() override;
  /// Function executing the algorithm
  virtual StatusCode execute(const EventContext& ctx) const override;

private:

    /// @name Input device resident traccc detector geometry
    /// {@
    Gaudi::Property<std::string> m_deviceDetectorName{
        this, "DeviceDetectorName", "DetrayDeviceDetector",
        "Detray device detector object"};
    /// @}

    /// @name The tool that provides backend-specific traccc clusterization algorithms
    ToolHandle<IDeviceSPFormationAlgProviderTool> m_spAlgProviderTool{
        this, "SPFormationAlgProviderTool", "",
        "Tool providing the appropriate backend device spacepoint formation algorithm"};
    /// @name The device memory resource tool to use for memory allocations
    ToolHandle<AthDevice::IMemoryResourceTool> m_deviceMR{
        this, "DeviceMR", "",
        "Device memory resource tool"};

    /// @name The name of device resident input traccc measurement collection
    SG::ReadHandleKey<traccc::edm::measurement_collection::const_view> m_inputMeasKey{
        this, "InputTracccMeasurements", "",
        "Input traccc measurement collection buffer"};

    /// @name The name of device resident output traccc spacepoint collection
    /// {@
    SG::WriteHandleKey<traccc::edm::spacepoint_collection::buffer> m_outputPixelSPKey{
        this, "OutputTracccPixelSpacepoints", "",
        "Output traccc pixel spacepoint collection buffer"};
    /// @}

    // Device buffers — retrieved from detStore
    const traccc::detector_buffer* m_deviceDetector{nullptr};

};

} // namespace ActsTrk

#endif // ACTSGPUDATAPREPARATION_DEVICESPFORMATIONALG_H
