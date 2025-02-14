//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "TrackParticleCalibratorExampleAlg.h"

#include "TrackParticleCalibrate.h"
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

namespace AthCUDAExamples {

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
  copy(inputHostBuffer, inputDeviceBuffer)->wait();

  // Construct output buffer(s).
  TrackParticleContainer::buffer outputDeviceBuffer(input->size(), deviceMR);
  TrackParticleContainer::buffer outputHostBuffer(input->size(), hostMR);

  // Run the calibration on the device.
  calibrate(inputDeviceBuffer, outputDeviceBuffer);

  // Get the output back to the host.
  copy(outputDeviceBuffer, outputHostBuffer)->wait();

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
