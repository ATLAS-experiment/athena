/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUDATAPREPARATION_DEVICECLUSTERIZATIONALG_H
#define ACTSGPUDATAPREPARATION_DEVICECLUSTERIZATIONALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "IDeviceClusterizationAlgProviderTool.h"
#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "ActsGPUEvent/TracccSiliconCellCollection.h"
#include "ActsGPUEvent/TracccMeasurementCollection.h"
#include "ActsGPUEvent/TracccSiliconClusterCollection.h"
#include "ActsGPUEvent/TracccDetectorConditionsDescription.h"
#include "ActsGPUEvent/TracccDetectorDesignDescription.h"

namespace ActsTrk {
/**
 * @class DeviceClusterizationAlg
 *
 * @brief Algorithm executing traccc clusterization and measurement
 *        sorting on the GPU.
 *
 * The backend-specific clusterization and sorting algorithms are provided by
 * dedicated tools, together with the device memory resource and the
 * vecmem copy object (only needed for debugging).
 *
 * The algorithm retrieves the device resident traccc cell collection from the event
 * store and the detector description from the detector store, passes
 * them to the device clusterization algorithm, and records the
 * resulting device resident traccc measurement collection back into the event store.
 *
 * @param m_retrieveClusterCells perform disjoint clusterization
 *
 * If required (e.g. for truth matching), it instead runs the
 * so-called disjoint clusterization, which returns the measurements
 * together with a cluster collection holding the association between
 * cell indices and the produced measurements.
 *
 * @author Neža Ribarič <neza.ribaric@cern.ch>
 */
class DeviceClusterizationAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  /// Function initializing the algorithm
  virtual StatusCode initialize() override;
  /// Function executing the algorithm
  virtual StatusCode execute(const EventContext& ctx) const override;

private:

    Gaudi::Property<bool> m_retrieveClusterCells{
        this, "RetrieveClusterCells", true,
        "Whether to retrieve cell indices associated to the clusters."};
    /// @name Input device resident traccc detector design and condition descriptions
    /// {@
    Gaudi::Property<std::string> m_deviceDesignObjectName{
        this, "DeviceDigitizationObjectName", "TracccDeviceDigitizationConfig",
        "Traccc device digitization object"};
    /// @}

    /// @name The tool that provides backend-specific traccc clusterization algorithms
    ToolHandle<IDeviceClusterizationAlgProviderTool> m_clusteringAlgProviderTool{
        this, "ClusteringAlgProviderTool", "",
        "Tool providing the appropriate backend device clusterization algorithm"};
    /// @name The device memory resource tool to use for memory allocations
    ToolHandle<AthDevice::IMemoryResourceTool> m_deviceMR{
        this, "DeviceMR", "",
        "Device memory resource tool"};

    /// @name The name of device resident input traccc cell collection
    SG::ReadHandleKey<traccc::edm::silicon_cell_collection::const_view> m_inputCellsKey{
        this, "InputTracccCells", "",
        "Input traccc cell collection buffer"};

    /// @name The name of device resident output traccc measurement collection
    /// {@
    SG::WriteHandleKey<traccc::edm::measurement_collection::buffer> m_outputMeasKey{
        this, "OutputTracccMeasurements", "",
        "Output uncalibrated traccc measurement collection buffer"};
    SG::WriteHandleKey<traccc::edm::silicon_cluster_collection::buffer> m_outputClusterKey{
        this, "OutputTracccClusters", "",
        "Output uncalibrated traccc cluster collection buffer"};
    /// @}

    // Device buffers — retrieved from detStore
    const traccc::detector_design_description::const_view* m_deviceDesign{nullptr};
    SG::ReadCondHandleKey<traccc::detector_conditions_description::buffer> m_deviceCondObjectName{
        this, "DeviceConditionsObjectName", "TracccDeviceConditionsConfig",
        "Traccc device conditions object"};
};

} // namespace ActsTrk

#endif // ACTSGPUDATAPREPARATION_DEVICECLUSTERIZATIONALG_H
