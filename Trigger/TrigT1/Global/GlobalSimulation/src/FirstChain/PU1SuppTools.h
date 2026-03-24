/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file PU1SuppTools.h
 * @brief Header for utility functions and type definitions used in PU1
 * suppression logic.
 *
 * This module defines the core types and declares helper functions for:
 * - Converting between binary, hexadecimal, and numeric types
 * - Extracting and manipulating TOB fields
 * - Running suppression simulations based on LUT thresholds
 */

#ifndef PU1SUPPTOOLS_H
#define PU1SUPPTOOLS_H

#include <bitset>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/StatusCode.h"

namespace GlobalSim {

/** @name Type aliases for clarity in TOB manipulation */
///@{
using rho_type = uint16_t;           ///< 16-bit type for pileup energy density
using tob_type = uint64_t;           ///< 64-bit type for individual TOB segment
using et_type = uint16_t;            ///< 16-bit type for transverse energy
using phi_type = uint8_t;            ///< 8-bit type for azimuthal angle
using eta_type = int;                ///< Signed integer for pseudorapidity
using rsvd_type = int;               ///< Integer for reserved data field
using rho_index_type = std::size_t;  ///< Index derived from rho value
using eta_index_type = std::size_t;  ///< Index derived from eta value
///@}

/**
 * @brief Prints a full 256-bit number (4x 64-bit).
 */
void printFullNumber(const std::vector<std::bitset<64>> &number, MsgStream& msg);

/**
 * @brief Converts a vector of 4x 64-bit TOBs to a 256-bit binary string.
 */
std::string remakeFullNumberToBinary(
    const std::vector<std::bitset<64>> &number);

/**
 * @brief Converts a vector of 4x 64-bit TOBs to a 64-character hex string.
 */
std::string remakeFullNumberToHex(const std::vector<std::bitset<64>> &number,
                                  MsgStream &msg);

/**
 * @brief Extracts ET, eta, phi, and reserved bits from a 64-bit TOB.
 */
void readInputTOB(std::bitset<64> tob_data, et_type &et_value,
                  eta_type &eta_value, phi_type &phi_value,
                  rsvd_type &rsvd_data);

/**
 * @brief Writes a vector of TOBs as a hex string to an output stream.
 */
void writeFullNumberOut(std::ostream &outFile,
                        std::vector<std::bitset<64>> &line, MsgStream &msg);

/**
 * @brief Converts a raw rho value to its LUT index.
 */
rho_index_type Rho_to_index_Converter(rho_type rho);

/**
 * @brief Converts a raw eta value to its LUT index.
 */
eta_index_type Eta_to_index_Converter(eta_type eta);

/**
 * @brief Applies LUT-based suppression logic to a vector of TOBs.
 * @param entry The TOBs to process (must be size 4).
 * @param lut The LUT containing ET thresholds.
 * @param rho_data A 16-bit uint16_t representation of rho.
 * @param msg Athena message stream for error reporting.
 * @return StatusCode::SUCCESS or StatusCode::FAILURE.
 */
StatusCode runSimulation(std::vector<std::bitset<64>> &entry,
                         const std::vector<std::vector<et_type>> &lut,
                         rho_type rho_data, MsgStream &msg);

}  // namespace GlobalSim

#endif  // PU1SUPPTOOLS_H
