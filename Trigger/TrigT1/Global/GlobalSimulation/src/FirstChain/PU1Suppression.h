/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/// \file PU1Suppression.h
/// \brief Interface for the PU1 suppression algorithm logic.
///
/// Defines the function `runPU1suppression`, which processes a single TOB
/// (Trigger Object Block) input and applies pile-up suppression based on a
/// lookup table and rho value. Input and output are passed via structured
/// ports.

#ifndef PU1SUPPRESSION_H
#define PU1SUPPRESSION_H

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/StatusCode.h"
#include "PU1SuppPortsIn.h"
#include "PU1SuppPortsOut.h"

namespace GlobalSim {

/**
 * @brief Runs PU1 suppression algorithm on a single event.
 *
 * This function takes a 256-bit TOB input in hex format and a rho value encoded
 * as a binary string, splits the TOB into 4x64-bit components, and applies
 * pile-up suppression using a predefined LUT. It returns the resulting TOB in
 * hex format.
 *
 * @param input  The input ports containing the TOB hex string and rho value.
 * @param output The output ports to hold the suppressed TOB hex string.
 *
 * @return True if the suppression succeeded and output was written; false if
 * input was invalid.
 */
StatusCode runPU1Suppression(const PU1SuppPortsIn& input,
                             PU1SuppPortsOut& output, MsgStream& msg);

}  // namespace GlobalSim

#endif  // PU1SUPPRESSION_H
