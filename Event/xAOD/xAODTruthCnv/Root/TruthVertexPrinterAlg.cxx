// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "xAODTruthCnv/TruthVertexPrinterAlg.h"

// Framework include(s).
#include "AsgDataHandles/ReadHandle.h"
#include "TruthUtils/MagicNumbers.h"

// EDM include(s).
#include "xAODTruth/TruthParticle.h"

// System include(s).
#include <iomanip>
#include <sstream>

namespace xAODReader {

StatusCode TruthVertexPrinterAlg::initialize() {

  // Initialize the handle.
  ATH_CHECK(m_key.initialize());

  // Return gracefully.
  return StatusCode::SUCCESS;
}

StatusCode TruthVertexPrinterAlg::execute(const EventContext& ctx) const {

  // Print a header.
  ANA_MSG_INFO(
      "----------------------------------------------------------------------"
      "-");
  ANA_MSG_INFO("                     \"" << m_key.key() << "\"");
  ANA_MSG_INFO(
      "----------------------------------------------------------------------"
      "-");
  ANA_MSG_INFO(
      "   uniqueID |  status  |     x     |     y     |     z     |     t    "
      " ");
  ANA_MSG_INFO(
      "----------------------------------------------------------------------"
      "-");

  // Access the input container.
  auto container = SG::makeHandle(m_key, ctx);

  // Print the particles one by one.
  for (const xAOD::TruthVertex* vertex : *container) {\

    // Construct a string with code copied from xAODTruthReader.
    std::ostringstream ss;
    ss.width(9);
    ss << HepMC::uniqueID(vertex) << " |   ";
    ss.width(5);
    ss << HepMC::status(vertex) << "  | ";
    ss.width(9);
    ss.precision(2);
    ss.setf(std::ios::scientific, std::ios::floatfield);
    ss.setf(std::ios_base::showpos);
    ss << vertex->x() << " | ";
    ss.width(9);
    ss.precision(2);
    ss << vertex->y() << " | ";
    ss.width(9);
    ss.precision(2);
    ss << vertex->z() << " | ";
    ss.width(9);
    ss.precision(2);
    ss << vertex->t();

    // Properly print the constructed string.
    ANA_MSG_INFO("  " << ss.str());
  }

  // Print a footer.
  ANA_MSG_INFO(
      "----------------------------------------------------------------------"
      "-");

  // Return gracefully.
  return StatusCode::SUCCESS;
}

}  // namespace xAODReader
