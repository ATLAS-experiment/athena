/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUPATTERNRECOGNITION_DEVICETRIPLETSEEDINGALG_H
#define ACTSGPUPATTERNRECOGNITION_DEVICETRIPLETSEEDINGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "IDeviceSeedingAlgProviderTool.h"
#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "ActsGPUEvent/TracccDetectorGeometryDescription.h"
#include "ActsGPUEvent/TracccSpacepointCollection.h"
#include "ActsGPUEvent/TracccSeedCollection.h"

template <typename scalar_t>
using unit = detray::unit<scalar_t>;

namespace ActsTrk {
/**
 * @class DeviceSeedingAlg
 *
 * @brief Algorithm executing traccc (pixel) seeding on the GPU.
 *
 * The backend-specific seeding algorithm is provided by
 * a dedicated tool, together with the device memory resource.
 *
 * The algorithm retrieves the device resident traccc spacepoint collection from the event
 * store and the detector geometry from the detector store, passes
 * them to the device seeding algorithm of your choice (triplet/GBTS), and records the
 * resulting device resident traccc seed collection back into the event store.
 *
 *
 * @author Neža Ribarič <neza.ribaric@cern.ch>
 */
class DeviceTripletSeedingAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  /// Function initializing the algorithm
  virtual StatusCode initialize() override;
  /// Function executing the algorithm
  virtual StatusCode execute(const EventContext& ctx) const override;

private:

    virtual StatusCode configureTripletSeeder();

    /// @name The tool that provides backend-specific traccc seeding algorithms
    ToolHandle<IDeviceSeedingAlgProviderTool> m_seedingAlgProviderTool{
        this, "SeedingAlgProviderTool", "",
        "Tool providing the appropriate backend device seeding algorithm"};
    /// @name The device memory resource tool to use for memory allocations
    ToolHandle<AthDevice::IMemoryResourceTool> m_deviceMR{
        this, "DeviceMR", "",
        "Device memory resource tool"};

    /// @name The name of device resident input traccc spacepoint collection
    SG::ReadHandleKey<traccc::edm::spacepoint_collection::const_view> m_inputPixelSPKey{
        this, "InputTracccPixelSpacepoints", "",
        "Input traccc spacepoint collection buffer"};

    /// @name The name of device resident output traccc seed collection
    /// {@
    SG::WriteHandleKey<traccc::edm::seed_collection::buffer> m_outputPixelSeedsKey{
        this, "OutputTracccPixelSeeds", "",
        "Output traccc pixel seed collection buffer"};
    /// @}

    traccc::seedfinder_config m_seedfinder;
    traccc::seedfilter_config m_seedfilter;

};

} // namespace ActsTrk

#endif // ACTSGPUPATTERNRECOGNITION_DEVICETRIPLETSEEDINGALG_H
