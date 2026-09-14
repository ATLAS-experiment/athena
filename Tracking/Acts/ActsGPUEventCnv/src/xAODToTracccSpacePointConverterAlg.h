/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENTCNV_XAODTOTRACCCSPACEPOINTCONVERTERALG_H
#define ACTSGPUEVENTCNV_XAODTOTRACCCSPACEPOINTCONVERTERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"
#include "GaudiKernel/ToolHandle.h"

#include "ActsGPUEvent/TracccSpacepointCollection.h"

#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthDeviceInterfaces/ICopiesTool.h"

#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "SGTools/StlVectorClids.h"

#include <atomic>
#include <vector>

namespace ActsTrk {

/**
 * @class xAODToTracccSpacePointConverterAlg
 *
 * @brief Algorithm converting xAOD space points (host containers) to traccc spacepoints (device buffer)
 *
 * This algorithm retrieves the input xAOD space point containers from the event store,
 * converts them to a single traccc spacepoint collection, copies the collection to the device
 * and records the resulting device resident buffer in the event store.
 * It works under the assumption that the traccc measurements of the space point clusters
 * are available on the device and that the mapping from the traccc measurement index to the
 * xAOD cluster index has been recorded in the event store, either by TracccMeasurementConverterAlg
 * or by xAODToTracccMeasurementConverterAlg.
 *
 * All input space points must be of the same type (pixel or strip), matching the measurement
 * to cluster index map, so that the output collection can be passed exclusively
 * to the corresponding seeding algorithm.
 * The mapping from the traccc spacepoint index to the input container and the index
 * within that container is recorded in the event store.
 *
 * @author Jackson Burzynski <jackson.carl.burzynski@cern.ch>
 */
class xAODToTracccSpacePointConverterAlg : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  /// Function initializing the algorithm
  virtual StatusCode initialize() override;
  /// Function executing the algorithm
  virtual StatusCode execute(const EventContext& ctx) const override;
  /// Function finalizing the algorithm
  virtual StatusCode finalize() override;

private:
  /// @name The input host resident space point containers and index maps
  /// {@
  SG::ReadHandleKeyArray<xAOD::SpacePointContainer> m_inputSpacePointKeys{
      this, "InputSpacePoints", {}, "Input xAOD space point containers"};
  SG::ReadHandleKey<std::vector<unsigned int>> m_inputMeasToClusterKey{
      this, "InputMeasToCluster", "",
      "Input mapping from traccc measurement index to xAOD cluster index"};
  /// @}

  /// @name The output device resident spacepoint collection name
  /// {@
  SG::WriteHandleKey<traccc::edm::spacepoint_collection::buffer> m_outputSPKey{
      this, "OutputTracccSpacepoints", "",
      "Output traccc spacepoint collection buffer"};
  /// @}

  /// @name The output host resident index map names
  /// {@
  SG::WriteHandleKey<std::vector<unsigned int>> m_outputSPContainerKey{
      this, "OutputSPToHostContainer", "",
      "Output mapping from traccc spacepoint index to input container number"};
  SG::WriteHandleKey<std::vector<unsigned int>> m_outputSPIndexKey{
      this, "OutputSPToHostIndex", "",
      "Output mapping from traccc spacepoint index to index in the input container"};
  /// @}

  /// @name The memory resource and copy tools
  /// {@
  ToolHandle<AthDevice::IMemoryResourceTool> m_hostMR{
      this, "HostMR", "", "The host memory resource tool to use"};
  ToolHandle<AthDevice::IMemoryResourceTool> m_deviceMR{
      this, "DeviceMR", "", "The device memory resource tool to use"};
  ToolHandle<AthDevice::ICopiesTool> m_copiesTool{
      this, "CopiesTool", "", "Tool that provides host and device copy objects"};
  /// @}

  /// The object counter for debug prints in finalize method
  mutable std::atomic<unsigned long> m_nSpacePoints = 0;
};

} // namespace ActsTrk

#endif // ACTSGPUEVENTCNV_XAODTOTRACCCSPACEPOINTCONVERTERALG_H
