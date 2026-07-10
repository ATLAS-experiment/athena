// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "EventDataCopyExampleAlg.h"

// System include(s).
#include <cassert>
#include <memory>

namespace AthExDevice {

StatusCode EventDataCopyExampleAlg::initialize() {

  // Initialize the StoreGate keys.
  ATH_CHECK(m_inputKey.initialize());
  ATH_CHECK(m_outputKey.initialize());

  // Retrieve the tools.
  ATH_CHECK(m_copies.retrieve());
  ATH_CHECK(m_mrs.retrieve());

  // Return gracefully.
  return StatusCode::SUCCESS;
}

StatusCode EventDataCopyExampleAlg::execute(const EventContext& ctx) const {

  // Create the StoreGate handles.
  auto inputHandle = SG::makeHandle(m_inputKey, ctx);
  auto outputHandle = SG::makeHandle(m_outputKey, ctx);

  // Helper lambda for copying the payload from the input (host) container into
  // a host-accessible output buffer.
  auto copyToBuffer = [](const xAOD::TrackParticleContainer& input,
                         DeviceObjectCollection::buffer& output) {
    assert(input.size() == output.capacity());
    DeviceObjectCollection::device helper{output};
    for (size_t i = 0; i < input.size(); ++i) {
      const xAOD::TrackParticle* inputObj = input.at(i);
      DeviceObject outputObj = helper.at(i);
      outputObj.eta() = inputObj->eta();
      outputObj.phi() = inputObj->phi();
      outputObj.indices().at(0u) = 1u;
      outputObj.indices().at(1u) = 2u;
    }
  };

  // Decide whether an explicit host->device copy will be necessary or not.
  if (m_mrs->hostMR() == nullptr) {

    // In this case we have just one "main" memory resource. This should mean
    // that the memory provided by that one memory resource, is accessible
    // from both the host and the device.

    // Tell the user what we're doing.
    ATH_MSG_DEBUG("Using a single memory resource for both host and device");

    // Create the final/output buffer.
    auto output = std::make_unique<DeviceObjectCollection::buffer>(
        std::vector<unsigned int>(inputHandle->size(), 2), m_mrs->mainMR());
    m_copies->hostCopy(ctx)->setup(*output)->wait();

    // Fill it with data directly.
    copyToBuffer(*inputHandle, *output);

    // Record the output buffer in StoreGate.
    ATH_CHECK(outputHandle.record(std::move(output)));

  } else {

    // In this case we have separate "device"/"main" and "host" memory
    // resources. In this case we first need to set up a buffer on the host, and
    // copy data into that. Then we can set up a buffer on the device, and
    // perform the host->device copy between the two buffers.

    // Tell the user what we're doing.
    ATH_MSG_DEBUG("Using separate memory resources for host and device");

    // Create the host buffer.
    DeviceObjectCollection::buffer hostBuffer(
        std::vector<unsigned int>(inputHandle->size(), 2), *(m_mrs->hostMR()));
    m_copies->hostCopy(ctx)->setup(hostBuffer)->wait();

    // Fill it with data.
    copyToBuffer(*inputHandle, hostBuffer);

    // Get the "device" copy object.
    auto deviceCopy = m_copies->deviceCopy(ctx);

    // Create the device buffer.
    auto output = std::make_unique<DeviceObjectCollection::buffer>(
        std::vector<unsigned int>(inputHandle->size(), 2), m_mrs->mainMR(),
        m_mrs->hostMR());
    deviceCopy->setup(*output)->ignore();

    // Perform the host->device copy.
    (*deviceCopy)(hostBuffer, *output)->wait();

    // Record the output buffer in StoreGate.
    ATH_CHECK(outputHandle.record(std::move(output)));
  }

  // Return gracefully.
  return StatusCode::SUCCESS;
}

}  // namespace AthExDevice
