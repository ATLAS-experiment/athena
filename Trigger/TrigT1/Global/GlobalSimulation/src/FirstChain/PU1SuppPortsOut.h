/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file PU1SuppPortsOut.h
 * @brief Defines the output structure for PU1 suppression results.
 *
 * Contains a list of output TOBs in hexadecimal and an optional multiplicity count.
 */

#ifndef GLOBALSIM_PU1SUPPPORTSOUT_H
#define GLOBALSIM_PU1SUPPPORTSOUT_H

#include "AthenaKernel/CLASS_DEF.h"
#include <vector>
#include <string>

namespace GlobalSim {

/**
 * @struct PU1SuppPortsOut
 * @brief Output data structure for PU1 suppression.
 *
 * This struct holds the result of the PU1 suppression algorithm, which includes:
 * - A list of TOBs (as hex strings) that passed the suppression cut.
 * - A multiplicity counter (optional usage) for the number of accepted TOBs.
 */
struct PU1SuppPortsOut {
    /// List of TOBs that passed suppression, in 64-character hex string format.
    std::vector<std::string> m_outputTobs;

    /// Optional: Number of TOBs that passed suppression.
    unsigned int m_outputMultiplicity{0};
};

/**
 * @typedef GepAlgoPU1SuppPortsOutFIFO
 * @brief Vector of PU1SuppPortsOut, one element per input event.
 */
using GepAlgoPU1SuppPortsOutFIFO = std::vector<PU1SuppPortsOut>;

} // namespace GlobalSim

/// Register single PU1SuppPortsOut object for StoreGate
CLASS_DEF(GlobalSim::PU1SuppPortsOut, 218218218, 1)

/// Register vector of PU1SuppPortsOut objects for StoreGate
CLASS_DEF(GlobalSim::GepAlgoPU1SuppPortsOutFIFO, 19192000, 1)

#endif // GLOBALSIM_PU1SUPPPORTSOUT_H

