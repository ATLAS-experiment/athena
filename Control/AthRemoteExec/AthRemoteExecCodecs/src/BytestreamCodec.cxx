/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <AthRemoteExecCodecs/Bytestream.h>
#include <AthRemoteExecCodecs/BytestreamCodec.h>
#include <StoreGate/WriteHandle.h>

#include <string>
#include <utility>
#include <vector>

namespace AthRemoteExec {
const std::string BytestreamCodec::s_encoding = "bytestream";

StatusCode BytestreamCodec::initializeBoundary() {
  ATH_CHECK(m_decoded.assign(key()));
  ATH_CHECK(m_decoded.initialize(writingToSG()));

  ATH_CHECK(m_robsSvc.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode BytestreamCodec::doDecode(const EventContext& ctx, IProxyDict& /*store*/, const Payload& payload) const {
  auto cache = Bytestream::fromBytes(payload.data);
  if (!cache) {
    ATH_MSG_ERROR("{}: {}", key(), cache.error().diagnostics);
    return StatusCode::FAILURE;
  }

  SG::WriteHandle<RawEventCache> handle(m_decoded, ctx);
  ATH_CHECK(handle.record(std::move(*cache)));

  m_robsSvc->setNextEvent(ctx, handle->event());

  ATH_MSG_DEBUG("Decoded RawEvent data ({} bytes) and called setNextEvent()", payload.data.size());

  return StatusCode::SUCCESS;
}

StatusCode BytestreamCodec::doEncode(const EventContext& ctx, IProxyDict& /*store*/, Payload& payload,
                                     const std::vector<const IPayloadCodec*>& /*alsoCrossing*/,
                                     std::vector<std::string>& /*dangling*/) const {
  auto bytes = Bytestream::toBytes(m_robsSvc->getEvent(ctx));
  if (!bytes) {
    ATH_MSG_ERROR("{}: {}", key(), bytes.error().diagnostics);
    return StatusCode::FAILURE;
  }

  payload.data = std::move(*bytes);

  ATH_MSG_DEBUG("Encoded {} bytes of RawEvent data", payload.data.size());

  return StatusCode::SUCCESS;
}

}  // namespace AthRemoteExec
