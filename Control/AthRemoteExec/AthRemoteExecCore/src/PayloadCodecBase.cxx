/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <AthRemoteExecCore/PayloadCodecBase.h>
#include <AthenaKernel/ExtendedEventContext.h>
#include <AthenaKernel/IProxyDict.h>

namespace AthRemoteExec {

std::string describeDangling(const std::vector<std::string>& dangling) {
  std::string list;
  for (const std::string& entry : dangling) {
    list += (list.empty() ? "" : ", ") + entry;
  }
  return list;
}

StatusCode PayloadCodecBase::initialize() {
  if (m_key.empty()) {
    ATH_MSG_ERROR(
        "Key is not set; this codec serves one boundary and would not know "
        "which");
    return StatusCode::FAILURE;
  }

  // Set direction
  if (m_direction == "WriteToSG") {
    m_writing = true;
  } else if (m_direction == "ReadFromSG") {
    m_writing = false;
  } else {
    ATH_MSG_ERROR("Direction is '{}'; it must be 'ReadFromSG' or 'WriteToSG'",
                  m_direction.value());
    return StatusCode::FAILURE;
  }

  ATH_CHECK(initializeBoundary());

  ATH_MSG_DEBUG("Boundary '{}' on {}({}), {}", m_key.value(), encoding(),
                (m_schema.empty() ? "" : " as " + m_schema.value()),
                m_direction.value());

  return StatusCode::SUCCESS;
}

StatusCode PayloadCodecBase::decode(const EventContext& ctx,
                                    const Payload& payload) const {
  if (payload.encoding != encoding()) {
    ATH_MSG_ERROR(
        "'{}' arrived with encoding '{}' but this boundary declares '{}'",
        m_key.value(), payload.encoding, encoding());
    return StatusCode::FAILURE;
  }

  if (payload.schema != m_schema) {
    ATH_MSG_ERROR("''{}' arrived as '{}' but this boundary declares '{}'",
                  m_key.value(), payload.schema, m_schema.value());
    return StatusCode::FAILURE;
  }

  IProxyDict* store = storeOf(ctx);
  if (store == nullptr) {
    return StatusCode::FAILURE;
  }

  return doDecode(ctx, *store, payload);
}

StatusCode PayloadCodecBase::encode(
    const EventContext& ctx, Payload& payload,
    const std::vector<const IPayloadCodec*>& alsoCrossing,
    std::vector<std::string>& dangling) const {
  IProxyDict* store = storeOf(ctx);
  if (store == nullptr) {
    return StatusCode::FAILURE;
  }

  payload.key = m_key;
  payload.encoding = encoding();
  payload.schema = m_schema;
  return doEncode(ctx, *store, payload, alsoCrossing, dangling);
}

IProxyDict* PayloadCodecBase::storeOf(const EventContext& ctx) const {
  // From the context rather than from evtStore(), so this does not depend on
  // which slot happens to be selected on this thread.
  IProxyDict* store = Atlas::getExtendedEventContext(ctx).proxy();
  if (store == nullptr) {
    ATH_MSG_ERROR("No proxy dictionary in the event context");
  }
  return store;
}

}  // namespace AthRemoteExec
