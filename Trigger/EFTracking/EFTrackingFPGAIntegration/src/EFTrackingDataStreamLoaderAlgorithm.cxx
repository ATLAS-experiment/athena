/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "EFTrackingDataStreamLoaderAlgorithm.h"

#include <optional>
#include <vector>

namespace {
StatusCode getNthEvent(
  const int eventNumber,
  const std::vector<unsigned long>& testVector,
  std::vector<unsigned long>& eventTestVector
) {
  int eventCounter = 0;
  std::size_t wordIndex = 0;

  while (true) {
    if (eventCounter >= eventNumber) {
      break;
    }

    if (wordIndex >= testVector.size()) {
      return StatusCode::FAILURE;
    }

    if ((testVector[wordIndex] >> 56) == 0xCD) {
      wordIndex += 2;
      eventCounter++;
    }

    wordIndex++;
  }

  while (true) {
    if (wordIndex >= testVector.size()) {
      return StatusCode::FAILURE;
    }

    if ((testVector[wordIndex] >> 56) == 0xCD) {
      eventTestVector.push_back(testVector[wordIndex]);
      eventTestVector.push_back(testVector[wordIndex + 1]);
      eventTestVector.push_back(testVector[wordIndex + 2]);

      break;
    }

    eventTestVector.push_back(testVector[wordIndex]);
    wordIndex++;
  }

  return StatusCode::SUCCESS;
}
};

EFTrackingDataStreamLoaderAlgorithm::EFTrackingDataStreamLoaderAlgorithm(
  const std::string& name,
  ISvcLocator* pSvcLocator
) : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode EFTrackingDataStreamLoaderAlgorithm::initialize() {
  ATH_MSG_INFO("Initializing " << name());
  ATH_CHECK(m_inputDataStreamKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode EFTrackingDataStreamLoaderAlgorithm::execute(const EventContext& ctx) const {
  SG::WriteHandle<std::vector<unsigned long>>inputDataStream(
    m_inputDataStreamKey,
    ctx
  );

  ATH_CHECK(inputDataStream.record(std::make_unique<std::vector<unsigned long>>()));
  inputDataStream->reserve(m_bufferSize);

  std::vector<unsigned long> testVector{};
  ATH_CHECK(m_testVectorTool->prepareTV(m_inputCsvPath, testVector));

  if (testVector.size() > m_bufferSize) {
    ATH_MSG_ERROR("Test vector larger than buffer size.");

    return StatusCode::FAILURE;
  }

  {
    std::lock_guard<std::mutex> lock{m_mutex};
    ATH_CHECK(getNthEvent(m_eventNumber, testVector, *inputDataStream));

    m_eventNumber++;
  }
  
  return StatusCode::SUCCESS;
}

