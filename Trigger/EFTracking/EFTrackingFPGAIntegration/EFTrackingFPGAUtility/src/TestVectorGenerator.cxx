/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "EFTrackingFPGAUtility/TestVectorGenerator.h"
#include "EFTrackingFPGAUtility/FPGADataFormatUtilities.h"

#include <random>

namespace EFTrackingFPGAUtility{
TestVectorGenerator::TestVectorGenerator(
  const std::string& name,
  ISvcLocator* pSvcLocator
) : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode TestVectorGenerator::initialize() {
  ATH_MSG_INFO("Initializing " << name());
  ATH_CHECK(m_inputDataStreamKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode TestVectorGenerator::execute(const EventContext& ctx) const {
  SG::WriteHandle<std::vector<unsigned long>>inputDataStream(
    m_inputDataStreamKey,
    ctx
  );

  ATH_CHECK(inputDataStream.record(std::make_unique<std::vector<unsigned long>>()));
  inputDataStream->reserve(m_bufferSize);

  std::random_device randomDevice;
  std::mt19937 randomNumberGenerator(randomDevice());
  std::uniform_int_distribution<uint64_t> uniformDistribution(0, std::numeric_limits<uint64_t>::max());

  for (std::size_t index = 0; index < m_bufferSize - 3; index++) {
    // Create random input with module header (0x55) at each end to avoid alignment issues within each word.
    // Module headers have total length 64bits therefore easiest to get proper alignment across multiple word.
    inputDataStream->push_back(((uniformDistribution(randomNumberGenerator) << 16) >> 8) ^ 0x5500000000000055);
  }

  // Repeat flag to avoid possible alingment issues.
  inputDataStream->push_back(0xcdcdcdcdcdcdcdcd);
  inputDataStream->push_back(0x0000000000000000);
  inputDataStream->push_back(FPGADataFormatUtilities::get_dataformat_EVT_FTR_w3({
    .word_count = m_bufferSize - 3,
    .crc = 0,
  }));

  ATH_CHECK(inputDataStream->size() == m_bufferSize);
  
  return StatusCode::SUCCESS;
}
}

