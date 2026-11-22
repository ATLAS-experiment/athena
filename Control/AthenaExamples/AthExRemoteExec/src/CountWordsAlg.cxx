/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CountWordsAlg.h"

#include <SGTools/BuiltinsClids.h>
#include <StoreGate/ReadHandle.h>
#include <StoreGate/WriteHandle.h>

namespace AthRemoteExec::Example {

StatusCode CountWordsAlg::initialize() {
  // Initialize handles
  ATH_CHECK(m_words.initialize());
  ATH_CHECK(m_count.initialize());

  return StatusCode::SUCCESS;
}

StatusCode CountWordsAlg::execute(const EventContext& ctx) const {
  // Read vector from the SG
  SG::ReadHandle<std::vector<uint32_t>> words(m_words, ctx);

  // Check if input that we received is valid
  if (!words.isValid()) {
    ATH_MSG_ERROR("Missing input: {} must be present", m_words.key());
    return StatusCode::FAILURE;
  }

  // Create write handle
  SG::WriteHandle<int64_t> count(m_count, ctx);
  ATH_CHECK(count.record(std::make_unique<int64_t>(words->size())));

  ATH_MSG_INFO("{} has {} element(s)", m_words.key(), words->size());

  return StatusCode::SUCCESS;
}

}  // namespace AthRemoteExec::Example
