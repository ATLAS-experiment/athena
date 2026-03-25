/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
// PU1suppression.cxx
#include "PU1Suppression.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/StatusCode.h"
#include "PU1SuppPortsIn.h"
#include "PU1SuppPortsOut.h"

#include "PU1SuppLUT.h" //MED_LUT
#include <bitset>
#include <string>
#include <sstream>
#include <ios> //std::hex
#include <iomanip> //std::setfill
#include <utility> //std::move

namespace GlobalSim {

StatusCode runPU1Suppression(const PU1SuppPortsIn& input,
                             PU1SuppPortsOut& output, MsgStream& msg) {

  // Create our four 64-bit TOBs from the input array
  std::vector<std::bitset<64>> tobBits(4);
  for (int i = 0; i < 4; ++i) {
    tobBits[i] = std::bitset<64>(input.m_I_PU1TobData[i]);
  }

  // Pass to the core suppression logic with the LUT and rho value
  const StatusCode runOK = runSimulation(tobBits, MED_LUT, input.m_rho, msg);
  if (runOK.isFailure()) {
    msg << MSG::WARNING << "runPU1Suppression: runSimulation failed" << endmsg;
    return StatusCode::FAILURE;
  }

  // Store the processed output TOB as a single 256-bit hex string
  output.m_outputTobs.clear();
  std::string remadeHex = remakeFullNumberToHex(tobBits, msg);

  // Add in catch to pass on the default TOB back into the event store rather
  // than empty string
  if (remadeHex.empty()) {
    msg << MSG::WARNING
        << "runPU1Suppression: issue with input TOB - pushing back input TOB"
        << endmsg;
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 4; ++i) {
      ss << std::setw(16) << input.m_I_PU1TobData[i];
    }
    output.m_outputTobs.push_back(ss.str());
    return StatusCode::SUCCESS;  // After this, downstream gets at least some
                                 // TOB back
  }
  output.m_outputTobs.push_back(std::move(remadeHex));
  return StatusCode::SUCCESS;
}
}  // namespace GlobalSim
