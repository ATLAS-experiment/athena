/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include <fstream> // For Csv.
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>

#include "EFTrackingFPGAUtility/EFTrackingDataStreamUnloaderAlgorithm.h"

namespace {
std::string toHex(const unsigned long src) {
  std::stringstream stringStream{};
  stringStream << std::hex << src;

  return stringStream.str();
}

std::string padString(const std::string& src, std::size_t width) {
  std::string dst = src;
  dst.insert(dst.begin(), width - src.size(), '0');

  return dst;
}
}

EFTrackingDataStreamUnloaderAlgorithm::EFTrackingDataStreamUnloaderAlgorithm(
  const std::string& name,
  ISvcLocator* pSvcLocator
) : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode EFTrackingDataStreamUnloaderAlgorithm::initialize() {
  ATH_MSG_INFO("Initializing " << name());
  ATH_CHECK(m_outputDataStreamKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode EFTrackingDataStreamUnloaderAlgorithm::execute(const EventContext& ctx) const {
  SG::ReadHandle<std::vector<unsigned long>> outputDataStream(
    m_outputDataStreamKey,
    ctx
  );

  std::ofstream outputFile(m_outputCsvPath);

  if (!outputFile.is_open()) {
    ATH_MSG_ERROR("Failed to write " << m_outputCsvPath);

    return StatusCode::FAILURE;
  }

  for (const unsigned long word : *outputDataStream) {
    outputFile << padString(toHex(word), 16) << std::endl;
  }

  return StatusCode::SUCCESS;
}

