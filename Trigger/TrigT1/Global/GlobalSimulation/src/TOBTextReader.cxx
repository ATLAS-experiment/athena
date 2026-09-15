/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */
#include "TOBTextReader.h"

#include "SpecRegistry.h"

#include "xAODCore/AuxContainerBase.h"
#include "AthContainers/AuxElement.h"

#include <fstream>
#include <sstream>

namespace GlobalSim {

  StatusCode TOBTextReader::initialize() {
    ATH_CHECK(m_outKey.initialize());

    const auto entry = specRegistry().find(m_specName.value());
    if (entry == specRegistry().end()) {
      ATH_MSG_ERROR("Unknown BitSpec '" << m_specName.value()
                    << "'. Known specs are: " << knownSpecNames());
      return StatusCode::FAILURE;
    }
    m_width = entry->second.width;
    m_unpack = entry->second.unpack;

    // The whole file is read here so that execute() can stay const and
    // reentrant: after this point m_events is only ever read.
    std::ifstream in(m_inputFile.value());
    if (!in) {
      ATH_MSG_ERROR("Could not open input file '" << m_inputFile.value() << "'");
      return StatusCode::FAILURE;
    }

    const std::size_t expected = m_width / 4;
    std::string line;
    std::size_t lineNo{0};

    while (std::getline(in, line)) {
      ++lineNo;
      std::istringstream ss(line);
      std::string token;
      std::vector<std::string> tokens;

      while (ss >> token) {
        // Checked here rather than at decode time: a token of the wrong
        // length would be silently truncated by std::bitset, which is how a
        // file written for one spec gets read as another without complaint.
        if (token.size() != expected) {
          ATH_MSG_ERROR("line " << lineNo << ": token '" << token << "' has "
                        << token.size() << " hex characters, but "
                        << m_specName.value() << " needs " << expected);
          return StatusCode::FAILURE;
        }
        tokens.push_back(token);
      }

      if (!tokens.empty()) { m_events.push_back(std::move(tokens)); }
    }

    if (m_events.empty()) {
      ATH_MSG_ERROR("No events read from '" << m_inputFile.value() << "'");
      return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("Loaded " << m_events.size() << " events from '"
                 << m_inputFile.value() << "' as " << m_specName.value()
                 << ", " << expected << " hex characters per TOB");
    return StatusCode::SUCCESS;
  }

  StatusCode TOBTextReader::execute(const EventContext& ctx) const {
    // event N -> line N, wrapping round if the job runs more events than the
    // file has lines.
    const std::size_t idx = ctx.evt() % m_events.size();
    const std::vector<std::string>& tokens = m_events[idx];

    auto tobs = std::make_unique<xAOD::BaseContainer>();
    auto tobsAux = std::make_unique<xAOD::AuxContainerBase>();
    tobs->setStore(tobsAux.get());

    // Assigning the whole word fills every field the spec declares, so the
    // objects carry the flags as well as the kinematics.
    for (const std::string& token : tokens) {
      tobs->push_back(std::make_unique<SG::AuxElement>());
      m_unpack(*tobs->back(), token);
    }

    ATH_MSG_INFO("event " << ctx.evt() << ": produced " << tobs->size()
                 << " TOBs into '" << m_outKey.key() << "'");

    SG::WriteHandle<xAOD::BaseContainer> h(m_outKey, ctx);
    ATH_CHECK(h.record(std::move(tobs), std::move(tobsAux)));
    return StatusCode::SUCCESS;
  }

} // namespace GlobalSim
