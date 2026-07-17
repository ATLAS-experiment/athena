//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

#include "AthenaKernel/errorcheck.h"
#include "GaudiKernel/StatusCode.h"
#include "TracccMeasurementDeviceConverterAlg.h"
#include "DeviceMeasurementContainers.h"

#include <vecmem/utils/copy.hpp>
#include <traccc/edm/measurement_collection.hpp>
#include "traccc/geometry/detector_conditions_description.hpp"
#include <detray/geometry/tracking_surface.hpp>
#include <thrust/execution_policy.h>
#include <thrust/scan.h>

namespace ActsTrk {

/// Separate namespace kernel(s).
namespace kernels {

// Position i in the input gets assigned 1 in the pixel output and 0 in the
// strip output and vice versa. This is later used to index and count the
// measurements per type.
__global__ void mark_pixels_strips(
    const traccc::edm::measurement_collection::const_view input_view
  , const vecmem::data::vector_view<unsigned int> output_pixels_view
  , const vecmem::data::vector_view<unsigned int> output_strips_view
  )
{
  const unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;

  traccc::edm::measurement_collection::const_device input(input_view);
  if (index >= input.size()) return;

  vecmem::device_vector<unsigned int> output_pixels(output_pixels_view);
  vecmem::device_vector<unsigned int> output_strips(output_strips_view);

  if (input[index].dimensions() == 2) {
    output_pixels.at(index) = 1;
    output_strips.at(index) = 0;
  } else {
    output_pixels.at(index) = 0;
    output_strips.at(index) = 1;
  }
}

__global__ void convertTracccToAthenaMeasurements(
    const traccc::itk_detector::view detector_view
  , const traccc::detector_conditions_description::const_view detector_conditions_view
  , const traccc::edm::measurement_collection::const_view input_view
  , const vecmem::data::vector_view<const unsigned int> index_pixels_view
  , const vecmem::data::vector_view<const unsigned int> index_strips_view
  , DevicePixelMeasurementContainer::view output_pixel_meas_view
  , DeviceStripMeasurementContainer::view output_strip_meas_view
	)
{
  const unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;

  traccc::edm::measurement_collection::const_device input(input_view);
  if (index >= input.size()) return;

  const traccc::itk_detector::device detector{detector_view};
  const traccc::detector_conditions_description::const_device detector_conditions(detector_conditions_view);
  vecmem::device_vector<const unsigned int> index_pixels(index_pixels_view);
  vecmem::device_vector<const unsigned int> index_strips(index_strips_view);
  DevicePixelMeasurementContainer::device output_pixel_meas(output_pixel_meas_view);
  DeviceStripMeasurementContainer::device output_strip_meas(output_strip_meas_view);
  assert(input.size() == output_pixel_meas.size());
  assert(input.size() == output_strip_meas.size());

  const detray::tracking_surface ts{detector, input[index].surface_link()};

  if (input[index].dimensions() == 2) {
    unsigned int const j = index_pixels[index];
    output_pixel_meas[j].identifier() = index;
    output_pixel_meas[j].local_position() = input[index].local_position();
    output_pixel_meas[j].local_variance() = {
        input[index].local_variance()[0], 0
      , 0, input[index].local_variance()[1]
      };
    output_pixel_meas[j].module_id_hash() = detector_conditions.user_data()[ts.index()];

    output_pixel_meas[j].global_position() = ts.local_to_global({},
        input[index].local_position(), {});
  } else {
    unsigned int const j = index_strips[index];

    unsigned int pos_var_idx = input[index].subspace()[0] == 1 ? 1 : 0;

    output_strip_meas[j].identifier() = index;
    output_strip_meas[j].local_position() = input[index].local_position()[pos_var_idx];
    output_strip_meas[j].local_variance() = input[index].local_variance()[pos_var_idx];
    output_strip_meas[j].module_id_hash() = detector_conditions.user_data()[ts.index()];
  }
}

} //namespace kernels

StatusCode convertMeasurementsOnGPU(
    cudaStream_t stream
  , vecmem::memory_resource & mr
  , vecmem::copy const & copy
  , traccc::itk_detector::view const & detector_view
  , traccc::detector_conditions_description::const_view const & detector_conditions_view
  , size_type nb_measurements
  , const traccc::edm::measurement_collection::const_view & input_meas
  , size_type & output_n_pixels
  , size_type & output_n_strips
  , std::unique_ptr<DevicePixelMeasurementContainer::buffer> & output_pixel_meas
  , std::unique_ptr<DeviceStripMeasurementContainer::buffer> & output_strip_meas
	)
{
  static const unsigned int block_size = 256;
  const unsigned int num_blocks =
      (nb_measurements + block_size - 1) / block_size;

  // Mark which measurement is what type
  vecmem::data::vector_buffer<unsigned int> state_pixels(nb_measurements, mr);
  copy.setup(state_pixels)->ignore();
  vecmem::data::vector_buffer<unsigned int> state_strips(nb_measurements, mr);
  copy.setup(state_strips)->ignore();

  kernels::mark_pixels_strips<<<num_blocks, block_size, 0, stream>>>(
    input_meas, state_pixels, state_strips);

  // Scan: provides index and count
  vecmem::data::vector_buffer<unsigned int> index_pixels(nb_measurements, mr);
  copy.setup(state_pixels)->ignore();
  vecmem::data::vector_buffer<unsigned int> index_strips(nb_measurements, mr);
  copy.setup(state_strips)->ignore();

  // Scan pixels
  const vecmem::device_vector<const unsigned int> state_pixels_device(state_pixels);
  vecmem::device_vector<unsigned int> index_pixels_device(index_pixels);
  thrust::exclusive_scan(
    thrust::cuda::par_nosync(std::pmr::polymorphic_allocator(&mr)).on(stream),
    state_pixels_device.begin(), state_pixels_device.end(),
    index_pixels_device.begin());

  // Scan strips
  const vecmem::device_vector<const unsigned int> state_strips_device(state_strips);
  vecmem::device_vector<unsigned int> index_strips_device(index_strips);
  thrust::exclusive_scan(
    thrust::cuda::par_nosync(std::pmr::polymorphic_allocator(&mr)).on(stream),
    state_strips_device.begin(), state_strips_device.end(),
    index_strips_device.begin());

  // Rretrieve counts from indices last value + last value
  // (exclusive scan: last value is sum_0^(last-1))
  unsigned int last_pixel_value = 0, last_strip_value = 0;
  CUDA_ERROR_CHECK(cudaMemcpy(&output_n_pixels,
    &index_pixels_device[nb_measurements - 1],
    sizeof(unsigned int),
    cudaMemcpyDeviceToHost));
  CUDA_ERROR_CHECK(cudaMemcpy(&output_n_strips,
    &index_strips_device[nb_measurements - 1],
    sizeof(unsigned int),
    cudaMemcpyDeviceToHost));
  CUDA_ERROR_CHECK(cudaMemcpy(&last_pixel_value,
    state_pixels.ptr() +  nb_measurements - 1,
    sizeof(unsigned int),
    cudaMemcpyDeviceToHost));
  CUDA_ERROR_CHECK(cudaMemcpy(&last_strip_value,
    state_strips.ptr() +  nb_measurements - 1,
    sizeof(unsigned int),
    cudaMemcpyDeviceToHost));
  output_n_pixels += last_pixel_value;
  output_n_strips += last_strip_value;

  // Allocate output buffer now that we now their size
  output_pixel_meas = std::make_unique<DevicePixelMeasurementContainer::buffer>(output_n_pixels, mr);
  output_strip_meas = std::make_unique<DeviceStripMeasurementContainer::buffer>(output_n_strips, mr);
  copy.setup(*output_pixel_meas)->ignore();
  copy.setup(*output_strip_meas)->ignore();

  // Do the actual conversion now that we know where is what
  kernels::convertTracccToAthenaMeasurements<<<num_blocks, block_size, 0, stream>>>(
    detector_view, detector_conditions_view, input_meas, index_pixels,
    index_strips, *output_pixel_meas, *output_strip_meas);

  // Check for errors, and wait for the kernel to finish.
  CUDA_ERROR_CHECK(cudaGetLastError());
  CUDA_ERROR_CHECK(cudaDeviceSynchronize());

  return StatusCode::SUCCESS;
}

} // namespace ActsTrk

