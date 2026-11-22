/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <AthRemoteExecCore/InputCnvAlg.h>

#include "StoreGate/ReadHandle.h"

namespace AthRemoteExec {

StatusCode InputCnvAlg::initialize() {
  if (m_sequence.empty()) {
    ATH_MSG_ERROR(
        "SequenceName is not set; this gate would never select anything");
    return StatusCode::FAILURE;
  }

  ATH_CHECK(m_request.initialize());
  ATH_CHECK(m_decoders.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode InputCnvAlg::execute(const EventContext& ctx) const {
  SG::ReadHandle<RequestDescriptor> request(m_request, ctx);
  if (!request.isValid()) {
    ATH_MSG_ERROR(
        "No request under '{}'; the loop manager should have recorded one",
        m_request.key());
    return StatusCode::FAILURE;
  }

  if (request->sequence() != m_sequence) {
    // This request is not for us
    setFilterPassed(false, ctx);
    return StatusCode::SUCCESS;
  }

  setFilterPassed(true, ctx);
  ATH_MSG_DEBUG("Selected by request {} on slot {}", request->requestId(),
                ctx.slot());

  if (m_decoders.empty()) {
    return StatusCode::SUCCESS;
  }

  for (const ToolHandle<IPayloadCodec>& codec : m_decoders) {
    const Payload* payload = request->find(codec->key());
    if (payload == nullptr) {
      ATH_MSG_ERROR("Request {} for '{}' does not supply '{}'",
                    request->requestId(), request->sequence(), codec->key());
      return StatusCode::FAILURE;
    }
    ATH_CHECK(codec->decode(ctx, *payload));
    ATH_MSG_DEBUG("Unpacked {} into '{}' ", describe(*payload), codec->key());
  }

  return StatusCode::SUCCESS;
}

}  // namespace AthRemoteExec
