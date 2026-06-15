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

// System include(s).
#include <cstring>

namespace AthCUDAExamples {

StatusCode TrackParticleCalibratorExampleAlg::initialize() {

  // Initialize the keys.
  ATH_CHECK(m_inputKey.initialize());
  ATH_CHECK(m_outputKey.initialize());

  // Initialize the tools.
  ATH_CHECK(m_hostMR.retrieve());
  ATH_CHECK(m_deviceMR.retrieve());
  ATH_CHECK(m_hostCopyTool.retrieve());
  ATH_CHECK(m_deviceCopyTool.retrieve());
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

  // Get the CUDA stream to use.
  cudaStream_t stream = m_streamTool->stream(ctx);

  // The object managing host memory copies.
  auto hostCopy = m_hostCopyTool->copy(ctx);
  // The object managing device memory copies.
  auto deviceCopy = m_deviceCopyTool->copy(ctx);

  // Construct input buffer(s).
  traccc::edm::track_collection<traccc::default_algebra>::buffer
      inputHostBuffer(std::vector<unsigned int>(input->size(), 0u),
                      m_hostMR->mr());
  traccc::edm::track_collection<traccc::default_algebra>::buffer
      inputDeviceBuffer(std::vector<unsigned int>(input->size(), 0u),
                        m_deviceMR->mr(), &(m_hostMR->mr()));
  hostCopy->setup(inputHostBuffer)->wait();
  deviceCopy->setup(inputDeviceBuffer)->wait();

  // Copy the relevant data into the input buffer.
  traccc::edm::track_collection<traccc::default_algebra>::device inputHost{
      inputHostBuffer};
  for (unsigned int i = 0; i < input->size(); ++i) {
    inputHost[i].params().set_theta(input->at(i)->theta());
    inputHost[i].params().set_phi(input->at(i)->phi());
    inputHost[i].params().set_qop(input->at(i)->qOverP());
  }

  // Copy the input buffer to the device.
  (*deviceCopy)(inputHostBuffer, inputDeviceBuffer)->wait();

  // Construct output buffer(s).
  traccc::edm::track_collection<traccc::default_algebra>::buffer
      outputDeviceBuffer(std::vector<unsigned int>(input->size(), 0u),
                         m_deviceMR->mr(), &(m_hostMR->mr()));
  deviceCopy->setup(outputDeviceBuffer)->wait();
  traccc::edm::track_collection<traccc::default_algebra>::host
      outputHostCollection(m_hostMR->mr());

  // Run the kernel.
  ATH_CHECK(calibrateOnGPU(stream, inputDeviceBuffer, outputDeviceBuffer));

  // Get the output back to the host.
  (*deviceCopy)(outputDeviceBuffer, outputHostCollection)->wait();

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

}  // namespace AthCUDAExamples
