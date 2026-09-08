//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "TrackParticleCalibratorExampleAlg.h"

// Framework include(s).
#include "AthContainers/tools/copyAuxStoreThinned.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "xAODCore/AuxContainerBase.h"

// ROCm/HIP include(s).
#include <hip/hip_runtime.h>

// System include(s).
#include <cstring>

/// Helper macro used for checking @c hipError_t type return values.
#define HIP_ERROR_CHECK(EXP)                                                 \
  do {                                                                       \
    hipError_t errorCode = EXP;                                              \
    if (errorCode != hipSuccess) {                                           \
      ATH_MSG_ERROR("Failed to execute: "                                    \
                    << #EXP << " (" << hipGetErrorString(errorCode) << ")"); \
      return StatusCode::FAILURE;                                            \
    }                                                                        \
  } while (false)

namespace AthHIPExamples {

/// Separate namespace for the example HIP kernel(s).
namespace kernels {

/// Dummy kernel performing a trivial transformation on the track particle
/// parameters.
__global__ void trackParticleCalibrate(
    const traccc::edm::track_collection<traccc::default_algebra>::const_view
        input_view,
    traccc::edm::track_collection<traccc::default_algebra>::view output_view) {

  // Get the current thread's index.
  const unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;

  // Create the device containers.
  traccc::edm::track_collection<traccc::default_algebra>::const_device input(
      input_view);
  traccc::edm::track_collection<traccc::default_algebra>::device output(
      output_view);
  assert(input.size() == output.size());

  // Check that the index is in range.
  if (index < input.size()) {
    // Copy the angle parameters as they are.
    output[index].params().set_theta(input.params()[index].theta());
    output.at(index).params().set_phi(input.params().at(index).phi());

    // Transform the momentum in some silly way.
    output[index].params().set_qop(
        input[index].params().qop() *
        std::abs((input[index].params().theta() - input[index].params().phi()) /
                 input[index].params().phi()));
  }

  return;
}

}  // namespace kernels

StatusCode TrackParticleCalibratorExampleAlg::initialize() {

  // Initialize the keys.
  ATH_CHECK(m_inputKey.initialize());
  ATH_CHECK(m_outputKey.initialize());

  // Initialize the tools.
  ATH_CHECK(m_mrTool.retrieve());
  ATH_CHECK(m_copyTool.retrieve());
  ATH_CHECK(m_streamTool.retrieve());

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

  // Get the HIP stream to use.
  hipStream_t stream = m_streamTool->stream(ctx);

  // The object managing host memory copies.
  auto hostCopy = m_copyTool->hostCopy(ctx);
  // The object managing device memory copies.
  auto deviceCopy = m_copyTool->deviceCopy(ctx);

  // Buffer size(s) to create.
  const std::vector<unsigned int> bufferSize(input->size(), 0u);

  // Construct input buffer(s).
  auto inputHostBuffer =
      m_mrTool->hostMR()
          ? traccc::edm::track_collection<
                traccc::default_algebra>::buffer{bufferSize,
                                                 *(m_mrTool->hostMR())}
          : traccc::edm::track_collection<traccc::default_algebra>::buffer{
                bufferSize, m_mrTool->mainMR()};
  auto inputDeviceBuffer =
      m_mrTool->hostMR()
          ? traccc::edm::track_collection<
                traccc::default_algebra>::buffer{bufferSize, m_mrTool->mainMR(),
                                                 m_mrTool->hostMR()}
          : traccc::edm::track_collection<traccc::default_algebra>::buffer{};
  hostCopy->setup(inputHostBuffer)->wait();
  deviceCopy->setup(inputDeviceBuffer)->ignore();

  // Copy the relevant data into the input buffer.
  traccc::edm::track_collection<traccc::default_algebra>::device inputHost{
      inputHostBuffer};
  for (unsigned int i = 0; i < input->size(); ++i) {
    inputHost[i].params().set_theta(input->at(i)->theta());
    inputHost[i].params().set_phi(input->at(i)->phi());
    inputHost[i].params().set_qop(input->at(i)->qOverP());
  }

  // Copy the input buffer to the device.
  if (m_mrTool->hostMR()) {
    (*deviceCopy)(inputHostBuffer, inputDeviceBuffer)->ignore();
  }

  // Construct output buffer(s).
  traccc::edm::track_collection<traccc::default_algebra>::buffer
      outputDeviceBuffer(bufferSize, m_mrTool->mainMR(), m_mrTool->hostMR());
  if (m_mrTool->hostMR()) {
    deviceCopy->setup(outputDeviceBuffer)->ignore();
  } else {
    hostCopy->setup(outputDeviceBuffer)->ignore();
  }
  traccc::edm::track_collection<traccc::default_algebra>::host
      outputHostCollection(m_mrTool->hostMR() ? *(m_mrTool->hostMR())
                                              : m_mrTool->mainMR());

  // Run the kernel.
  hipLaunchKernelGGL(kernels::trackParticleCalibrate,
                     dim3((nTracks + 255) / 256), dim3(256), 0, stream,
                     m_mrTool->hostMR() ? inputDeviceBuffer : inputHostBuffer,
                     outputDeviceBuffer);
  HIP_ERROR_CHECK(hipGetLastError());
  HIP_ERROR_CHECK(hipDeviceSynchronize());

  // Get the output back to the host.
  if (m_mrTool->hostMR()) {
    (*deviceCopy)(outputDeviceBuffer, outputHostCollection)->wait();
  } else {
    (*hostCopy)(outputDeviceBuffer, outputHostCollection)->wait();
  }

  // Construct the output container.
  auto outputAux = std::make_unique<xAOD::AuxContainerBase>();
  SG::copyAuxStoreThinned(*(input->getConstStore()), *outputAux, nullptr);
  auto output = std::make_unique<xAOD::TrackParticleContainer>();
  for (std::size_t i = 0; i < nTracks; ++i) {
    output->push_back(std::make_unique<xAOD::TrackParticle>());
  }
  output->setStore(outputAux.get());
  for (std::size_t i = 0; i < nTracks; ++i) {
    xAOD::TrackParticle* track = output->at(i);
    track->setDefiningParameters(input->at(i)->d0(), input->at(i)->z0(),
                                 outputHostCollection[i].params().phi(),
                                 outputHostCollection[i].params().theta(),
                                 outputHostCollection[i].params().qop());
  }

  // Record the output container.
  auto outputHandle = SG::makeHandle(m_outputKey, ctx);
  ATH_CHECK(outputHandle.record(std::move(output), std::move(outputAux)));

  // Return gracefully.
  return StatusCode::SUCCESS;
}

}  // namespace AthHIPExamples
