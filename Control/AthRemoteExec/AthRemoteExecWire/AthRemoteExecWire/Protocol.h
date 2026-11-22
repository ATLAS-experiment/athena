/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHREMOTEEXECWIRE_REMOTEEXECWIRE_H
#define ATHREMOTEEXECWIRE_REMOTEEXECWIRE_H

/**
 * @file Protocol.h
 * @brief Plain C++ interfaces of RPC wire.
 *
 * Plain C++ interfaces for objects used for RPC communication. Nothing
 * here is aware of any RPC framework. Conversion between these C++ intefraces
 * and given RPC framework must be implemented separately.
 */

#include <CxxUtils/StringUtils.h>

#include <cstdint>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace AthRemoteExec {

/// RPC reply status.
///
/// The first six fields are the possible replies to the request while last two
/// fields are related to server failures/refusal.
enum class Status : int {
  Unspecified = 0,
  Ok = 1,
  UnknownSequence = 2,
  InvalidRequest = 3,
  AlgFailure = 4,
  Timeout = 5,
  Overloaded = 6,
  ShuttingDown = 7,
};

/// Check if server refused to process the request
constexpr bool isRefusal(Status status) {
  return status == Status::Overloaded || status == Status::ShuttingDown;
}

/// String representation of Status enum
[[nodiscard]] constexpr std::string_view toString(Status status) {
  switch (status) {
    case Status::Unspecified:
      return "UNSPECIFIED";
    case Status::Ok:
      return "OK";
    case Status::UnknownSequence:
      return "UNKNOWN_SEQUENCE";
    case Status::InvalidRequest:
      return "INVALID_REQUEST";
    case Status::AlgFailure:
      return "ALG_FAILURE";
    case Status::Timeout:
      return "TIMEOUT";
    case Status::Overloaded:
      return "OVERLOADED";
    case Status::ShuttingDown:
      return "SHUTTING_DOWN";
  }
  return "UNSPECIFIED";
}

/// The actual data that is sent via RPC
struct Payload {
  std::string key;       ///< StoreGate key
  std::string encoding;  ///< serialisation mechanism; see IPayloadCodec.h
  std::string schema;    ///< what the bytes are, in that mechanism's terms
  std::string data;      ///< binary data
};

/// Human-readable rendering of a payload, for log messages.
inline std::string describe(const Payload& payload) {
  static constexpr size_t maxShown = 16;
  std::ostringstream out;
  out << std::format("{} [{}, {} bytes]", payload.schema, payload.encoding,
                     payload.data.size());

  if (payload.data.empty()) {
    return out.str();
  }

  const size_t toShow = std::min(payload.data.size(), maxShown);
  out << std::format(" payload[0:{}]=0x", toShow - 1) << std::hex
      << std::setfill('0');
  std::for_each(payload.data.begin(), payload.data.begin() + toShow,
                [&out](const char byte) {
                  out << std::setw(2) << static_cast<unsigned int>(byte);
                });

  return out.str();
}

/// An unique event identifier.
///
/// Using std::optional to allow explicit check if the field was set.
struct EventId {
  std::optional<uint32_t> runNumber;
  std::optional<uint64_t> eventNumber;
  std::optional<uint32_t> lumiBlock;
  std::optional<uint32_t> timeStamp;
  uint32_t timeStampNsOffset = 0;
  uint32_t bunchCrossingId = 0;

  /// Names of the required fields that are missing.
  std::vector<std::string> missing() const {
    std::vector<std::string> absent;
    if (!runNumber) {
      absent.push_back("run_number");
    }
    if (!eventNumber) {
      absent.push_back("event_number");
    }
    if (!lumiBlock) {
      absent.push_back("lumi_block");
    }
    if (!timeStamp) {
      absent.push_back("time_stamp");
    }
    return absent;
  }
};

struct ExecuteRequest {
  std::string sequence;  ///< fragment name, e.g. "RemoteExecSeqSum"
  std::vector<Payload> inputs;
  uint64_t requestId = 0;  ///< client correlation id, echoed back
  EventId eventId;
};

struct ExecuteReply {
  Status status = Status::Unspecified;
  std::string diagnostics;
  std::vector<Payload> outputs;
  uint64_t requestId = 0;
};

/// Information what kind of data is expected, in which RPC protocol, and the
/// meaning of the data
struct BoundaryInfo {
  std::string key;       ///< StoreGate key
  std::string encoding;  ///< serialisation mechanism; see IPayloadCodec.h
  std::string schema;    ///< what the bytes are, may be empty
};

/// Parse "key#encoding#schema" into BoundaryInfo. `key` and `encoding` must be
/// present, `schema` is optional.
inline std::optional<BoundaryInfo> parseBoundary(std::string_view spec) {
  const std::vector<std::string> fields = CxxUtils::tokenize(spec, '#');
  if (fields.size() != 3 && !(fields.size() == 2 && spec.back() == '#')) {
    return std::nullopt;
  }

  if (fields[0].empty() || fields[1].empty()) {
    return std::nullopt;
  }

  return BoundaryInfo{.key = fields[0],
                      .encoding = fields[1],
                      .schema = fields.size() == 3 ? fields[2] : ""};
}

/// Information about what sequence the server is capable of executing
struct SequenceInfo {
  std::string name;
  std::vector<BoundaryInfo> inputs;
  std::vector<BoundaryInfo> outputs;
};

/// @name Codec
/// Serialisations to/from the RPC format, with wire being the view of binary
/// data.
bool encode(const ExecuteRequest& request, std::string& wire);
bool decode(std::string_view wire, ExecuteRequest& request);
bool encode(const ExecuteReply& reply, std::string& wire);
bool decode(std::string_view wire, ExecuteReply& reply);
/// @}

/// Returns string describing RPC framework and its version.
std::string rpcFrameworkVersion();

}  // namespace AthRemoteExec

#endif  // ATHREMOTEEXECWIRE_REMOTEEXECWIRE_H
