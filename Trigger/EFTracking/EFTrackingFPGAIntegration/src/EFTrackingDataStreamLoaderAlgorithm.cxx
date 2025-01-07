/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include <fstream> // For Csv.

#include "EFTrackingDataStreamLoaderAlgorithm.h"

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

  std::ifstream inputFile(m_inputCsvPath);

  if (!inputFile.is_open()) {
    ATH_MSG_ERROR("Failed to read " << m_inputCsvPath);

    return StatusCode::FAILURE;
  }

  while (!inputFile.eof()) {
    if (inputDataStream->size() >= m_bufferSize) {
      ATH_MSG_ERROR("Input csv is too large, "  << 
                    m_inputCsvPath <<
                    ", consider increasing buffer size");

      return StatusCode::FAILURE;
    }

    const auto word = [&inputFile]->std::optional<unsigned long> {
                        std::string word{};
                        std::getline(inputFile, word);
                        
                        // (8 ascii character) x (2 char per ascii character)
                        if (word.size() != 16) {
                          return std::nullopt;
                        }

                        return {std::stoul(word.c_str(), nullptr, 16)};
                      }();

    if (word == std::nullopt) {
      ATH_MSG_ERROR("Failed to read " << 
                     m_inputCsvPath << 
                     ", check for trailing characters");

      return StatusCode::FAILURE;
    }

    inputDataStream->push_back(*word);
  }

  return StatusCode::SUCCESS;
}

