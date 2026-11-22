/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file remote_exec_protocol_roundtrip.cxx
 * @brief The wire codec round-trips, and the gRPC linkage is real.
 *
 * A payload is bytes plus two strings, so what there is to verify is that
 * those survive intact -- including binary data with embedded nulls, which is
 * the case a std::string-based carrier gets wrong if anyone ever reaches for
 * strlen.
 */

#include <AthRemoteExecWire/Protocol.h>

#include <cstdio>
#include <string>

#include "CxxUtils/checker_macros.h"

// Single-threaded by construction.
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

namespace {

int s_failures = 0;

void check(bool condition, const std::string& what) {
  if (!condition) {
    std::printf("FAIL: %s\n", what.c_str());
    ++s_failures;
  } else {
    std::printf("ok: %s\n", what.c_str());
  }
}

AthRemoteExec::Payload payload(const std::string& key,
                               const std::string& encoding,
                               const std::string& schema,
                               const std::string& data) {
  AthRemoteExec::Payload made;
  made.key = key;
  made.encoding = encoding;
  made.schema = schema;
  made.data = data;
  return made;
}

}  // anonymous namespace

int main() {
  std::printf("linked against gRPC %s\n",
              AthRemoteExec::rpcFrameworkVersion().c_str());

  AthRemoteExec::ExecuteRequest request;
  request.sequence = "RemoteExecSeqSum";
  request.requestId = 4242;
  request.eventId.runNumber = 456789;
  request.eventId.eventNumber = 0;  // zero is a real event number...
  request.eventId.lumiBlock = 12;
  request.eventId.timeStamp = 0;  // ...and a real timestamp
  request.eventId.bunchCrossingId = 1234;
  request.inputs.push_back(
      payload("addends", "protobuf", "test_ints", std::string(8, '\x01')));
  request.inputs.push_back(payload("blob", "some-other-codec",
                                   "whatever.that.codec.calls.it",
                                   std::string("\x01\x00\x02\x00\x03", 5)));
  request.inputs.push_back(payload("empty", "protobuf", "test_floats", ""));

  std::string wire;
  check(AthRemoteExec::encode(request, wire), "Encoding works");

  AthRemoteExec::ExecuteRequest decoded;
  check(AthRemoteExec::decode(wire, decoded), "Decoding works");
  check(decoded.sequence == request.sequence && decoded.requestId == 4242,
        "Envelope survives");
  check(decoded.inputs.size() == 3, "Payloads survive");

  check(decoded.eventId.runNumber == 456789u &&
            decoded.eventId.eventNumber == 0u &&
            decoded.eventId.lumiBlock == 12u &&
            decoded.eventId.timeStamp == 0u &&
            decoded.eventId.bunchCrossingId == 1234u,
        "Event identity survives, zeros included");
  check(decoded.eventId.missing().empty(), "Nothing is missing");

  AthRemoteExec::ExecuteRequest bare;
  bare.sequence = "RemoteExecSeqSum";
  std::string bareWire;
  AthRemoteExec::ExecuteRequest bareDecoded;
  check(AthRemoteExec::encode(bare, bareWire) &&
            AthRemoteExec::decode(bareWire, bareDecoded),
        "Request with no identity round-trips");
  check(bareDecoded.eventId.missing().size() == 4,
        "Reports with no identity reports all four required fields missing");

  bool identical = decoded.inputs.size() == request.inputs.size();
  for (size_t i = 0; identical && i < decoded.inputs.size(); ++i) {
    identical = decoded.inputs[i].key == request.inputs[i].key &&
                decoded.inputs[i].encoding == request.inputs[i].encoding &&
                decoded.inputs[i].schema == request.inputs[i].schema &&
                decoded.inputs[i].data == request.inputs[i].data;
  }
  check(identical, "Keys, encodings, schema names and bytes survive");
  check(decoded.inputs[1].data.size() == 5,
        "Binary data with embedded nulls keeps its length");

  AthRemoteExec::ExecuteReply reply;
  reply.status = AthRemoteExec::Status::AlgFailure;
  reply.diagnostics = "something went wrong";
  reply.requestId = 4242;
  reply.outputs.push_back(
      payload("sum", "raw", "long", std::string(8, '\x07')));

  std::string replyWire;
  check(AthRemoteExec::encode(reply, replyWire), "Encoding works");
  AthRemoteExec::ExecuteReply decodedReply;
  check(AthRemoteExec::decode(replyWire, decodedReply), "Decoding works");
  check(decodedReply.status == AthRemoteExec::Status::AlgFailure &&
            decodedReply.diagnostics == reply.diagnostics &&
            decodedReply.outputs.size() == 1,
        "Reply survives");
  check(std::string(toString(AthRemoteExec::Status::UnknownSequence)) ==
            "UNKNOWN_SEQUENCE",
        "Statuses have names");

  // describe() is called on every reply, so it must never touch the data.
  const std::string described = describe(decodedReply.outputs[0]);
  check(described == "long [raw, 8 bytes] payload[0:7]=0x0707070707070707",
        "describe() returns correct human-readable description");

  if (s_failures != 0) {
    std::printf("remote_exec_protocol_roundtrip: %d check(s) failed\n",
                s_failures);
    return EXIT_FAILURE;
  }
  std::printf("remote_exec_protocol_roundtrip: all checks passed\n");
  return EXIT_SUCCESS;
}
