/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <AthRemoteExecCore/OutputCnvAlg.h>

#include <memory>
#include <utility>

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

namespace AthRemoteExec {

StatusCode OutputCnvAlg::initialize() {
  if (m_reply.key().empty()) {
    ATH_MSG_ERROR(
        "Reply is not set; the loop manager would never find this sequence's "
        "result");
    return StatusCode::FAILURE;
  }

  ATH_CHECK(m_request.initialize());
  ATH_CHECK(m_reply.initialize());
  ATH_CHECK(m_encoders.retrieve());

  m_crossing.reserve(m_encoders.size());
  for (const ToolHandle<IPayloadCodec>& codec : m_encoders) {
    m_crossing.push_back(codec.get());
  }

  return StatusCode::SUCCESS;
}

StatusCode OutputCnvAlg::execute(const EventContext& ctx) const {
  SG::ReadHandle<RequestDescriptor> request(m_request, ctx);
  if (!request.isValid()) {
    ATH_MSG_ERROR("No request under '{}'", m_request.key());
    return StatusCode::FAILURE;
  }

  ExecuteReply reply;
  reply.status = AthRemoteExec::Status::Ok;
  reply.requestId = request->requestId();

  if (!m_encoders.empty()) {
    reply.outputs.reserve(m_encoders.size());
    std::vector<std::string> dangling;
    for (const ToolHandle<IPayloadCodec>& codec : m_encoders) {
      Payload payload;
      ATH_CHECK(codec->encode(ctx, payload, m_crossing, dangling));
      reply.outputs.push_back(std::move(payload));
    }

    // See RequestAlg for why this is a warning reported once rather
    // than an error: a reply may legitimately carry references nobody is meant
    // to follow, and the client cannot tell that case from a forgotten output.
    if (!dangling.empty() &&
        !m_warnedDangling.exchange(true, std::memory_order_relaxed)) {
      ATH_MSG_WARNING("This reply refers to keys it does not carry: "
                      << describeDangling(dangling)
                      << ". They will not resolve on the client. Add them to "
                         "this fragment's outputs, or ignore this if they are "
                         "not meant to be followed there. Reported once");
    }
  }

  SG::WriteHandle<ReplyStaging> staged(m_reply, ctx);
  // The loop manager is the sole reader and moves the payload out of this
  // object rather than copying it, which needs a non-const proxy. record()
  // locks the object, and the retrieval then fails correcly with "No valid
  // proxy".
  ATH_CHECK(staged.recordNonConst(
      std::make_unique<ReplyStaging>(std::move(reply))));
  ATH_MSG_DEBUG("Staged reply for request {} on slot {}", request->requestId(),
                ctx.slot());
  return StatusCode::SUCCESS;
}

}  // namespace AthRemoteExec
