/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUPATTERNRECOGNITION_DEVICETRKPARAMESTIMATIONALG_H
#define ACTSGPUPATTERNRECOGNITION_DEVICETRKPARAMESTIMATIONALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "IDeviceTrkParamAlgProviderTool.h"
#include "AthDeviceInterfaces/ICopyTool.h"
#include "ActsGPUEvent/TracccMagField.h"
#include "ActsGPUEvent/TracccDetectorGeometryDescription.h"
#include "ActsGPUEvent/TracccTrkParamCollection.h"
#include "ActsGPUEvent/TracccSeedCollection.h"
#include "ActsGPUEvent/TracccSpacepointCollection.h"
#include "ActsGPUEvent/TracccMeasurementCollection.h"

template <typename scalar_t>
using unit = detray::unit<scalar_t>;

namespace ActsTrk {
/**
 * @class DeviceTrkParamEstimationAlg
 *
 * @brief Algorithm executing traccc track parameter estimation on the GPU.
 *
 * The backend-specific track parameter estimation algorithm is provided by
 * a dedicated tool, together with the device memory resource.
 *
 * The algorithm retrieves the device resident traccc seeds collection from the event
 * store, passes them to the device track parameter estimation algorithm and records the
 * resulting device resident traccc track parameter collection back into the event store.
 *
 *
 * @author Neža Ribarič <neza.ribaric@cern.ch>
 */
class DeviceTrkParamEstimationAlg    : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  /// Function initializing the algorithm
  virtual StatusCode initialize() override;
  /// Function executing the algorithm
  virtual StatusCode execute(const EventContext& ctx) const override;

private:

    virtual StatusCode configureTrkParamEstimation();

    /// @name The tool that provides backend-specific traccc track parameter estimation algorithms
    ToolHandle<IDeviceTrkParamAlgProviderTool> m_trkParamAlgProviderTool{
        this, "TrkParamAlgProviderTool", "",
        "Tool providing the appropriate backend device track parameter estimation algorithm"};

    /// @name The name of device resident input traccc spacepoint collection
    SG::ReadHandleKey<traccc::edm::spacepoint_collection::const_view> m_inputSpacePointsKey{
        this, "InputTracccSpacepoints", "",
        "Input traccc space points collection buffer"};

    /// @name The name of device resident input traccc measurement collection
    SG::ReadHandleKey<traccc::edm::measurement_collection::const_view> m_inputMeasKey{
        this, "InputTracccMeasurements", "",
        "Input traccc measurements collection buffer"};

    /// @name The name of device resident input traccc seeds collection
    SG::ReadHandleKey<traccc::edm::seed_collection::const_view> m_inputSeedsKey{
        this, "InputTracccSeeds", "",
        "Input traccc seeds collection buffer"};

    /// @name The name of device resident input traccc inhom. mag. field
    Gaudi::Property<std::string> m_inputMagFieldKey{
        this, "InputTracccMagField", "",
        "Input traccc inhom. mag. field"};
    /// @name The name of device resident input traccc detector geometry,
    /// required for spacepoints made of two measurements
    Gaudi::Property<std::string> m_deviceDetectorObjectName{
        this, "InputTracccDetectorGeometry", "",
        "Input traccc detector geometry"};

    /// @name The name of device resident output traccc track parameter collection
    /// {@
    SG::WriteHandleKey<traccc::bound_track_parameters_collection_types::buffer> m_outputTrkParamKey{
        this, "OutputTracccTrackParameters", "",
        "Output traccc track parameter collection buffer"};
    /// @}

    traccc::track_params_estimation_config m_trkparam_config;
    const traccc::magnetic_field* m_deviceMagField{nullptr};
    const traccc::detector_buffer* m_deviceDetector{nullptr};

};

} // namespace ActsTrk

#endif // ACTSGPUPATTERNRECOGNITION_DEVICETRKPARAMESTIMATIONALG_H
