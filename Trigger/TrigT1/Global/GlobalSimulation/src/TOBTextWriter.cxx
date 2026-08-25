/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */
#include "TOBTextWriter.h"

#include "SpecRegistry.h"

namespace GlobalSim {

  StatusCode TOBTextWriter::initialize() {
    ATH_CHECK(m_inKey.initialize());

    const auto entry = specRegistry().find(m_specName.value());
    if (entry == specRegistry().end()) {
      ATH_MSG_ERROR("Unknown BitSpec '" << m_specName.value()
                    << "'. Known specs are: " << knownSpecNames());
      return StatusCode::FAILURE;
    }
    m_width = entry->second.width;
    m_pack = entry->second.pack;

    // Opened here rather than in finalize() so that a job which cannot write
    // its output fails before running the event loop.
    m_out.open(m_outFile.value());
    if (!m_out) {
      ATH_MSG_ERROR("Could not open output file '" << m_outFile.value() << "'");
      return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("Writing '" << m_inKey.key() << "' to '" << m_outFile.value()
                 << "' as " << m_specName.value() << ", " << m_width / 4
                 << " hex characters per TOB");
    return StatusCode::SUCCESS;
  }

  StatusCode TOBTextWriter::execute(const EventContext& ctx) {
    // Lines go out as the events arrive, so the events have to arrive in order.
    // The first one seen sets the sequence; a gap after that means more than one
    // event is in flight and the line order can no longer be trusted.
    if (m_nWritten == 0) { m_nextEvt = ctx.evt(); }
    if (ctx.evt() != m_nextEvt) {
      ATH_MSG_ERROR("event " << ctx.evt() << " arrived out of order (expected "
                    << m_nextEvt << "), so line N would no longer be event N. "
                    << "Run this job with a single concurrent event.");
      return StatusCode::FAILURE;
    }

    SG::ReadHandle<xAOD::BaseContainer> tobs(m_inKey, ctx);
    if (!tobs.isValid()) {
      ATH_MSG_ERROR("Could not retrieve '" << m_inKey.key() << "'");
      return StatusCode::FAILURE;
    }

    std::string line;
    for (const SG::AuxElement* tob : *tobs) {
      if (!line.empty()) { line += ' '; }
      line += m_pack(*tob);
    }
    m_out << line << '\n';

    ++m_nextEvt;
    ++m_nWritten;

    ATH_MSG_INFO("event " << ctx.evt() << ": wrote " << tobs->size()
                 << " TOBs from '" << m_inKey.key() << "'");
    return StatusCode::SUCCESS;
  }

  StatusCode TOBTextWriter::finalize() {
    m_out.close();
    if (!m_out) {
      ATH_MSG_ERROR("Failed writing '" << m_outFile.value() << "'");
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("wrote " << m_nWritten << " events to '" << m_outFile.value() << "'");
    return StatusCode::SUCCESS;
  }

} // namespace GlobalSim
