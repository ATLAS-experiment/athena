/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_TRACCCSEEDDEVICECONVERTERALG_H
#define ACTSGPUEVENT_TRACCCSEEDDEVICECONVERTERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "GaudiKernel/ToolHandle.h"

#include "ActsGPUEvent/TracccSeedCollection.h"
#include "ActsGPUEvent/TracccSpacepointCollection.h"
#include "ActsEvent/SeedContainer.h"

#include "AthCUDAInterfaces/IStreamTool.h"
#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthDeviceInterfaces/IMemoryResourcesTool.h"
#include "AthDeviceInterfaces/ICopiesTool.h"

#include "xAODInDetMeasurement/SpacePointContainer.h"

#include "AthenaKernel/Chrono.h"

// Device-side interface: size_type, DeviceSeedContainer, convertSeedsOnGPU
#include "src/DeviceSeedContainers.h"

// FOr map loading of traccc msmt index to pixel index
#include "ActsGPUEvent/TracccMeasurementCollection.h"
#include "ActsGPUEvent/TracccMeasToPixelIndex.h"
#include "src/cuda/MeasToPixelIndex.h"

#include <atomic>
#include <cassert>

namespace ActsTrk {

/**
 * @class TracccSeedDeviceConverterAlg
 *
 * @brief Converts traccc seeds (device buffer) to ActsTrk seeds (host
 * container), doing the index lookups on device.
 *
 * Device: one thread per seed resolves its 3 device spacepoints to host xAOD
 * pixel spacepoint indices and copies the quality. Host: one bulk copy, then
 * one loop building the SeedContainer.
 *
 */
class TracccSeedDeviceConverterAlg : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual StatusCode finalize() override;

private:
  /// @name The input host and device resident collection names
  /// {@
  SG::ReadHandleKey<xAOD::SpacePointContainer> m_inputSPKey{
      this, "InputSpacepoints", "TracccSpacepoints",
      "Input host xAOD pixel spacepoint container"};
  SG::ReadHandleKey<traccc::edm::spacepoint_collection::buffer> m_inputSPDeviceKey{
      this, "InputSpacepointsDevice", "TracccSpacepointsDevice",
      "Input traccc SP collection device buffer"};
  SG::ReadHandleKey<ActsTrk::TracccMeasToPixelIndex> m_inputMeasToPixelKey{
      this, "InputMeasToPixelIndex", "",
      "Optional: device map traccc measurement -> host pixel SP index. "
      "Empty: built here from InputMeasurements"};
  SG::ReadHandleKey<traccc::edm::measurement_collection::buffer> m_inputMeasKey{
      this, "InputMeasurements", "TracccMeasurements",
      "Device traccc measurements (only used if InputMeasToPixelIndex is empty)"};
  SG::ReadHandleKey<traccc::edm::seed_collection::buffer> m_inputSeedsKey{
      this, "InputSeeds", "TracccSeeds",
      "Input traccc seed collection buffer"};
  /// @}

  ServiceHandle<IChronoSvc> m_chronoSvc{this, "ChronoStatSvc", "ChronoStatSvc"};

  /// @name The output host resident seed container name
  SG::WriteHandleKey<ActsTrk::SeedContainer> m_outputSeedsKey{
      this, "OutputSeeds", "ITkTracccSeedsFromDevice",
      "Output ACTS seeds container"};

  /// @name Tools
  /// {@
  ToolHandle<AthDevice::IMemoryResourceTool> m_hostMR{
    this, "HostMR", "", "Host (pinned) memory resource for the bulk copy"};
  ToolHandle<AthDevice::IMemoryResourcesTool> m_mrTool{
    this, "MemoryResourcesTool", "", "Tool that provides memory resources"};
  ToolHandle<AthDevice::ICopiesTool> m_copiesTool{
    this, "CopiesProviderTool", "", "Tool that provides vecmem copy objects"};
  ToolHandle<AthCUDA::IStreamTool> m_streamTool{
    this, "StreamTool", "", "Tool that provides CUDA streams"};
  /// @}

  /// The object counters for debug prints in finalize method
  /// {@
  mutable std::atomic<std::size_t> m_nSP = 0;
  mutable std::atomic<std::size_t> m_nSeeds = 0;
  /// @}
};

} // namespace ActsTrk

#endif // ACTSGPUEVENT_TRACCCSEEDDEVICECONVERTERALG_H