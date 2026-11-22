/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "DemoWordsCodec.h"

#include <StoreGate/ReadHandle.h>
#include <StoreGate/WriteHandle.h>

#include <memory>

namespace AthRemoteExec::Example {

const std::string& DemoWordsCodec::encoding() const {
  // Return which RPC protocol we support
  static const std::string tag = "protobuf";
  return tag;
}

StatusCode DemoWordsCodec::initializeBoundary() {
  // Fail if we are configured to handle messages the we do not expect
  if (schema() != "athremoteexec.demo.v1.Words") {
    ATH_MSG_ERROR(
        "Schema is '{}'; this codec implements 'athremoteexec.demo.v1.Words'",
        schema());
    return StatusCode::FAILURE;
  }

  // Initialize handles
  ATH_CHECK(m_decoded.initialize(writingToSG()));
  ATH_CHECK(m_encoded.initialize(!writingToSG()));

  // Fail if incorrect keys are set for de/encoding
  const bool haveKeys = writingToSG() ? !m_decoded.empty() : !m_encoded.empty();
  if (!haveKeys) {
    ATH_MSG_ERROR(
        "Direction is '{}' but the matching key list is empty; this codec "
        "would carry nothing",
        writingToSG() ? "WriteToSG" : "ReadFromSG");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode DemoWordsCodec::doDecode(const EventContext& ctx,
                                    IProxyDict& /*store*/,
                                    const Payload& payload) const {
  // Check if received data makes sense, return failure otherwise
  if (payload.data.size() % sizeof(uint32_t) != 0) {
    ATH_MSG_ERROR("Received payload size ({} bytes) does not align to {} bytes",
                  payload.data.size(), sizeof(uint32_t));
    return StatusCode::FAILURE;
  }

  // Calculate how many words we received
  const size_t nWords = payload.data.size() / sizeof(uint32_t);

  // Allocate memory to hold these numbers
  std::vector<uint32_t> words(nWords);

  // Copy bytes from payload as words to vector
  std::memcpy(words.data(), payload.data.data(), nWords);

  // Create handle and record data to StoreGate
  SG::WriteHandle<std::vector<uint32_t>> wordsHandle(m_decoded, ctx);
  ATH_CHECK(wordsHandle.record(
      std::make_unique<std::vector<uint32_t>>(std::move(words))));

  ATH_MSG_INFO("Unpacked {} bytes into {} word(s) as {}", payload.data.size(),
               nWords, wordsHandle.key());

  return StatusCode::SUCCESS;
}

StatusCode DemoWordsCodec::doEncode(
    const EventContext& ctx, IProxyDict& /*store*/, Payload& payload,
    const std::vector<const IPayloadCodec*>& /*alsoCrossing*/,
    std::vector<std::string>& /*dangling*/) const {
  SG::ReadHandle<std::vector<uint32_t>> words(m_encoded, ctx);

  // Calculate how many bytes we need to send
  const size_t nBytes = words->size() * sizeof(uint32_t);

  // Clear data and allocate needed space
  payload.data.clear();
  payload.data.resize(nBytes);

  // Copy entire vector as bytes
  std::memcpy(payload.data.data(), reinterpret_cast<const char*>(words->data()),
              nBytes);

  ATH_MSG_INFO("Packed {} word(s) as {} bytes into '{}'", words->size(), nBytes,
               key());

  return StatusCode::SUCCESS;
}

}  // namespace AthRemoteExec::Example
