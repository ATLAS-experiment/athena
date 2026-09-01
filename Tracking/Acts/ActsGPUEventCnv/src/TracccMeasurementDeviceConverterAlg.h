/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_TRACCCMEASUREMENTDEVICECONVERTERALG_H
#define ACTSGPUEVENT_TRACCCMEASUREMENTDEVICECONVERTERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "AthCUDAInterfaces/IStreamTool.h"
#include "AthDeviceInterfaces/IMemoryResourcesTool.h"
#include "AthDeviceInterfaces/ICopiesTool.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "GaudiKernel/ToolHandle.h"
#include "DeviceMeasurementContainers.h"
#include "ActsGPUEvent/TracccDetectorConditionsDescription.h"
#include "ActsGPUEvent/TracccDetectorGeometryDescription.h"
#include "ActsGPUEvent/TracccMeasurementCollection.h"

#include <traccc/edm/measurement_collection.hpp>
#include <traccc/geometry/detector.hpp>
#include <vecmem/memory/memory_resource.hpp>
#include <traccc/geometry/detector_conditions_description.hpp>
#include <traccc/geometry/detector_buffer.hpp>

/// Helper macro used for checking @c cudaError_t type return values.
#define CUDA_ERROR_CHECK(EXP)                                      \
  do {                                                             \
    cudaError_t errorCode = EXP;                                   \
    if (errorCode != cudaSuccess) {                                \
      errorcheck::ReportMessage(MSG::ERROR, ERRORCHECK_ARGS,       \
        __PRETTY_FUNCTION__,                                       \
        StatusCode::FAILURE)                                       \
              .msgstream()                                         \
          << "Failed to execute: " << #EXP << " ("                 \
          << cudaGetErrorString(errorCode) << ")";                 \
      return StatusCode::FAILURE;                                  \
    }                                                              \
  } while (false)

namespace ActsTrk {

// vecmem objects use this type for size
using size_type = unsigned int;

/**
 * @class TracccMeasurementDeviceConverterAlg
 *
 * @brief Algorithm converting traccc measurements (device buffer) to xAOD
 * clusters (host container)
 *
 * This algorithm converts the Traccc measurements into dedicated data format on
 * device. The converted data is then transferred to host and used to create the
 * corresponding xAOD cluster objects. The data format is designed to match the
 * memory layout of the xAOD cluster containers so that the on-host "conversion"
 * is only a few memory copy operations.
 *
 * @author Fabrice Le Goff <flegoff@cern.ch>
 */
class TracccMeasurementDeviceConverterAlg : public AthReentrantAlgorithm
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

  /// @name The input device traccc measurement collection name
  SG::ReadHandleKey<traccc::edm::measurement_collection::buffer> m_inputMeasKey{
      this, "InputMeasurements", "TracccMeasurements",
      "Input traccc measurement collection buffer"};

  /// @name The output host resident cluster collection names
  /// {@
  SG::WriteHandleKey<xAOD::PixelClusterContainer> m_outputPixelKey{
      this, "OutputPixelClusters", "ITkTracccPixelClusters",
      "Output xAOD pixel cluster container"};
  SG::WriteHandleKey<xAOD::StripClusterContainer> m_outputStripKey{
      this, "OutputStripClusters", "ITkTracccStripClusters",
      "Output xAOD strip cluster container"};
  /// @}

  /// @name The host memory resource tool to use for memory allocations
  ToolHandle<AthDevice::IMemoryResourcesTool> m_mrTool{
    this, "MemoryResourcesTool", "", "Tool that provides memory resources"};
  /// @name The copy tool used for copying data from device
  ToolHandle<AthDevice::ICopiesTool> m_copiesTool{
    this, "CopiesProviderTool", "", "Tool that provides vecmem copy objects"};
  /// @name Tool that provides CUDA streams
  ToolHandle<AthCUDA::IStreamTool> m_streamTool{
    this, "StreamTool", "", "Tool that provides CUDA streams"};
  /// @name Name of the detector object on device
  Gaudi::Property<std::string> m_deviceDetectorName{
    this, "DeviceDetectorName", "",
    "Detray device detector object"};
  SG::ReadCondHandleKey<traccc::detector_conditions_description::buffer> m_deviceCondKey{
      this, "DeviceConditionsObjectName", "",
      "Key for reading the per-IOV traccc device conditions object"};

  const traccc::detector_buffer* m_detector{nullptr};

  /// The object counters for debug prints in finalize method
  /// {@
  mutable std::atomic<int> m_nPix = 0;
  mutable std::atomic<int> m_nStrip = 0;
  mutable std::atomic<int> m_nMeas = 0;
  /// @}
};

StatusCode convertMeasurementsOnGPU(
    cudaStream_t stream
  , vecmem::memory_resource & mr
  , vecmem::copy const & copy
  , traccc::itk_detector::view const & detector_view
  , traccc::detector_conditions_description::const_view const & detector_conditions_view
  , size_type nb_measurements
  , const traccc::edm::measurement_collection::const_view & input
  , size_type & output_n_pixels
  , size_type & output_n_strips
  , std::unique_ptr<DevicePixelMeasurementContainer::buffer> & output_pixel_meas
  , std::unique_ptr<DeviceStripMeasurementContainer::buffer> & output_strip_meas
  );


} // namespace ActsTrk

#endif // ACTSGPUEVENT_TRACCCMEASUREMENTDEVICECONVERTERALG_H