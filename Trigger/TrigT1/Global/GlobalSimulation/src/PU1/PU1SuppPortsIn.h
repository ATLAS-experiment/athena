/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file PU1SuppPortsIn.h
 * @brief Defines the data structure for PU1 Suppression algorithm inputs.
 *
 * This file declares the `PU1SuppPortsIn` struct used to encapsulate a single
 * event's input to the PU1 Suppression algorithm, including a 256-bit TOB (as a
 * hex string) and an associated rho value.
 */

#ifndef GLOBALSIM_GEPALGOPU1SUPPPORTSIN_H
#define GLOBALSIM_GEPALGOPU1SUPPPORTSIN_H

#include <array>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

#include "AthenaKernel/CLASS_DEF.h"

namespace GlobalSim {

/**
 * @brief Data structure representing a single PU1 suppression input.
 *
 * Each object represents one event's data input into the PU1 suppression
 * algorithm, The 256-bit TOB is stored as four 64-bit unsigned integers. The
 * rho value is stored as a uint16_t.
 */
struct PU1SuppPortsIn {
  /// 256-bit TOB stored as four 64-bit unsigned integers
  std::array<uint64_t, 4> m_I_PU1TobData{};

  /// Rho value as a 16-bit initialized unsigned integer
  uint16_t m_rho{0};
};

/**
 * @brief FIFO container of PU1 suppression inputs.
 *
 * This vector holds all the input events to be processed by the PU1 suppression
 * algorithm.
 */
using GepAlgoPU1SuppFIFO = std::vector<PU1SuppPortsIn>;

}  // namespace GlobalSim

/// Output stream operator for easy logging/debug printing
std::ostream& operator<<(std::ostream&, const GlobalSim::PU1SuppPortsIn&);

/// CLASS_DEF macro to register GepAlgoPU1SuppFIFO with Athena's type system
CLASS_DEF(GlobalSim::GepAlgoPU1SuppFIFO, 19191999, 1)

#endif  // GLOBALSIM_GEPALGOPU1SUPPPORTSIN_H
