// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "xAODTruthCnv/TruthParticlePrinterAlg.h"

// Framework include(s).
#include "AsgDataHandles/ReadHandle.h"
#include "TruthUtils/MagicNumbers.h"

// EDM include(s).
#include "xAODTruth/TruthVertex.h"

// System include(s).
#include <iomanip>
#include <sstream>

namespace xAODReader {

StatusCode TruthParticlePrinterAlg::initialize() {

  // Initialize the handle.
  ATH_CHECK(m_key.initialize());

  // Return gracefully.
  return StatusCode::SUCCESS;
}

StatusCode TruthParticlePrinterAlg::execute(const EventContext& ctx) const {

  // Print a header.
  ANA_MSG_INFO(
      "------------------------------------------------------------------------"
      "-----------------------");
  ANA_MSG_INFO("                     \"" << m_key.key() << "\"");
  ANA_MSG_INFO(
      "------------------------------------------------------------------------"
      "-----------------------");
  ANA_MSG_INFO(
      "   uniqueID |   pdgId   |     pt    |    eta    |    phi    |     e     "
      "| status | decayVtxID");
  ANA_MSG_INFO(
      "------------------------------------------------------------------------"
      "-----------------------");

  // Access the input container.
  auto container = SG::makeHandle(m_key, ctx);

  // Print the particles one by one.
  for (const xAOD::TruthParticle* particle : *container) {

    // Construct a string with code copied from xAODTruthReader.
    std::ostringstream ss;
    ss.width(9);
    ss << HepMC::uniqueID(particle) << " | ";
    ss.width(9);
    ss << particle->pdgId() << " | ";
    ss.width(9);
    ss.precision(2);
    ss.setf(std::ios::scientific, std::ios::floatfield);
    ss.setf(std::ios_base::showpos);
    ss << particle->pt() << " | ";
    ss.width(9);
    ss.precision(2);
    ss << particle->eta() << " | ";
    ss.width(9);
    ss.precision(2);
    ss << particle->phi() << " | ";
    ss.width(9);
    ss.precision(2);
    ss << particle->e() << " |   ";
    ss.setf(std::ios::fmtflags(0), std::ios::floatfield);
    ss.unsetf(std::ios_base::showpos);
    ss.width(3);
    ss << particle->status() << "  |  ";
    if (particle->hasDecayVtx() &&
        (HepMC::uniqueID(particle->decayVtx()) != HepMC::UNDEFINED_ID)) {
      ss.width(9);
      ss << HepMC::uniqueID(particle->decayVtx());
    } else {
      ss << "   n/a";
    }

    // Properly print the constructed string.
    ANA_MSG_INFO("  " << ss.str());
  }

  // Print a footer.
  ANA_MSG_INFO(
      "------------------------------------------------------------------------"
      "-----------------------");

  // Return gracefully.
  return StatusCode::SUCCESS;
}

}  // namespace xAODReader
