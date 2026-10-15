//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "DeviceMemoryResourceTool.h"

// CUDA include(s).
#include <cuda_runtime_api.h>

// System include(s).
#include <cassert>

namespace AthCUDA {

StatusCode DeviceMemoryResourceTool::initialize() {

  // Construct the appropriate memory resource.
  m_mr = std::make_unique<vecmem::cuda::device_memory_resource>(m_deviceID);

  // Figure out which device the memory resource was created for. Unfortunately
  // vecmem doesn't advertise this in a convenient way at the moment. :( So we
  // have to re-create its logic here.
  int deviceID = m_deviceID;
  if (deviceID == vecmem::cuda::device_memory_resource::INVALID_DEVICE) {
    // The default device was requested. We have to query for the current
    // device.
    cudaError_t err = cudaGetDevice(&deviceID);
    if (err != cudaSuccess) {
      ATH_MSG_ERROR("Failed to query the current CUDA device: "
                    << cudaGetErrorString(err));
      return StatusCode::FAILURE;
    }
  }

  // Get the device's properties.
  cudaDeviceProp props;
  cudaError_t err = cudaGetDeviceProperties(&props, deviceID);
  if (err != cudaSuccess) {
    ATH_MSG_ERROR("Failed to get the CUDA device properties: "
                  << cudaGetErrorString(err));
    return StatusCode::FAILURE;
  }

  // Construct a unique name out of those properties.
  std::ostringstream deviceName;
  deviceName << props.name << " [id: " << deviceID
             << ", bus: " << props.pciBusID << ", device: " << props.pciDeviceID
             << "]";
  m_deviceName = deviceName.str();

  // Return gracefully.
  return StatusCode::SUCCESS;
}

std::pmr::memory_resource& DeviceMemoryResourceTool::mr() const {

  assert(m_mr);
  return *m_mr;
}

std::string_view DeviceMemoryResourceTool::deviceName() const {

  return m_deviceName;
}

}  // namespace AthCUDA
