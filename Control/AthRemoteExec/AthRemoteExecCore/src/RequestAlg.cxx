/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <AthRemoteExecCore/RequestAlg.h>

#include <chrono>
#include <fstream>
#include <thread>

namespace AthRemoteExec {

namespace {

/// For port file, wait until it appears and return host:port or return empty
/// string when deadline expires.
std::string resolveTarget(const std::string& target, double timeoutSeconds) {
  if (target.contains(':')) {
    return target;
  }

  const auto deadline =
      std::chrono::steady_clock::now() +
      std::chrono::milliseconds(static_cast<long>(timeoutSeconds * 1000));
  do {
    std::ifstream in(target);
    int port = 0;
    if (in >> port && port > 0) {
      //coverity[TAINTED_SCALAR]
      return "localhost:" + std::to_string(port);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  } while (std::chrono::steady_clock::now() < deadline);

  return {};
}

}  // anonymous namespace

StatusCode RequestAlg::initialize() {
  if (m_sequence.empty()) {
    ATH_MSG_ERROR("SequenceName is not set; there is nothing to ask for");
    return StatusCode::FAILURE;
  }

  if (m_target.empty()) {
    ATH_MSG_ERROR("Target is not set");
    return StatusCode::FAILURE;
  }

  ATH_CHECK(m_inputs.retrieve());
  ATH_CHECK(m_outputs.retrieve());

  m_crossing.reserve(m_inputs.size());
  for (const ToolHandle<IPayloadCodec>& codec : m_inputs) {
    m_crossing.push_back(codec.get());
  }

  const std::string target = resolveTarget(m_target.value(), m_readyTimeout);
  if (target.empty()) {
    ATH_MSG_ERROR("No server port appeared in '{}' within {} s",
                  m_target.value(), m_readyTimeout.value());
    return StatusCode::FAILURE;
  }

  m_client = std::make_unique<Client>(target);
  if (!m_client->waitUntilReady(m_readyTimeout)) {
    ATH_MSG_ERROR("Server at {} did not answer within {} s", target,
                  m_readyTimeout.value());
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("Will call '{}' at {}", m_sequence.value(), target);

  return StatusCode::SUCCESS;
}

StatusCode RequestAlg::execute(const EventContext& ctx) const {
  ExecuteRequest request;
  request.sequence = m_sequence;
  request.requestId = ctx.evt();

  const EventIDBase& id = ctx.eventID();
  request.eventId.runNumber = id.run_number();
  request.eventId.eventNumber = id.event_number();
  request.eventId.lumiBlock = id.lumi_block();
  request.eventId.timeStamp = id.time_stamp();
  request.eventId.timeStampNsOffset = id.time_stamp_ns_offset();
  request.eventId.bunchCrossingId = id.bunch_crossing_id();

  request.inputs.reserve(m_inputs.size());
  std::vector<std::string> dangling;
  for (const ToolHandle<IPayloadCodec>& codec : m_inputs) {
    Payload payload;
    ATH_CHECK(codec->encode(ctx, payload, m_crossing, dangling));
    ATH_MSG_DEBUG("Sending " << describe(payload));
    request.inputs.push_back(std::move(payload));
  }

  // Once, because it is a property of the configuration rather than of the
  // event: the same references point at the same missing key on every one. Not
  // an error, because a reference that is not meant to be followed on the far
  // side is a legitimate thing to send -- but a fragment that meant to carry
  // the target and did not gets no other signal at all, since an unresolvable
  // reference reads downstream as an absence.
  if (!dangling.empty() &&
      !m_warnedDangling.exchange(true, std::memory_order_relaxed)) {
    ATH_MSG_WARNING("This request refers to keys it does not carry: "
                    << describeDangling(dangling)
                    << ". They will not resolve on the server. Declare them "
                       "as inputs too, or ignore this if they are not meant "
                       "to be followed there. Reported once");
  }

  const ExecuteReply reply = m_client->execute(request);

  if (reply.status != AthRemoteExec::Status::Ok) {
    ATH_MSG_ERROR(
        "{} {} for '{}': {} -- {}",
        isRefusal(reply.status) ? "Server refused request" : "Request",
        request.requestId, request.sequence, toString(reply.status),
        reply.diagnostics);
    return StatusCode::FAILURE;
  }

  for (const ToolHandle<IPayloadCodec>& codec : m_outputs) {
    const Payload* payload = nullptr;
    for (const Payload& candidate : reply.outputs) {
      if (candidate.key == codec->key()) {
        payload = &candidate;
        break;
      }
    }

    if (payload == nullptr) {
      ATH_MSG_ERROR("Reply to request {} does not carry '{}'", reply.requestId,
                    codec->key());
      return StatusCode::FAILURE;
    }

    ATH_CHECK(codec->decode(ctx, *payload));
    ATH_MSG_DEBUG("Received {} into '{}' ", describe(*payload), codec->key());
  }

  return StatusCode::SUCCESS;
}

}  // namespace AthRemoteExec
