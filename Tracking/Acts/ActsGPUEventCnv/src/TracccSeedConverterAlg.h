/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_TRACCCSEEDCONVERTERALG_H
#define ACTSGPUEVENT_TRACCCSEEDCONVERTERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "ActsGPUEvent/TracccSeedCollection.h"
#include "ActsGPUEvent/TracccSpacepointCollection.h"
#include "ActsEvent/SeedContainer.h"

#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"

#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

#include "GaudiKernel/ToolHandle.h"

namespace ActsTrk {

/**
 * @class TracccSeedConverterAlg
 *
 * @brief Algorithm converting traccc seeds (device buffer) to ACTS seeds (host container)
 *
 * This algorithm retrieves the input device resident traccc seed collection from the event store,
 * copies the data to host buffer and converts the traccc seeds to ActsTrk::Seed.
 * It works under the assumption that the relevant traccc spacepoints 
 * have already been converted to xAOD spacepoints and are available on host in the event store
 * and that the indices between the HOST and DEVICE resident containers have been recorded in a index map. 
 *
 * @author Neža Ribarič <neza.ribaric@cern.ch>
 */
class TracccSeedConverterAlg : public AthReentrantAlgorithm
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

  /// @name The input device resident cluster, measurement and cell collection names
  /// {@
  SG::ReadHandleKey<xAOD::SpacePointContainer> m_inputSPKey{
      this, "InputSpacepoints", "TracccSpacepoints",
      "Input traccc SP collection buffer"};
  SG::ReadHandleKey<traccc::edm::spacepoint_collection::buffer> m_inputSPDeviceKey{
      this, "InputSpacepointsDevice", "TracccSpacepointsDevice",
      "Input traccc SP collection device buffer"};
  SG::ReadHandleKey<std::vector<unsigned int>> m_inputMeasToPixelSPKey{
      this, "InputMeasToPixelSP", "TracccMeasToPixelSP",
      "Input mapping from traccc measurement index to pixel spacepoint index"};        
  SG::ReadHandleKey<traccc::edm::seed_collection::buffer> m_inputSeedsKey{
      this, "InputSeeds", "TracccSeeds",
      "Input traccc seed collection buffer"};
  /// @}

  /// @name The output host resident seed container name
  /// {@
  SG::WriteHandleKey<ActsTrk::SeedContainer> m_outputSeedsKey{
      this, "OutputSeeds", "ITkTracccSeeds",
      "Output ACTS seeds container"};
  /// @}

  /// @name The host memory resource tool to use for memory allocations
  ToolHandle<AthDevice::IMemoryResourceTool> m_hostMR{
    this, "HostMR", "", "Host memory resource tool"};
  /// @name The copy tool used for copying data from device
  ToolHandle<AthDevice::ICopyTool> m_copy{
      this, "CopyProviderTool", "", "Vecmem copy provider tool"};

  /// The object counters for debug prints in finalize method
  /// {@
  mutable std::atomic<int> m_nSP = 0;
  mutable std::atomic<int> m_nSeeds = 0;
  /// @}    
  
};

} // namespace ActsTrk

#endif // ACTSGPUEVENT_TRACCCSEEDCONVERTERALG_H