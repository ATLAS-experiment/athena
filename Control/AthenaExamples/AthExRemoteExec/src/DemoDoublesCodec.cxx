/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "DemoDoublesCodec.h"

#include <StoreGate/ReadHandle.h>
#include <StoreGate/WriteHandle.h>

#include <memory>

#include "DemoSchema.h"

namespace AthRemoteExec::Example {

const std::string& DemoDoublesCodec::encoding() const {
  static const std::string tag = "protobuf";
  return tag;
}

StatusCode DemoDoublesCodec::initializeBoundary() {
  if (schema() != DemoSchema::doubles) {
    ATH_MSG_ERROR("Schema is '{}'; this codec implements '{}'", schema(),
                  DemoSchema::doubles);
    return StatusCode::FAILURE;
  }

  ATH_CHECK(m_decoded.initialize(writingToSG()));
  ATH_CHECK(m_encoded.initialize(!writingToSG()));

  const bool haveKey =
      writingToSG() ? !m_decoded.key().empty() : !m_encoded.key().empty();
  if (!haveKey) {
    ATH_MSG_ERROR(
        "Direction is '{}' but the matching key is not set; this codec would "
        "carry nothing",
        writingToSG() ? "WriteToSG" : "ReadFromSG");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode DemoDoublesCodec::doDecode(const EventContext& ctx,
                                      IProxyDict& /*store*/,
                                      const Payload& payload) const {
  auto values = std::make_unique<std::vector<double>>();
  std::string name;
  std::string error;
  if (!DemoSchema::decodeDoubles(payload.data, name, *values, error)) {
    ATH_MSG_ERROR("'{}': {}", key(), error);
    return StatusCode::FAILURE;
  }

  SG::WriteHandle<std::vector<double>> handle(m_decoded, ctx);
  ATH_MSG_DEBUG("Unpacked {} value(s) named '{}' into '{}'", values->size(),
                name, handle.key());
  ATH_CHECK(handle.record(std::move(values)));
  return StatusCode::SUCCESS;
}

StatusCode DemoDoublesCodec::doEncode(
    const EventContext& ctx, IProxyDict& /*store*/, Payload& payload,
    const std::vector<const IPayloadCodec*>& /*alsoCrossing*/,
    std::vector<std::string>& /*dangling*/) const {
  SG::ReadHandle<std::vector<double>> values(m_encoded, ctx);
  if (!values.isValid()) {
    ATH_MSG_ERROR("'{}': nothing under '{}'", key(), m_encoded.key());
    return StatusCode::FAILURE;
  }

  std::string error;
  if (!DemoSchema::encodeDoubles(m_encoded.key(), *values, payload.data,
                                 error)) {
    ATH_MSG_ERROR("'{}': {}", key(), error);
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Packed {} value(s) into '{}'", values->size(), key());
  return StatusCode::SUCCESS;
}

}  // namespace AthRemoteExec::Example
