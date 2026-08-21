/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file PU1SuppTestBench.cxx
 * @brief Implementation of PU1SuppTestBenchAlg, which feeds TOB and rho inputs
 * to the Event Store.
 *
 * This file defines the logic for reading test vectors from file and injecting
 * them into StoreGate containers for downstream PU1 suppression algorithm
 * processing.
 */

#include "PU1SuppTestBench.h"

#include <algorithm>
#include <bitset>
#include <cctype>
#include <fstream>
#include <sstream>
#include <CxxUtils/StringUtils.h>

namespace GlobalSim {

/// Constructor implementation
PU1SuppTestBenchAlg::PU1SuppTestBenchAlg(const std::string& name,
                                         ISvcLocator* pSvcLocator)
    : AthAlgorithm(name, pSvcLocator) {}

/// Athena initialize method
StatusCode PU1SuppTestBenchAlg::initialize() {
  ATH_MSG_INFO("Initializing PU1SuppTestBench");

  CHECK(m_suppFIFO_WriteKey.initialize());
  CHECK(m_PU1SuppExpectations_WriteKey.initialize());

  if (m_testsFileName.empty()) {
    ATH_MSG_FATAL("testsFileName is empty");
    return StatusCode::FAILURE;
  }

  CHECK(init_from_file());

  ATH_MSG_INFO("Loaded " << m_fifos.size() << " FIFOs for input");
  return StatusCode::SUCCESS;
}

/// Athena execute method (runs once per event)
StatusCode PU1SuppTestBenchAlg::execute(const EventContext& ctx) {
  if (m_fifo_ptr >= m_fifos.size()) {
    ATH_MSG_ERROR("No more FIFO data to write");
    return StatusCode::FAILURE;
  }

  // Write TOB FIFO to event store
  SG::WriteHandle<GepAlgoPU1SuppFIFO> h_write(m_suppFIFO_WriteKey, ctx);
  CHECK(h_write.record(std::move(m_fifos[m_fifo_ptr])));

  // Write dummy expectation to event store (placeholder)
  auto expectations = std::make_unique<PU1SuppExpectations>("0000", "0000");
  SG::WriteHandle<PU1SuppExpectations> h_write_exp(
      m_PU1SuppExpectations_WriteKey, ctx);
  CHECK(h_write_exp.record(std::move(expectations)));

  ++m_fifo_ptr;
  return StatusCode::SUCCESS;
}

/// Read TOB and rho test vectors from input files and populate FIFO buffer
StatusCode PU1SuppTestBenchAlg::init_from_file() {
  std::ifstream tobFile(m_testsFileName);
  std::ifstream rhoFile(m_rhoFileName);

  if (!tobFile) {
    ATH_MSG_FATAL("Failed to open TOB input file: " << m_testsFileName);
    return StatusCode::FAILURE;
  }
  if (!rhoFile) {
    ATH_MSG_FATAL("Failed to open rho input file: " << m_rhoFileName);
    return StatusCode::FAILURE;
  }

  std::string tobLine, rhoLine;
  int lineNum = 0;

  while (std::getline(tobFile, tobLine) && std::getline(rhoFile, rhoLine)) {
    ++lineNum;

    std::string tobStr = trim(tobLine);
    std::string rhoStr = trim(rhoLine);

    const bool isHex = (tobStr.length() == 64 &&
                        std::all_of(tobStr.begin(), tobStr.end(), ::isxdigit));

    const bool isBinary =
        (tobStr.length() == 256 &&
         std::all_of(tobStr.begin(), tobStr.end(),
                     [](char c) { return c == '0' || c == '1'; }));

    if (!isHex && !isBinary) {
      ATH_MSG_WARNING("Skipping bad TOB line "
                      << lineNum
                      << ": expected 64 length hex or 256 length bit");
      continue;
    }

    auto fifo = std::make_unique<GepAlgoPU1SuppFIFO>();
    PU1SuppPortsIn ports_in;

    const int chunkSize = isHex ? 16 : 64;
    std::string_view tobView = tobStr;
    bool parseError = false;

    for (int i=0; i < 4; ++i) {
      auto tobWord = tobView.substr(i * chunkSize, chunkSize);
      uint64_t value = 0;
      auto [ptr, ec] = std::from_chars(
          tobWord.data(),
          tobWord.data() + tobWord.size(),
          value,
          isHex ? 16 : 2
      );
      if (ec != std::errc{}) {
        ATH_MSG_WARNING("Skipping bad TOB line " << lineNum
                        << ": failed to parse section " << i);
        parseError = true;
        break;
      }
      ports_in.m_I_PU1TobData[i] = value;
    }

    if (parseError) continue;
    
    ports_in.m_rho = static_cast<uint16_t>(std::stoul(rhoStr, nullptr, 2));

    fifo->push_back(ports_in);
    m_fifos.push_back(std::move(fifo));
  }

  return StatusCode::SUCCESS;
}

/// Helper to trim leading and trailing whitespace from a string
std::string PU1SuppTestBenchAlg::trim(const std::string& s) {
  return std::string(CxxUtils::trimWhiteSpaces(s));
}

}  // namespace GlobalSim
