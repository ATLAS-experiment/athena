/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENTCNV_TRACCCTRKPARAMDEVICECONVERTERALG_H
#define ACTSGPUEVENTCNV_TRACCCTRKPARAMDEVICECONVERTERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "AthCUDAInterfaces/IStreamTool.h"
#include "AthDeviceInterfaces/IMemoryResourcesTool.h"
#include "AthDeviceInterfaces/ICopiesTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "ActsGPUEvent/TracccTrkParam.h"
#include "ActsGPUEvent/TracccSeedCollection.h"
#include "ActsGPUEvent/TracccSpacepointCollection.h"
#include "ActsGPUEvent/TracccTrkParamCollection.h"

#include <traccc/edm/seed_collection.hpp>
#include <traccc/edm/spacepoint_collection.hpp>
#include <traccc/edm/track_parameters.hpp>

#include "TracccTrkParamDeviceConverterKernel.h"

#include <atomic>

namespace ActsTrk {

/**
 * @class TracccTrkParamDeviceConverterAlg
 *
 * @brief Algorithm packing, on device, what the host CKF needs to know about each traccc seed
 *
 * For each traccc seed, one kernel packs the track parameters estimated on
 * device (without the time covariance) and gathers the measurement index of
 * each seed space point into one TracccTrkParam record. Only pixel seeds are
 * supported for now: one measurement per space point is kept. The buffer is
 * index-aligned with the seed collection, and is all the host needs to make
 * the CKF seeds, in a single copy. The traccc space points and seeds ARE NOT
 * copied to the host, and the traccc bound_track_parameters are copied only
 * in the reduced form of TracccTrkParam: the 6x6 covariance is reduced to the
 * 15 independent entries of the loc0..q/p block and the time variance (the
 * initial traccc covariance has no correlations with the time).
 */
class TracccTrkParamDeviceConverterAlg : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  /// Function initializing the algorithm
  virtual StatusCode initialize() override;
  /// Function executing the algorithm
  virtual StatusCode execute(const EventContext& ctx) const override;
  /// Function finalizing the algorthm
  virtual StatusCode finalize() override;

private:

  /// @name The input device resident track parameters, space points and seeds
  /// {@
  SG::ReadHandleKey<traccc::bound_track_parameters_collection_types::buffer> m_inputTrackParamsKey{
      this, "InputTrackParameters", "TracccTrkParam",
      "Input traccc track parameters buffer, one entry per seed"};
  SG::ReadHandleKey<traccc::edm::spacepoint_collection::buffer> m_inputSPKey{
      this, "InputSpacepoints", "TracccSpacepoints",
      "Input traccc space point collection buffer"};
  SG::ReadHandleKey<traccc::edm::seed_collection::buffer> m_inputSeedsKey{
      this, "InputSeeds", "TracccSeeds",
      "Input traccc seed collection buffer"};
  /// @}

  /// @name The output device resident packed track parameters (TracccTrkParam)
  SG::WriteHandleKey<TracccTrkParamBuffer> m_outputPackedTrkParamsKey{
      this, "OutputPackedTrkParams", "TracccPackedTrkParams",
      "Output buffer with the packed track parameters and measurement indices of each seed"};

  /// @name The memory resource tool to use for memory allocations
  ToolHandle<AthDevice::IMemoryResourcesTool> m_mrTool{
    this, "MemoryResourcesTool", "", "Tool that provides memory resources"};
  /// @name The copy tool used for setting up the device buffer
  ToolHandle<AthDevice::ICopiesTool> m_copiesTool{
    this, "CopiesProviderTool", "", "Tool that provides vecmem copy objects"};
  /// @name Tool that provides CUDA streams
  ToolHandle<AthCUDA::IStreamTool> m_streamTool{
    this, "StreamTool", "", "Tool that provides CUDA streams"};

  /// The object counters for debug prints in finalize method
  mutable std::atomic<int> m_nSeeds = 0;
};

} // namespace ActsTrk

#endif // ACTSGPUEVENTCNV_TRACCCTRKPARAMDEVICECONVERTERALG_H
