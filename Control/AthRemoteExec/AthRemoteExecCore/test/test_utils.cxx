/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "test_utils.h"

#include <CxxUtils/StringUtils.h>
#include <SGTools/BuiltinsClids.h>
#include <StoreGate/ReadHandle.h>
#include <StoreGate/WriteHandle.h>

namespace AthRemoteExec {

StatusCode test_IntsCodec::initializeBoundary() {
  if (schema() != "test_ints") {
    ATH_MSG_ERROR("Schema is '{}'; this codec implements 'test_ints'",
                  schema());
    return StatusCode::FAILURE;
  }

  ATH_CHECK(m_inputInts.initialize());
  ATH_CHECK(m_outputInts.initialize());

  const bool haveKeys =
      writingToSG() ? !m_outputInts.empty() : !m_inputInts.empty();
  if (!haveKeys) {
    ATH_MSG_ERROR(
        "Direction is '{}' but the matching key list is empty; this codec "
        "would carry nothing",
        writingToSG() ? "WriteToSG" : "ReadFromSG");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode test_IntsCodec::doDecode(const EventContext& ctx,
                                    IProxyDict& /*store*/,
                                    const Payload& payload) const {
  if (!payload.data.empty() && !payload.data.contains(':')) {
    ATH_MSG_ERROR("Invalid message: {}", payload.data);
    return StatusCode::FAILURE;
  }

  const std::vector<std::string> tokens = CxxUtils::tokenize(payload.data, '#');
  std::vector<std::pair<std::string, int32_t>> pairs;
  std::transform(tokens.begin(), tokens.end(), std::back_inserter(pairs),
                 [](std::string_view token) -> std::pair<std::string, int32_t> {
                   const std::vector<std::string> split =
                       CxxUtils::tokenize(token, ':');
                   return {split[0], CxxUtils::atoi(split[1])};
                 });

  for (SG::WriteHandle<int32_t>& handle : m_outputInts.makeHandles(ctx)) {
    const auto found = std::find_if(
        pairs.begin(), pairs.end(),
        [&handle](const std::pair<std::string, int32_t>& candidate) {
          return candidate.first == handle.key();
        });
    if (found == pairs.end()) {
      ATH_MSG_ERROR("'{}' carries no entry named '{}'", key(), handle.key());
      return StatusCode::FAILURE;
    }
    ATH_CHECK(handle.record(std::make_unique<int32_t>(found->second)));
    ATH_MSG_INFO("Unpacked {} = {}", handle.key(), found->second);
  }

  return StatusCode::SUCCESS;
}

StatusCode test_IntsCodec::doEncode(
    const EventContext& ctx, IProxyDict& /*store*/, Payload& payload,
    const std::vector<const IPayloadCodec*>& /*alsoCrossing*/,
    std::vector<std::string>& /*dangling*/) const {
  // Encode named ints as <name>:<value> pairs separated by '#'

  payload.data.clear();
  for (SG::ReadHandle<int32_t>& handle : m_inputInts.makeHandles(ctx)) {
    if (!handle.isValid()) {
      ATH_MSG_ERROR("'{}': nothing under '{}'", key(), handle.key());
      return StatusCode::FAILURE;
    }
    payload.data += std::format("{}:{}#", handle.key(), *handle);
    ATH_MSG_INFO("Buffer: {}", payload.data);
  }

  if (m_inputInts.size() > 0) {
    payload.data.pop_back();
  }

  ATH_MSG_INFO("Packed {} integer(s) into '{}'", m_inputInts.size(), key());
  ATH_MSG_INFO("Buffer: {}", payload.data);

  return StatusCode::SUCCESS;
}

StatusCode test_SumAlg::initialize() {
  ATH_CHECK(m_a.initialize());
  ATH_CHECK(m_b.initialize());
  ATH_CHECK(m_sum.initialize());
  return StatusCode::SUCCESS;
}

StatusCode test_SumAlg::execute(const EventContext& ctx) const {
  SG::ReadHandle<int32_t> a(m_a, ctx);
  SG::ReadHandle<int32_t> b(m_b, ctx);
  if (!a.isValid() || !b.isValid()) {
    ATH_MSG_ERROR("Missing input: {} and {} must both be present", m_a.key(),
                  m_b.key());
    return StatusCode::FAILURE;
  }

  SG::WriteHandle<int32_t> sum(m_sum, ctx);
  ATH_CHECK(sum.record(std::make_unique<int32_t>(*a + *b)));
  ATH_MSG_INFO("{} + {} = {}", *a, *b, *sum);

  return StatusCode::SUCCESS;
}
StatusCode test_NumbersAlg::initialize() {
  ATH_CHECK(m_values.initialize());

  if (m_values.size() != m_offsets.size()) {
    ATH_MSG_ERROR("Values has {} entries and Offsets {}", m_values.size(),
                  m_offsets.size());
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode test_NumbersAlg::execute(const EventContext& ctx) const {
  const int32_t event = static_cast<int32_t>(ctx.eventID().event_number());
  size_t index = 0;

  for (SG::WriteHandle<int32_t>& handle : m_values.makeHandles(ctx)) {
    ATH_CHECK(
        handle.record(std::make_unique<int32_t>(m_offsets[index] + event)));
    ATH_MSG_INFO("Wrote {} as '{}'", *handle, handle.name());
    ++index;
  }

  return StatusCode::SUCCESS;
}

StatusCode test_CheckAlg::initialize() {
  ATH_CHECK(m_values.initialize());
  ATH_CHECK(m_result.initialize());
  return StatusCode::SUCCESS;
}

StatusCode test_CheckAlg::execute(const EventContext& ctx) const {
  int32_t expected = m_offset;
  for (SG::ReadHandle<int32_t>& handle : m_values.makeHandles(ctx)) {
    if (!handle.isValid()) {
      ATH_MSG_ERROR("Nothing under {}", handle.key());
      return StatusCode::FAILURE;
    }
    expected += *handle;
  }

  SG::ReadHandle<int32_t> result(m_result, ctx);
  if (!result.isValid()) {
    ATH_MSG_ERROR("Nothing under '{}'; the reply did not decode",
                  m_result.key());
    return StatusCode::FAILURE;
  }
  if (*result != expected) {
    ATH_MSG_ERROR("'{}' is {}, expected {} for event {}", m_result.key(),
                  *result, expected, ctx.eventID().event_number());
    return StatusCode::FAILURE;
  }
  ++m_checked;
  return StatusCode::SUCCESS;
}

StatusCode test_CheckAlg::finalize() {
  ATH_MSG_INFO("Checked {} reply/replies", m_checked.load());
  if (m_checked == 0) {
    ATH_MSG_ERROR("No replies were checked");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode test_FailAlg::execute(const EventContext&) const {
  ATH_MSG_ERROR("Failing on purpose");

  return StatusCode::FAILURE;
}

StatusCode test_DelayAlg::execute(const EventContext& ctx) const {
  if (m_milliseconds > 0) {
    ATH_MSG_INFO("Sleeping " << m_milliseconds.value() << " ms on slot "
                             << ctx.slot());
    std::this_thread::sleep_for(std::chrono::milliseconds(m_milliseconds));
  }
  return StatusCode::SUCCESS;
}

}  // namespace AthRemoteExec

DECLARE_COMPONENT(AthRemoteExec::test_IntsCodec)
DECLARE_COMPONENT(AthRemoteExec::test_SumAlg)
DECLARE_COMPONENT(AthRemoteExec::test_NumbersAlg)
DECLARE_COMPONENT(AthRemoteExec::test_CheckAlg)
DECLARE_COMPONENT(AthRemoteExec::test_FailAlg)
DECLARE_COMPONENT(AthRemoteExec::test_DelayAlg)
