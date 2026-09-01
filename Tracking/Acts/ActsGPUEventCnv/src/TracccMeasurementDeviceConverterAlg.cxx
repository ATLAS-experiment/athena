/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TracccMeasurementDeviceConverterAlg.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "AthAllocators/DataPool.h"
#include "src/DeviceMeasurementContainers.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"
#include <memory>
#include <traccc/edm/measurement_collection.hpp>
#include <traccc/geometry/detector.hpp>

namespace {
  const SG::Accessor<unsigned long> s_acc_id("identifier");
  const SG::Accessor<unsigned int> s_acc_id_hash("identifierHash");
  const SG::Accessor<std::array<float, 1>> s_acc_loc_pos_x("localPositionDim1");
  const SG::Accessor<std::array<float, 1>> s_acc_loc_cov_x("localCovarianceDim1");
  const SG::Accessor<std::array<float, 2>> s_acc_loc_pos_xy("localPositionDim2");
  const SG::Accessor<std::array<float, 4>> s_acc_loc_cov_xy("localCovarianceDim2");
  const SG::Accessor<xAOD::ArrayFloat3> s_acc_global_pos("globalPosition");
}

namespace ActsTrk {

StatusCode TracccMeasurementDeviceConverterAlg::initialize()
{
  ATH_CHECK(m_mrTool.retrieve());
  ATH_CHECK(m_copiesTool.retrieve());
  ATH_CHECK(m_streamTool.retrieve());

  ATH_CHECK(m_inputMeasKey.initialize());
  ATH_CHECK(m_outputPixelKey.initialize());
  ATH_CHECK(m_outputStripKey.initialize());
  ATH_CHECK(m_deviceCondKey.initialize());

  ATH_CHECK(detStore()->retrieve(m_detector, m_deviceDetectorName.value()));

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

StatusCode TracccMeasurementDeviceConverterAlg::execute(const EventContext& ctx) const
{
  SG::ReadCondHandle<traccc::detector_conditions_description::buffer>
    detector_conditions_handle{m_deviceCondKey, ctx};

  auto device_copy = m_copiesTool->deviceCopy(ctx);
  auto host_copy = m_copiesTool->hostCopy(ctx);
  cudaStream_t cuda_stream = m_streamTool->stream(ctx);

  // ---- Read input
  auto traccc_measurements = SG::makeHandle(m_inputMeasKey, ctx);
  ATH_CHECK(traccc_measurements.isValid());
  size_type const n_meas = device_copy->get_size(*traccc_measurements);

  // ---- Create output buffers
  // They are allocated during the conversion when we know how many pixels and strips there are
  std::unique_ptr<DevicePixelMeasurementContainer::buffer> converted_pixel_meas_device_buffer{nullptr};
  std::unique_ptr<DeviceStripMeasurementContainer::buffer> converted_strip_meas_device_buffer{nullptr};

  // ---- Conversion on GPU
  size_type n_pixels = 0, n_strips = 0;
  ATH_CHECK(convertMeasurementsOnGPU(cuda_stream, m_mrTool->mainMR(), *device_copy,
    m_detector->as_view<traccc::itk_detector>(), **detector_conditions_handle,
    n_meas, *traccc_measurements, n_pixels, n_strips,
    converted_pixel_meas_device_buffer, converted_strip_meas_device_buffer));

  // Create const device objects to access the field vectors directly
  DevicePixelMeasurementContainer::const_device pixel_meas_device{*converted_pixel_meas_device_buffer};
  DeviceStripMeasurementContainer::const_device strip_meas_device{*converted_strip_meas_device_buffer};

  // ---- Create output SG containers

  // Create data pools
  DataPool<xAOD::PixelCluster> pixel_pool(ctx);
  DataPool<xAOD::StripCluster> strip_pool(ctx);

  // Create non-owning SG containers
  auto pixel_cont = std::make_unique<xAOD::PixelClusterContainer>(
      SG::VIEW_ELEMENTS, SG::ALWAYS_TRACK_INDICES);
  auto strip_cont = std::make_unique<xAOD::StripClusterContainer>(
      SG::VIEW_ELEMENTS, SG::ALWAYS_TRACK_INDICES);

  pixel_pool.reserve(n_pixels);
  strip_pool.reserve(n_strips);

  // Reserve space and set element pointers in SG containers
  pixel_cont->push_new(n_pixels,
      [&pixel_pool]() { return pixel_pool.nextElementPtr(); });
  strip_cont->push_new(n_strips,
      [&strip_pool]() { return strip_pool.nextElementPtr(); });

  // Create Aux containers
  auto pixel_aux = std::make_unique<xAOD::PixelClusterAuxContainer>();
  auto strip_aux = std::make_unique<xAOD::StripClusterAuxContainer>();

  // Make sure the Aux containers have the same size as the corresponding containers
  pixel_aux->resize(pixel_cont->size());
  strip_aux->resize(strip_cont->size());

  // Associate the Aux container to the containers
  pixel_cont->setStore(pixel_aux.get());
  strip_cont->setStore(strip_aux.get());

  // ---- Copy data from vecmem containers on device directly to SG containers
  { // Pixels
    auto span_id = s_acc_id.getDataSpan(*pixel_cont);
    auto span_id_hash = s_acc_id_hash.getDataSpan(*pixel_cont);
    auto span_local_pos = s_acc_loc_pos_xy.getDataSpan(*pixel_cont);
    auto span_local_cov = s_acc_loc_cov_xy.getDataSpan(*pixel_cont);
    auto span_global_pos = s_acc_global_pos.getDataSpan(*pixel_cont);
    CUDA_ERROR_CHECK(cudaMemcpy(span_id.data(), &(pixel_meas_device.identifier().at(0)),
      sizeof(unsigned long) * n_pixels, cudaMemcpyDeviceToHost));
    CUDA_ERROR_CHECK(cudaMemcpy(span_local_pos.data(), &(pixel_meas_device.local_position().at(0)),
      sizeof(float) * n_pixels * 2, cudaMemcpyDeviceToHost));
    CUDA_ERROR_CHECK(cudaMemcpy(span_local_cov.data(), &(pixel_meas_device.local_variance().at(0)),
      sizeof(float) * n_pixels * 4, cudaMemcpyDeviceToHost));
    CUDA_ERROR_CHECK(cudaMemcpy(span_global_pos.data(), &(pixel_meas_device.global_position().at(0)),
      sizeof(float) * n_pixels * 3, cudaMemcpyDeviceToHost));
    CUDA_ERROR_CHECK(cudaMemcpy(span_id_hash.data(), &(pixel_meas_device.module_id_hash().at(0)),
      sizeof(unsigned int) * n_pixels, cudaMemcpyDeviceToHost));
  }
  {  // Strips
    auto span_id = s_acc_id.getDataSpan(*strip_cont);
    auto span_id_hash = s_acc_id_hash.getDataSpan(*strip_cont);
    auto span_local_pos = s_acc_loc_pos_x.getDataSpan(*strip_cont);
    auto span_local_cov = s_acc_loc_cov_x.getDataSpan(*strip_cont);
    CUDA_ERROR_CHECK(cudaMemcpy(span_id.data(), &(strip_meas_device.identifier().at(0)),
      sizeof(unsigned long) * n_strips, cudaMemcpyDeviceToHost));
    CUDA_ERROR_CHECK(cudaMemcpy(span_local_pos.data(), &(strip_meas_device.local_position().at(0)),
      sizeof(float) * n_strips, cudaMemcpyDeviceToHost));
    CUDA_ERROR_CHECK(cudaMemcpy(span_local_cov.data(), &(strip_meas_device.local_variance().at(0)),
      sizeof(float) * n_strips, cudaMemcpyDeviceToHost));
    CUDA_ERROR_CHECK(cudaMemcpy(span_id_hash.data(), &(strip_meas_device.module_id_hash().at(0)),
      sizeof(unsigned int) * n_strips, cudaMemcpyDeviceToHost));
  }

  // ---- Register to SG
  SG::WriteHandle<xAOD::PixelClusterContainer> pixelHandle{m_outputPixelKey, ctx};
  ATH_CHECK(pixelHandle.record(std::move(pixel_cont), std::move(pixel_aux)));

  SG::WriteHandle<xAOD::StripClusterContainer> stripHandle{m_outputStripKey, ctx};
  ATH_CHECK(stripHandle.record(std::move(strip_cont), std::move(strip_aux)));

  // ---- Et voilà
  ATH_MSG_DEBUG("Converted " << n_pixels << " pixel clusters, "
    << n_strips  << " strip clusters");

  m_nMeas += n_pixels + n_strips;
  m_nPix  += n_pixels;
  m_nStrip += n_strips;

  return StatusCode::SUCCESS;
}

StatusCode TracccMeasurementDeviceConverterAlg::finalize()
{
  ATH_MSG_DEBUG("Finalizing.");

  ATH_MSG_DEBUG("Received total number of measurements = " << m_nMeas
                  << ", of which pixel clusters = " << m_nPix
                  << " and strip clusters = " << m_nStrip);

  ATH_MSG_DEBUG("Successfully finalized");
  return StatusCode::SUCCESS;
}

} // namespace ActsTrk
