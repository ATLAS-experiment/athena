//
// Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "TrackParticleCalibratorExampleAlg.h"
#include "TrackParticleContainer.h"

// Framework include(s).
#include "AthContainers/tools/copyAuxStoreThinned.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "xAODCore/AuxContainerBase.h"

// VecMem include(s).
#include <vecmem/memory/cuda/device_memory_resource.hpp>
#include <vecmem/memory/host_memory_resource.hpp>
#include <vecmem/utils/cuda/copy.hpp>

// System include(s).
#include <cstring>

/// Helper macro used for checking @c cudaError_t type return values.
#define CUDA_ERROR_CHECK(EXP)                      \
  do {                                             \
    cudaError_t errorCode = EXP;                   \
    if (errorCode != cudaSuccess) {                \
      REPORT_ERROR(StatusCode::FAILURE)            \
          << "Failed to execute: " << #EXP << " (" \
          << cudaGetErrorString(errorCode) << ")"; \
      return StatusCode::FAILURE;                  \
    }                                              \
  } while (false)

namespace AthCUDAExamples {

/// Separate namespace for the example CUDA kernel(s).
namespace kernels {

/// Dummy kernel performing a trivial transformation on the track particle
/// parameters.
__global__ void trackParticleCalibrate(
    const TrackParticleContainer::const_view input_view,
    TrackParticleContainer::view output_view) {

  // Get the current thread's index.
  const unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;

  // Create the device containers.
  TrackParticleContainer::const_device input(input_view);
  TrackParticleContainer::device output(output_view);
  assert(input.size() == output.size());

  // Check that the index is in range.
  if (index < input.size()) {
    // Copy the angle parameters as they are.
    output.theta()[index] = input.theta()[index];
    output.phi()[index] = input.phi()[index];

    // Transform the momentum in some silly way.
    output.qOverP()[index] =
        input.qOverP()[index] *
        std::abs((input.theta()[index] - input.phi()[index]) /
                 input.phi()[index]);
  }

  return;
}

}  // namespace kernels

StatusCode TrackParticleCalibratorExampleAlg::initialize() {

  // Initialize the keys.
  ATH_CHECK(m_inputKey.initialize());
  ATH_CHECK(m_outputKey.initialize());

  // Print some information about the configuration:
  ATH_MSG_INFO("Input container key: " << m_inputKey);
  ATH_MSG_INFO("Output container key: " << m_outputKey);

  // Return gracefully.
  return StatusCode::SUCCESS;
}

StatusCode TrackParticleCalibratorExampleAlg::execute(
    const EventContext& ctx) const {

  // Retrieve the input container.
  auto inputHandle = SG::makeHandle(m_inputKey, ctx);
  const xAOD::TrackParticleContainer* input = inputHandle.cptr();
  if (input == nullptr) {
    ATH_MSG_ERROR("Failed to retrieve input container from: " << m_inputKey);
    return StatusCode::FAILURE;
  }

  // The number of input/output tracks.
  const std::size_t nTracks = input->size();

  // If the input container is empty, then create an empty output container, and
  // be done with it.
  if (nTracks == 0) {
    auto output = std::make_unique<xAOD::TrackParticleContainer>();
    auto outputAux = std::make_unique<xAOD::AuxContainerBase>();
    auto outputHandle = SG::makeHandle(m_outputKey, ctx);
    ATH_CHECK(outputHandle.record(std::move(output), std::move(outputAux)));
    return StatusCode::SUCCESS;
  }

  /// Memory resource for host allocations
  vecmem::host_memory_resource hostMR;
  /// Memory resource for device allocations
  vecmem::cuda::device_memory_resource deviceMR;
  // The object managing CUDA memory copies.
  vecmem::cuda::copy copy;

  // Construct input buffer(s).
  TrackParticleContainer::buffer inputHostBuffer(input->size(), hostMR);
  TrackParticleContainer::buffer inputDeviceBuffer(input->size(), deviceMR);

  // Copy the relevant data into the input buffer.
  static const SG::AuxElement::ConstAccessor<float> thetaAcc("theta");
  static const SG::AuxElement::ConstAccessor<float> phiAcc("phi");
  static const SG::AuxElement::ConstAccessor<float> qOverPAcc("qOverP");
  std::memcpy(inputHostBuffer.get<0>().ptr(), thetaAcc.getDataArray(*input),
              nTracks * sizeof(float));
  std::memcpy(inputHostBuffer.get<1>().ptr(), phiAcc.getDataArray(*input),
              nTracks * sizeof(float));
  std::memcpy(inputHostBuffer.get<2>().ptr(), qOverPAcc.getDataArray(*input),
              nTracks * sizeof(float));

  // Copy the input buffer to the device.
  copy(inputHostBuffer, inputDeviceBuffer);

  // Construct output buffer(s).
  TrackParticleContainer::buffer outputDeviceBuffer(input->size(), deviceMR);
  TrackParticleContainer::buffer outputHostBuffer(input->size(), hostMR);

  // Launch the kernel.
  static const unsigned int block_size = 256;
  const unsigned int num_blocks =
      (inputDeviceBuffer.capacity() + block_size - 1) / block_size;
  kernels::trackParticleCalibrate<<<num_blocks, block_size>>>(
      inputDeviceBuffer, outputDeviceBuffer);

  // Check for errors, and wait for the kernel to finish.
  CUDA_ERROR_CHECK(cudaGetLastError());
  CUDA_ERROR_CHECK(cudaDeviceSynchronize());

  // Get the output back to the host.
  copy(outputDeviceBuffer, outputHostBuffer);

  // Construct the output container.
  auto outputAux = std::make_unique<xAOD::AuxContainerBase>();
  SG::copyAuxStoreThinned(*(input->getConstStore()), *outputAux, nullptr);
  std::memcpy(outputAux->getData(thetaAcc.auxid(), nTracks, nTracks),
              outputHostBuffer.get<0>().ptr(), nTracks * sizeof(float));
  std::memcpy(outputAux->getData(phiAcc.auxid(), nTracks, nTracks),
              outputHostBuffer.get<1>().ptr(), nTracks * sizeof(float));
  std::memcpy(outputAux->getData(qOverPAcc.auxid(), nTracks, nTracks),
              outputHostBuffer.get<2>().ptr(), nTracks * sizeof(float));
  auto output = std::make_unique<xAOD::TrackParticleContainer>();
  for (std::size_t i = 0; i < nTracks; ++i) {
    output->push_back(new xAOD::TrackParticle());
  }
  output->setStore(outputAux.get());

  // Record the output container.
  auto outputHandle = SG::makeHandle(m_outputKey, ctx);
  ATH_CHECK(outputHandle.record(std::move(output), std::move(outputAux)));

  // Return gracefully.
  return StatusCode::SUCCESS;
}

}  // namespace AthCUDAExamples
