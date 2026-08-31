/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file rpc_wire_roundtrip.cxx
 * @brief The wire codec round-trips, and the gRPC linkage is real.
 *
 * A payload is bytes plus two strings, so what there is to verify is that
 * those survive intact -- including binary data with embedded nulls, which is
 * the case a std::string-based carrier gets wrong if anyone ever reaches for
 * strlen.
 */

#include "src/RpcWire.h"

#include "CxxUtils/checker_macros.h"

#include <cstdio>
#include <string>

// Single-threaded by construction.
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

namespace {

int s_failures = 0;

void check( bool condition, const std::string& what )
{
  if ( !condition ) {
    std::printf( "FAIL: %s\n", what.c_str() );
    ++s_failures;
  } else {
    std::printf( "ok: %s\n", what.c_str() );
  }
}

AthExRpc::Payload payload( const std::string& key, const std::string& encoding,
                           const std::string& schema,
                           const std::string& data )
{
  AthExRpc::Payload made;
  made.key = key;
  made.encoding = encoding;
  made.schema = schema;
  made.data = data;
  return made;
}

}  // anonymous namespace

int main()
{
  std::printf( "linked against gRPC %s\n", AthExRpc::grpcVersion().c_str() );

  AthExRpc::ExecuteRequest request;
  request.sequence = "RpcSeqSum";
  request.requestId = 4242;
  request.eventId.runNumber = 456789;
  request.eventId.eventNumber = 0;   // zero is a real event number...
  request.eventId.lumiBlock = 12;
  request.eventId.timeStamp = 0;     // ...and a real timestamp
  request.eventId.bunchCrossingId = 1234;
  request.inputs.push_back(
      payload( "addends", "protobuf", "athexrpc.demo.v1.Ints",
               std::string( 8, '\x01' ) ) );
  // Two things at once. The bytes are deliberately full of nulls, which is what
  // catches anyone treating a payload as a C string; and the encoding is one
  // nothing in this package implements, because the envelope is not supposed to
  // care -- it carries three strings and some bytes, and what they mean is the
  // receiving codec's business.
  request.inputs.push_back(
      payload( "blob", "some-other-codec", "whatever.that.codec.calls.it",
               std::string( "\x01\x00\x02\x00\x03", 5 ) ) );
  request.inputs.push_back(
      payload( "empty", "protobuf", "athexrpc.demo.v1.Doubles", "" ) );

  std::string wire;
  check( AthExRpc::encode( request, wire ), "a request encodes" );

  AthExRpc::ExecuteRequest decoded;
  check( AthExRpc::decode( wire, decoded ), "a request decodes" );
  check( decoded.sequence == request.sequence && decoded.requestId == 4242,
         "the envelope survives" );
  check( decoded.inputs.size() == 3, "every payload survives" );

  // The identity has to survive *including* its zeros. Both of these fields
  // were set to zero above, so a schema without explicit presence would decode
  // them as "not set" and the server would reject a perfectly good request.
  check( decoded.eventId.runNumber == 456789u &&
             decoded.eventId.eventNumber == 0u &&
             decoded.eventId.lumiBlock == 12u &&
             decoded.eventId.timeStamp == 0u &&
             decoded.eventId.bunchCrossingId == 1234u,
         "the event identity survives, zeros included" );
  check( decoded.eventId.missing().empty(),
         "a complete identity reports nothing missing" );

  // And an identity that was never set has to come back as missing rather than
  // as a plausible-looking run 0, which is the whole reason for the optionals.
  AthExRpc::ExecuteRequest bare;
  bare.sequence = "RpcSeqSum";
  std::string bareWire;
  AthExRpc::ExecuteRequest bareDecoded;
  check( AthExRpc::encode( bare, bareWire ) &&
             AthExRpc::decode( bareWire, bareDecoded ),
         "a request with no identity round-trips" );
  check( bareDecoded.eventId.missing().size() == 4,
         "and reports all four required fields missing" );

  bool identical = decoded.inputs.size() == request.inputs.size();
  for ( size_t i = 0; identical && i < decoded.inputs.size(); ++i ) {
    identical = decoded.inputs[i].key == request.inputs[i].key &&
                decoded.inputs[i].encoding == request.inputs[i].encoding &&
                decoded.inputs[i].schema == request.inputs[i].schema &&
                decoded.inputs[i].data == request.inputs[i].data;
  }
  check( identical, "keys, encodings, schema names and bytes all survive" );
  check( decoded.inputs[1].data.size() == 5,
         "binary data with embedded nulls keeps its length" );

  AthExRpc::ExecuteReply reply;
  reply.status = AthExRpc::Status::AlgFailure;
  reply.detail = "something went wrong";
  reply.requestId = 4242;
  reply.outputs.push_back( payload( "sum", "raw", "long",
                                    std::string( 8, '\x07' ) ) );

  std::string replyWire;
  check( AthExRpc::encode( reply, replyWire ), "a reply encodes" );
  AthExRpc::ExecuteReply decodedReply;
  check( AthExRpc::decode( replyWire, decodedReply ), "a reply decodes" );
  check( decodedReply.status == AthExRpc::Status::AlgFailure &&
             decodedReply.detail == reply.detail &&
             decodedReply.outputs.size() == 1,
         "the reply survives, status and all" );
  check( std::string( toString( AthExRpc::Status::UnknownSequence ) ) ==
             "UNKNOWN_SEQUENCE",
         "statuses have names" );

  // describe() is called on every reply, so it must never touch the data.
  const std::string described = describe( decodedReply.outputs[0] );
  check( described.find( "long" ) != std::string::npos &&
             described.find( "raw" ) != std::string::npos,
         "describe() names the type and encoding" );

  if ( s_failures != 0 ) {
    std::printf( "rpc_wire_roundtrip: %d check(s) failed\n", s_failures );
    return EXIT_FAILURE;
  }
  std::printf( "rpc_wire_roundtrip: all checks passed\n" );
  return EXIT_SUCCESS;
}
