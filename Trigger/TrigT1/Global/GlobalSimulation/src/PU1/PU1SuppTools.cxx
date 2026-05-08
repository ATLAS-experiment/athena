/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file PU1SuppTools.cxx
 * @brief Utility functions for bit manipulation, TOB decoding, and suppression
 * logic.
 *
 * This file provides a suite of helper functions to support the PU1 suppression
 * algorithm, including:
 * - Conversion between hex and binary representations
 * - Parsing and rebuilding 256-bit input lines
 * - TOB field extraction
 * - Threshold comparison and suppression bit setting
 */

#include "PU1SuppTools.h"

#include <array>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <ostream>

namespace GlobalSim {

void printFullNumber(const std::vector<std::bitset<64>> &number, MsgStream& msg) {
  for (const auto &part : number) {
    msg << MSG::DEBUG << part << endmsg;  // Print each 64-bit segment
  }
}

std::string remakeFullNumberToBinary(
    const std::vector<std::bitset<64>> &number) {
  return number[0].to_string() + number[1].to_string() + number[2].to_string() +
         number[3].to_string();  // Concatenate all 4 segments
}

std::string remakeFullNumberToHex(const std::vector<std::bitset<64>> &number,
                                  MsgStream &msg) {
  if (number.size() != 4) {
    msg << MSG::ERROR
        << "remakeFullNumberToHex: TOB vector does not contain 4 elements"
        << endmsg;
    return "";
  }
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  for (const auto &part : number) {
    ss << std::setw(16)
       << part.to_ullong();  // Format each 64-bit segment as hex
  }
  return ss.str();
}

void writeFullNumberOut(std::ostream &outFile,
                        std::vector<std::bitset<64>> &line, MsgStream &msg) {
  std::string remadeBinaryString = remakeFullNumberToHex(line, msg);
  msg << MSG::DEBUG << "writeFullNumberOut: hex result: " << remadeBinaryString
      << endmsg;
  outFile << remadeBinaryString << std::endl;  // Output to file
}

void readInputTOB(const std::bitset<64> tob_data, et_type &et_value,
                  eta_type &eta_value, phi_type &phi_value,
                  rsvd_type &rsvd_data) {
  unsigned long long tob_data_ull = tob_data.to_ullong();
  et_value = (tob_data_ull >> 19) & 0xFFFF;  // 16-bit ET from bits 19–34
  phi_value = tob_data_ull & 0xFF;           // 8-bit phi from bits 0–7
  eta_value = (tob_data_ull >> 8) & 0x7FF;   // 11-bit eta from bits 8–18
  rsvd_data =
      (tob_data_ull >> 36) & 0xFFFFFFF;  // 28-bit reserved from bits 36–63
}

// Designed to mimic a hardware subtractor circuit
bool compareThresholdEt(const std::bitset<16>& tobEt,
                        const std::bitset<16>& thresholdEt) {
  int borrow = 0;
  for (int i=0; i < 16; ++i) {
    int diff = (int)tobEt[i] - (int)thresholdEt[i] - borrow;
    borrow = (diff < 0) ? 1 : 0;
  }
  return borrow == 0;
}

eta_index_type Eta_to_index_Converter(eta_type eta) {
  if (eta >= 0 && eta <= 2047) {
    if (eta >= 1900)
      return 19;       // Clamp high eta values to last bin
    return eta / 100;  // Otherwise divide by 100 for bin index
  }
  throw std::out_of_range("eta is out of the valid range (0 to 2047)");
}

rho_index_type Rho_to_index_Converter(rho_type rho) {
  if (rho < 1024) {
    rho_index_type index = rho / 20;  // Compute bin index
    return std::min(index, static_cast<rho_index_type>(
                               49));  // Clamp to max LUT index, cast to
                                      // rho_index_type for min comparison
  } else {
    throw std::out_of_range("rho is out of the valid range (0 to 1023)");
  }
}

StatusCode runSimulation(std::vector<std::bitset<64>> &entry,
                         const std::vector<std::vector<et_type>> &lut,
                         rho_type rho_data, MsgStream &msg) {
  if (entry.size() != 4) {
    msg << MSG::ERROR << "runSimulation: entry must contain exactly 4 TOBs"
        << endmsg;
    return StatusCode::FAILURE;
  }

  for (int i = 0; i < 4; ++i) {
    auto &tob = entry[i];
    et_type et_data;
    eta_type eta_data;
    phi_type phi_data;
    rsvd_type rsvd_data;

    readInputTOB(tob, et_data, eta_data, phi_data,
                 rsvd_data);  // Decode TOB fields

    rho_index_type rho_index{};  // Initialize
    eta_index_type eta_index{};

    try {
      rho_index = Rho_to_index_Converter(rho_data);
      eta_index = Eta_to_index_Converter(eta_data);
    } catch (const std::out_of_range &e) {
      msg << MSG::ERROR << "runSimulation: index conversion failed for TOB "
          << i << ": " << e.what() << endmsg;
      continue;
    }

    if (rho_index >= lut.size() || eta_index >= lut[rho_index].size()) {
      msg << MSG::ERROR
          << "runSimulation: Index out of range for MED_LUT: rho=" << rho_index
          << ", eta=" << eta_index << endmsg;
      continue;
    }

    int threshold = lut[rho_index][eta_index];  // Lookup ET threshold from LUT
    std::bitset<16> etBits(et_data);
    std::bitset<16> thresholdBits(static_cast<uint16_t>(threshold));

    msg << MSG::VERBOSE << "runSimulation TOB[" << i << "]: ET=" << et_data
        << " threshold=" << threshold << endmsg;

    if (compareThresholdEt(etBits, thresholdBits)) {
      tob.set(55);  // Mark TOB as passing (set bit 55)
      msg << MSG::DEBUG << "runSimulation TOB[" << i << "] passes (bit 55 set)"
          << endmsg;
    } else {
      tob.reset(55);  // Mark TOB as failing (clear bit 55)
      msg << MSG::DEBUG << "runSimulation TOB[" << i
          << "] does not pass (bit 55 cleared)" << endmsg;
    }
  }
  return StatusCode::SUCCESS;
}

}  // namespace GlobalSim
