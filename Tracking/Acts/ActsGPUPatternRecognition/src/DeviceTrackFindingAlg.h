/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUPATTERNRECOGNITION_DEVICETRACKFINDINGALG_H
#define ACTSGPUPATTERNRECOGNITION_DEVICETRACKFINDINGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "IDeviceTrackFindingAlgProviderTool.h"
#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "ActsGPUEvent/TracccMagField.h"
#include "ActsGPUEvent/TracccDetectorGeometryDescription.h"
#include "ActsGPUEvent/TracccDetectorGeometryDescription.h"
#include "ActsGPUEvent/TracccTrkParamCollection.h"
#include "ActsGPUEvent/TracccMeasurementCollection.h"
#include "ActsGPUEvent/TracccTrackContainer.h"

namespace ActsTrk {
/**
 * @class DeviceTrackFindingAlg
 *
 * @brief Algorithm executing traccc track finding on the GPU.
 *
 * The backend-specific track finding algorithm is provided by
 * a dedicated tool, together with the device memory resource.
 *
 * The algorithm retrieves the device resident traccc track parameter and measurement collection from the event
 * store, passes them to the device track reconstruction algorithm and records the
 * resulting device resident traccc track and track states collections back into the event store.
 *
 *
 * @author Neža Ribarič <neza.ribaric@cern.ch>
 */
class DeviceTrackFindingAlg    : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  /// Function initializing the algorithm
  virtual StatusCode initialize() override;
  /// Function executing the algorithm
  virtual StatusCode execute(const EventContext& ctx) const override;

private:

    virtual StatusCode configureTrackFinding();

    /// @name The tool that provides backend-specific traccc track parameter estimation algorithms
    ToolHandle<IDeviceTrackFindingAlgProviderTool> m_trkFindingAlgProviderTool{
        this, "TrackFindingAlgProviderTool", "",
        "Tool providing the appropriate backend device track finding algorithm"};

    /// @name The name of device resident input traccc measurement collection
    SG::ReadHandleKey<traccc::edm::measurement_collection::const_view> m_inputMeasKey{
        this, "InputTracccMeasurements", "",
        "Input traccc measurements collection buffer"};
    
    /// @name The name of device resident input traccc inhom. mag. field
    Gaudi::Property<std::string> m_inputMagFieldKey{
        this, "InputTracccMagField", "",
        "Input traccc inhom. mag. field"};  
    /// @name The name of device resident input traccc detector geometry
    Gaudi::Property<std::string> m_deviceDetectorObjectName{
        this, "InputTracccDetectorGeometry", "",
        "Input traccc detector geometry"};             

    /// @name The name of device resident input traccc track parameter collection
    /// {@
    SG::WriteHandleKey<traccc::bound_track_parameters_collection_types::buffer> m_inputTrkParamKey{
        this, "InputTracccTrackParameters", "",
        "Input traccc track parameter collection buffer"};
    /// @}

    /// @name The name of device resident output traccc track collection
    /// {@
    SG::WriteHandleKey<traccc_track_container::buffer> m_outputTracksKey{
        this, "OutputTracccTracks", "",
        "Output traccc tracks collection buffer"};
    /// @}
    
    traccc::finding_config m_finding_cfg;
    const traccc::magnetic_field* m_deviceMagField{nullptr};
    const traccc::detector_buffer* m_deviceDetector{nullptr};

};

} // namespace ActsTrk

#endif // ACTSGPUPATTERNRECOGNITION_DEVICETRACKFINDINGALG_H
