/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "DemoIntsCodec.h"

#include <StoreGate/ReadHandle.h>
#include <StoreGate/WriteHandle.h>

#include <algorithm>
#include <memory>

#include "DemoSchema.h"

namespace AthRemoteExec::Example {

const std::string& DemoIntsCodec::encoding() const {
  static const std::string tag = "protobuf";
  return tag;
}

StatusCode DemoIntsCodec::initializeBoundary() {
  if (schema() != DemoSchema::ints) {
    ATH_MSG_ERROR("Schema is '{}'; this codec implements '{}'", schema(),
                  DemoSchema::ints);
    return StatusCode::FAILURE;
  }

  ATH_CHECK(m_decoded.initialize());
  ATH_CHECK(m_encoded.initialize());

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

StatusCode DemoIntsCodec::doDecode(const EventContext& ctx,
                                   IProxyDict& /*store*/,
                                   const Payload& payload) const {
  std::vector<DemoSchema::NamedInt> values;
  std::string error;
  if (!DemoSchema::decodeInts(payload.data, values, error)) {
    ATH_MSG_ERROR("'{}': {}", key(), error);
    return StatusCode::FAILURE;
  }

  for (SG::WriteHandle<int64_t>& handle : m_decoded.makeHandles(ctx)) {
    const auto found =
        std::find_if(values.begin(), values.end(),
                     [&handle](const DemoSchema::NamedInt& candidate) {
                       return candidate.name == handle.key();
                     });
    if (found == values.end()) {
      ATH_MSG_ERROR("'{}' carries no entry named '{}'", key(), handle.key());
      return StatusCode::FAILURE;
    }
    ATH_CHECK(handle.record(std::make_unique<int64_t>(found->value)));
    ATH_MSG_DEBUG("Unpacked {} = {}", handle.key(), found->value);
  }
  return StatusCode::SUCCESS;
}

StatusCode DemoIntsCodec::doEncode(
    const EventContext& ctx, IProxyDict& /*store*/, Payload& payload,
    const std::vector<const IPayloadCodec*>& /*alsoCrossing*/,
    std::vector<std::string>& /*dangling*/) const {
  std::vector<DemoSchema::NamedInt> values;
  for (SG::ReadHandle<int64_t>& handle : m_encoded.makeHandles(ctx)) {
    if (!handle.isValid()) {
      ATH_MSG_ERROR("'{}': nothing under '{}'", key(), handle.key());
      return StatusCode::FAILURE;
    }
    values.push_back(DemoSchema::NamedInt{handle.key(), *handle});
  }

  std::string error;
  if (!DemoSchema::encodeInts(values, payload.data, error)) {
    ATH_MSG_ERROR("'{}': {}", key(), error);
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Packed {} integer(s) into '{}'", values.size(), key());
  return StatusCode::SUCCESS;
}

}  // namespace AthRemoteExec::Example
