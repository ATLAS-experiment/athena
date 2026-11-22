/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "SumAlg.h"

#include <SGTools/BuiltinsClids.h>
#include <StoreGate/ReadHandle.h>
#include <StoreGate/WriteHandle.h>

namespace AthRemoteExec::Example {

StatusCode SumAlg::initialize() {
  ATH_CHECK(m_a.initialize());
  ATH_CHECK(m_b.initialize());
  ATH_CHECK(m_sum.initialize());
  return StatusCode::SUCCESS;
}

StatusCode SumAlg::execute(const EventContext& ctx) const {
  SG::ReadHandle<int64_t> a(m_a, ctx);
  SG::ReadHandle<int64_t> b(m_b, ctx);
  if (!a.isValid() || !b.isValid()) {
    ATH_MSG_ERROR("Missing input: {} and {} must both be present", m_a.key(),
                  m_b.key());
    return StatusCode::FAILURE;
  }

  SG::WriteHandle<int64_t> sum(m_sum, ctx);
  ATH_CHECK(sum.record(std::make_unique<int64_t>(*a + *b)));
  ATH_MSG_DEBUG("{} + {} = {}", *a, *b, *sum);
  return StatusCode::SUCCESS;
}

}  // namespace AthRemoteExec::Example
