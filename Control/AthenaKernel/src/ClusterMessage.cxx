/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "AthenaKernel/ClusterMessage.h"
#include <cstdint>

ClusterMessage::DataDescr::DataDescr(DataDescr&& rhs) noexcept
    : ptr(rhs.ptr),
      len(rhs.len),
      align(rhs.align),
      received(rhs.received),
      evtNumber(rhs.evtNumber),
      fileNumber(rhs.fileNumber) {
  rhs.ptr = nullptr;
  rhs.len = 0;
  rhs.align = 0;
  rhs.received = false;
}
ClusterMessage::DataDescr::DataDescr(const WireMsgBody& body)
    : ptr(reinterpret_cast<void*>((std::uint64_t(body[0]) << 32) +
                                  std::uint64_t(body[1]))),
      len((std::uint64_t(body[2]) << 32) + std::uint64_t(body[3])),
      align((std::uint64_t(body[4]) << 32) + std::uint64_t(body[5])),
      received(true),
      evtNumber((std::uint64_t(body[6]) << 32) + std::uint64_t(body[7])),
      fileNumber((std::uint64_t(body[8]) << 32) + std::uint64_t(body[9])) {}

ClusterMessage::DataDescr::~DataDescr() {
  if (received && (ptr != nullptr)) {
    std::free(ptr);
  }
}

ClusterMessage::DataDescr& ClusterMessage::DataDescr::operator=(
    DataDescr&& rhs) noexcept {
  ptr = rhs.ptr;
  len = rhs.len;
  align = rhs.align;
  received = rhs.received;
  evtNumber = rhs.evtNumber;
  fileNumber = rhs.fileNumber;
  rhs.ptr = nullptr;
  rhs.len = 0;
  rhs.align = 0;
  rhs.received = false;
  rhs.evtNumber = 0;
  rhs.fileNumber = 0;
  return *this;
}

ClusterMessage::ClusterMessage(ClusterMessageType mType, WorkerStatus payload)
    : messageType(mType), payload(payload) {
  if (mType != ClusterMessageType::FinalWorkerStatus &&
      mType != ClusterMessageType::WorkerError) {
    throw std::logic_error{std::format(
        "Incorrect ClusterMessage constructor used for message type {}",
        mType)};
  }
}

ClusterMessage::ClusterMessage(ClusterMessageType mType, int payload)
    : messageType(mType), payload(payload) {
  if (mType != ClusterMessageType::ProvideEvent) {
    throw std::logic_error{std::format(
        "Incorrect ClusterMessage constructor used for message type {}",
        mType)};
  }
}

ClusterMessage::ClusterMessage(ClusterMessageType mType, DataDescr&& payload)
    : messageType(mType), payload(std::move(payload)) {
  if (mType != ClusterMessageType::Data) {
    throw std::logic_error{std::format(
        "Incorrect ClusterMessage constructor used for message type {}",
        mType)};
  }
}

ClusterMessage::ClusterMessage(ClusterMessageType mType) : messageType(mType) {
  switch (mType) {
    case ClusterMessageType::RequestEvent:
    case ClusterMessageType::EventsDone:
    case ClusterMessageType::EmergencyStop:
      // OK
      break;
    default:
      throw std::logic_error{std::format(
          "Incorrect ClusterMessage constructor used for message type {}",
          mType)};
  }
}

ClusterMessage::ClusterMessage() = default;

ClusterMessage::ClusterMessage(const ClusterMessage::WireMsg& wire_msg) {
  const auto& [header, body] = wire_msg;
  messageType = static_cast<ClusterMessageType>(header[0]);
  source = header[1];
  if (body.has_value()) {
    const auto& body_2 = *body;
    if (messageType == ClusterMessageType::Data) {
      payload = std::move(DataDescr(body_2));
    } else {
      WorkerStatus status{};
      status.status = StatusCode(body_2[0]);
      status.createdEvents = body_2[1];
      status.finishedEvents = body_2[2];
      status.skippedEvents = body_2[3];
      payload = status;
    }
  } else {
    if (header[2] >= 0) {
      payload = header[2];
    }
  }
}

ClusterMessage::WireMsg ClusterMessage::wire_msg() const {
  constexpr int max_tag = 16383;
  constexpr std::uint64_t lower32 = 0xFFFFFFFF;

  static thread_local int next_msg =
      1;  // This is only ever called from one thread per process
  WireMsgHdr header{};
  header[0] = int(messageType);
  header[1] = source;
  if (payload.index() == 3) {
    next_msg = (next_msg % max_tag) + 1;
    header[2] = next_msg;
    WireMsgBody body{};
    const auto& payload_local = std::get<DataDescr>(payload);
    body[0] = int(std::uint64_t(payload_local.ptr) >> 32);
    body[1] = int(std::uint64_t(payload_local.ptr) & lower32);
    body[2] = int(std::uint64_t(payload_local.len) >> 32);
    body[3] = int(std::uint64_t(payload_local.len) & lower32);
    body[4] = int(std::uint64_t(payload_local.align) >> 32);
    body[5] = int(std::uint64_t(payload_local.align) & lower32);
    body[6] = int(std::uint64_t(payload_local.evtNumber) >> 32);
    body[7] = int(std::uint64_t(payload_local.evtNumber) & lower32);
    body[8] = int(std::uint64_t(payload_local.fileNumber) >> 32);
    body[9] = int(std::uint64_t(payload_local.fileNumber) & lower32);
    WireMsg msg{header, std::make_optional(body)};
    return msg;
  }
  if (payload.index() == 2) {
    next_msg = (next_msg % max_tag) + 1;
    header[2] = next_msg;
    WireMsgBody body{};
    const auto& payload_local = std::get<WorkerStatus>(payload);
    body[0] = static_cast<int>(payload_local.status.getCode());
    body[1] = payload_local.createdEvents;
    body[2] = payload_local.finishedEvents;
    body[3] = payload_local.skippedEvents;
    body[4] = body[5] = body[6] = body[7] = body[8] = body[9] = 0;
    WireMsg msg{header, std::make_optional(body)};
    return msg;
  }
  // else
  if (payload.index() == 1) {  // if we have an int payload
    header[2] = std::get<int>(payload);
  } else {
    header[2] = -1;
  }
  WireMsg msg{header, std::nullopt};
  return msg;
}
