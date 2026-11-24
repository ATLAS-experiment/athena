/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "test_bytestream_utils.h"

#include <CxxUtils/StringUtils.h>
#include <SGTools/BuiltinsClids.h>
#include <StoreGate/ReadHandle.h>
#include <StoreGate/WriteHandle.h>

#include <algorithm>
#include <charconv>
#include <iterator>

namespace AthRemoteExec {

namespace {

/// FNV-1a over the raw bytes of @c count words
uint32_t digest(const uint32_t* words, size_t count) {
  const unsigned char* bytes = reinterpret_cast<const unsigned char*>(words);
  uint32_t hash = 2166136261u;
  for (size_t i = 0; i < count * sizeof(uint32_t); ++i) {
    hash ^= bytes[i];
    hash *= 16777619u;
  }
  return hash;
}

}  // anonymous namespace

StatusCode test_NamedIntsCodec::initializeBoundary() {
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

StatusCode test_NamedIntsCodec::doDecode(const EventContext& ctx,
                                         IProxyDict& /*store*/,
                                         const Payload& payload) const {
  if (!payload.data.empty() && !payload.data.contains(':')) {
    ATH_MSG_ERROR("Invalid message: {}", payload.data);
    return StatusCode::FAILURE;
  }

  const std::vector<std::string> tokens = CxxUtils::tokenize(payload.data, '#');
  std::vector<std::pair<std::string, uint32_t>> pairs;
  pairs.reserve(tokens.size());
  for (const std::string& token : tokens) {
    const std::vector<std::string> split = CxxUtils::tokenize(token, ':');
    if (split.size() != 2) {
      ATH_MSG_ERROR("Invalid message token: {}", token);
      return StatusCode::FAILURE;
    }

    uint32_t value = 0;
    const auto [ptr, ec] = std::from_chars(
        split[1].data(), split[1].data() + split[1].size(), value);
    if (ec != std::errc{} || ptr != split[1].data() + split[1].size()) {
      ATH_MSG_ERROR("Invalid numeric string: {}", split[1]);
      return StatusCode::FAILURE;
    }

    pairs.emplace_back(split[0], value);
  }

  for (SG::WriteHandle<uint32_t>& handle : m_outputInts.makeHandles(ctx)) {
    const auto found = std::find_if(
        pairs.begin(), pairs.end(),
        [&handle](const std::pair<std::string, uint32_t>& candidate) {
          return candidate.first == handle.key();
        });
    if (found == pairs.end()) {
      ATH_MSG_ERROR("'{}' carries no entry named '{}'", key(), handle.key());
      return StatusCode::FAILURE;
    }

    ATH_CHECK(handle.record(std::make_unique<uint32_t>(found->second)));
    ATH_MSG_INFO("Unpacked {} = {}", handle.key(), found->second);
  }

  return StatusCode::SUCCESS;
}

StatusCode test_NamedIntsCodec::doEncode(
    const EventContext& ctx, IProxyDict& /*store*/, Payload& payload,
    const std::vector<const IPayloadCodec*>& /*alsoCrossing*/,
    std::vector<std::string>& /*dangling*/) const {
  // Encode named ints as <name>:<value> pairs separated by '#'

  payload.data.clear();
  for (SG::ReadHandle<uint32_t>& handle : m_inputInts.makeHandles(ctx)) {
    if (!handle.isValid()) {
      ATH_MSG_ERROR("'{}': nothing under '{}'", key(), handle.key());
      return StatusCode::FAILURE;
    }
    payload.data += std::format("{}:{}#", handle.key(), *handle);
  }

  if (m_inputInts.size() > 0) {
    payload.data.pop_back();
  }

  ATH_MSG_INFO("Packed {} integer(s) into '{}'", m_inputInts.size(), key());

  return StatusCode::SUCCESS;
}

StatusCode test_RawEventDigestAlg::initialize() {
  ATH_CHECK(m_cache.initialize());
  ATH_CHECK(m_size.initialize());
  ATH_CHECK(m_hash.initialize());

  return StatusCode::SUCCESS;
}

StatusCode test_RawEventDigestAlg::execute(const EventContext& ctx) const {
  SG::ReadHandle<RawEventCache> cache(m_cache, ctx);
  if (!cache.isValid()) {
    ATH_MSG_ERROR("Nothing under '{}'; the bytestream payload did not decode",
                  m_cache.key());
    return StatusCode::FAILURE;
  }

  const std::vector<uint32_t>& words = cache->words();
  const uint32_t hash = digest(words.data(), words.size());

  SG::WriteHandle<uint32_t> size(m_size, ctx);
  ATH_CHECK(size.record(
      std::make_unique<uint32_t>(static_cast<uint32_t>(words.size()))));
  SG::WriteHandle<uint32_t> digestHandle(m_hash, ctx);
  ATH_CHECK(digestHandle.record(std::make_unique<uint32_t>(hash)));

  ATH_MSG_INFO("Digested {} words of event {} into 0x{:08x}", words.size(),
               ctx.eventID().event_number(), hash);

  return StatusCode::SUCCESS;
}

StatusCode test_RawEventVerifyAlg::initialize() {
  ATH_CHECK(m_size.initialize());
  ATH_CHECK(m_hash.initialize());
  ATH_CHECK(m_robsSvc.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode test_RawEventVerifyAlg::execute(const EventContext& ctx) const {
  SG::ReadHandle<uint32_t> size(m_size, ctx);
  SG::ReadHandle<uint32_t> hash(m_hash, ctx);
  if (!size.isValid() || !hash.isValid()) {
    ATH_MSG_ERROR("Nothing under '{}'/'{}'; the reply did not decode",
                  m_size.key(), m_hash.key());
    return StatusCode::FAILURE;
  }

  const RawEvent* original = m_robsSvc->getEvent(ctx);
  if (original == nullptr) {
    ATH_MSG_ERROR("The bytestream input installed no event for slot {}",
                  ctx.slot());
    return StatusCode::FAILURE;
  }

  const size_t nWords = std::distance(original->start(), original->end());
  if (*size != nWords) {
    ATH_MSG_ERROR("Event {} has {} words but {} came back",
                  ctx.eventID().event_number(), nWords, *size);
    return StatusCode::FAILURE;
  }

  const uint32_t expected = digest(original->start(), nWords);
  if (*hash != expected) {
    ATH_MSG_ERROR("Event {} digest is 0x{:08x} but 0x{:08x} came back",
                  ctx.eventID().event_number(), expected, *hash);
    return StatusCode::FAILURE;
  }

  ++m_checked;
  ATH_MSG_INFO("Event {}: {} words, digest 0x{:08x} verified",
               ctx.eventID().event_number(), nWords, expected);

  return StatusCode::SUCCESS;
}

StatusCode test_RawEventVerifyAlg::finalize() {
  ATH_MSG_INFO("Checked {} reply/replies", m_checked.load());
  if (m_checked == 0) {
    ATH_MSG_ERROR("No replies were checked");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

}  // namespace AthRemoteExec

DECLARE_COMPONENT(AthRemoteExec::test_NamedIntsCodec)
DECLARE_COMPONENT(AthRemoteExec::test_RawEventDigestAlg)
DECLARE_COMPONENT(AthRemoteExec::test_RawEventVerifyAlg)
