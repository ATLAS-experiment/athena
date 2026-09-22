/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXREMOTEEXEC_REMOTEEXECWIRE_H
#define ATHEXREMOTEEXEC_REMOTEEXECWIRE_H

/**
 * @file RemoteExecProtocol.h
 * @brief Plain C++ mirror of the athremoteexec.v1 wire contract.
 *
 * Nothing in this header knows about protobuf or gRPC. LCG ships protobuf as a
 * static library only, so every consumer embeds its own copy of the runtime; a
 * second, differently built copy in the same process risks the descriptor-pool
 * abort ("File already exists in database"). The mitigation is to confine all
 * generated code to one library behind this interface. No generated header may
 * appear in a header outside that library.
 *
 * Note what is *not* here: any enumeration of payload types. A payload is bytes
 * plus an encoding and a schema name, both resolved at run time. See the design
 * note in proto/athremoteexec.proto for why.
 */

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace AthExRemoteExec {

/// Mirrors athremoteexec.v1.Status; the numeric values match the proto enum.
/// What became of a request.
///
/// The first six mirror athremoteexec.v1.Status and travel in the reply body. The last
/// two do not: a refusal means the server never looked at the request, so it
/// leaves as a gRPC status code instead (see isRefusal() and athremoteexec.proto for
/// why). They stay in this enum because the loop managers still need to say
/// which refusal it was; RemoteExecServer maps them at the boundary and RemoteExecClient maps
/// them back, so a caller sees one vocabulary either way.
enum class Status : int {
  Unspecified = 0,
  Ok = 1,
  UnknownSequence = 2,
  InvalidRequest = 3,
  AlgFailure = 4,
  Timeout = 5,
  Overloaded = 6,    ///< transport only: RESOURCE_EXHAUSTED
  ShuttingDown = 7,  ///< transport only: UNAVAILABLE
};

/// Is this a refusal rather than an outcome? Refusals never reach the reply
/// body, and are the ones worth retrying.
constexpr bool isRefusal( Status status )
{
  return status == Status::Overloaded || status == Status::ShuttingDown;
}

const char* toString( Status status );

/// One value crossing the fragment boundary. Mirrors athremoteexec.v1.Payload.
struct Payload {
  std::string key;       ///< StoreGate key at the fragment boundary
  std::string encoding;  ///< serialisation mechanism; see IPayloadCodec.h
  std::string schema;    ///< what the bytes are, in that mechanism's terms
  std::string data;      ///< binary, and may contain embedded nulls
};

/// Human-readable rendering of a payload, for log messages. Never renders the
/// data itself: a payload can be tens of megabytes.
std::string describe( const Payload& payload );

/**
 * @brief Which event a request is for.
 *
 * The four std::optional members are the ones the server refuses to guess:
 * conditions are resolved against the EventID built from them, so a defaulted
 * value is not a failure but a wrong answer. std::optional rather than a
 * sentinel because zero is a representable event number and a representable
 * timestamp, and "unset" has to be distinguishable from either.
 */
struct EventId {
  std::optional<uint32_t> runNumber;
  std::optional<uint64_t> eventNumber;
  std::optional<uint32_t> lumiBlock;
  std::optional<uint32_t> timeStamp;
  /// Nanosecond part of the timestamp; zero is a real value, so not optional.
  uint32_t timeStampNsOffset = 0;
  /// Zero is a real bunch crossing, and conditions do not key on it.
  uint32_t bunchCrossingId = 0;

  /// Names of the required fields this identity is missing, for the client's
  /// error message. Empty means it is usable.
  std::vector<std::string> missing() const;
};

struct ExecuteRequest {
  std::string sequence;
  std::vector<Payload> inputs;
  uint64_t requestId = 0;
  EventId eventId;
};

struct ExecuteReply {
  Status status = Status::Unspecified;
  std::string detail;
  std::vector<Payload> outputs;
  uint64_t requestId = 0;
};

/// What a fragment expects or returns under one key. Advertised by
/// ListSequences so a client can discover what to build.
struct BoundaryInfo {
  std::string key;
  std::string encoding;
  /// Empty for an encoding that carries exactly one payload type.
  std::string schema;
};

struct SequenceInfo {
  std::string name;
  std::vector<BoundaryInfo> inputs;
  std::vector<BoundaryInfo> outputs;
};

/// @name Codec
/// Serialisation to/from the athremoteexec.v1 wire format. The gRPC server uses the
/// generated message classes directly (see RemoteExecProtocolConvert.h); these entry
/// points exist for tests and for transports that carry opaque bytes.
/// @{
bool encode( const ExecuteRequest& request, std::string& wire );
bool decode( std::string_view wire, ExecuteRequest& request );
bool encode( const ExecuteReply& reply, std::string& wire );
bool decode( std::string_view wire, ExecuteReply& reply );
/// @}

/// Version of the gRPC runtime this library was linked against. Exists so that
/// a build-level test can prove the gRPC (not just protobuf) linkage works.
std::string grpcVersion();

}  // namespace AthExRemoteExec

#endif  // ATHEXREMOTEEXEC_RPCWIRE_H
