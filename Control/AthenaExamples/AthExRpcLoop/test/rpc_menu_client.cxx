/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file rpc_menu_client.cxx
 * @brief Drives the menu server with concurrent, mixed requests.
 *
 * Usage: rpc_menu_client <port-file|host:port> [delay-ms]
 *
 * This client is deliberately built the way a third party would build one. It
 * shares nothing with the server but two .proto files -- the envelope,
 * athrpc.proto, and the demonstration fragment's own schema,
 * athexrpc_demo.proto. It includes no header from this package, links no Athena
 * library, and uses the generated stubs directly rather than the convenience
 * wrapper in src/RpcClient.h.
 *
 * That split is the design claim, made executable: to call a fragment you need
 * the transport's schema and *that fragment's* schema, and nothing else. There
 * is no third thing to agree on, no registry of payload types, and no Athena on
 * this side of the socket.
 *
 * What it verifies:
 *  - mixed requests run concurrently, measured by wall clock: N requests that
 *    each take `delay` finish in appreciably less than N*delay;
 *  - the answers are right and are not mixed up between concurrent requests;
 *  - a fragment reading conditions gets the client's run, not the job's;
 *  - unknown sequences, malformed boundaries, undeclared and duplicated keys,
 *    unparseable payloads and failing algorithms all come back as error
 *    replies, with the right status, while the server keeps serving.
 */

#include <grpcpp/grpcpp.h>

#include "athexrpc_demo.pb.h"
#include "athrpc.grpc.pb.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <optional>
#include <set>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

constexpr const char* kInts = "athexrpc.demo.v1.Ints";
constexpr const char* kDoubles = "athexrpc.demo.v1.Doubles";

/// Every request must carry an event identity: the server rejects one that
/// does not, because conditions are resolved against it and a defaulted value
/// would be a wrong answer rather than an error. A synthetic client has to
/// invent one, but it has to invent it *explicitly*, which is the point.
void setEventId( athrpc::v1::ExecuteRequest& request, uint64_t eventNumber,
                 uint32_t runNumber = 1 )
{
  athrpc::v1::EventId* id = request.mutable_event_id();
  id->set_run_number( runNumber );
  id->set_event_number( eventNumber );
  id->set_lumi_block( 1 );
  id->set_time_stamp( 1735689600 );  // 2025-01-01, an arbitrary but real time
}

std::atomic<int> s_failures{0};

void check( bool condition, const std::string& what )
{
  if ( !condition ) {
    std::printf( "FAIL: %s\n", what.c_str() );
    ++s_failures;
  } else {
    std::printf( "ok: %s\n", what.c_str() );
  }
}

/// Wait for the server to publish its port, then build the target string.
std::string resolveTarget( const std::string& argument, double timeoutSeconds )
{
  if ( argument.find( ':' ) != std::string::npos ) {
    return argument;
  }
  const auto deadline =
      std::chrono::steady_clock::now() +
      std::chrono::milliseconds( static_cast<long>( timeoutSeconds * 1000 ) );
  while ( std::chrono::steady_clock::now() < deadline ) {
    std::ifstream in( argument );
    int port = 0;
    if ( in >> port && port > 0 ) {
      return "localhost:" + std::to_string( port );
    }
    std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
  }
  return {};
}

/// @name Building the fragment's payloads
/// The framework knows none of this; the fragment's schema is all it takes.
/// @{

athrpc::v1::Payload* addPayload( athrpc::v1::ExecuteRequest& request,
                                 const std::string& key,
                                 const std::string& schema,
                                 const std::string& bytes )
{
  athrpc::v1::Payload* payload = request.add_inputs();
  payload->set_key( key );
  payload->set_encoding( "protobuf" );
  payload->set_schema( schema );
  payload->set_data( bytes );
  return payload;
}

void addInts( athrpc::v1::ExecuteRequest& request, const std::string& key,
              const std::vector<std::pair<std::string, int64_t>>& values )
{
  athexrpc::demo::v1::Ints message;
  for ( const auto& [name, value] : values ) {
    athexrpc::demo::v1::Ints_Entry* entry = message.add_entry();
    entry->set_name( name );
    entry->set_value( value );
  }
  addPayload( request, key, kInts, message.SerializeAsString() );
}

void addDoubles( athrpc::v1::ExecuteRequest& request, const std::string& key,
                 const std::string& name, const std::vector<double>& values )
{
  athexrpc::demo::v1::Doubles message;
  message.set_name( name );
  message.mutable_value()->Add( values.begin(), values.end() );
  addPayload( request, key, kDoubles, message.SerializeAsString() );
}

/// The named integer inside the reply's payload under @c key.
std::optional<int64_t> intOf( const athrpc::v1::ExecuteReply& reply,
                              const std::string& key, const std::string& name )
{
  for ( const athrpc::v1::Payload& output : reply.outputs() ) {
    if ( output.key() != key || output.schema() != kInts ) {
      continue;
    }
    athexrpc::demo::v1::Ints message;
    if ( !message.ParseFromString( output.data() ) ) {
      return std::nullopt;
    }
    for ( const athexrpc::demo::v1::Ints_Entry& entry : message.entry() ) {
      if ( entry.name() == name ) {
        return entry.value();
      }
    }
  }
  return std::nullopt;
}

std::vector<double> doublesOf( const athrpc::v1::ExecuteReply& reply,
                               const std::string& key )
{
  for ( const athrpc::v1::Payload& output : reply.outputs() ) {
    if ( output.key() != key || output.schema() != kDoubles ) {
      continue;
    }
    athexrpc::demo::v1::Doubles message;
    if ( message.ParseFromString( output.data() ) ) {
      return {message.value().begin(), message.value().end()};
    }
  }
  return {};
}
/// @}

athrpc::v1::ExecuteRequest sumRequest( int64_t a, int64_t b, uint64_t id )
{
  athrpc::v1::ExecuteRequest request;
  request.set_sequence( "RpcSeqSum" );
  request.set_request_id( id );
  setEventId( request, id );
  addInts( request, "addends", {{"a", a}, {"b", b}} );
  return request;
}

athrpc::v1::ExecuteRequest chainRequest( int64_t p, int64_t q, int64_t r,
                                         uint64_t id )
{
  athrpc::v1::ExecuteRequest request;
  request.set_sequence( "RpcSeqChain" );
  request.set_request_id( id );
  setEventId( request, id );
  addInts( request, "terms", {{"p", p}, {"q", q}, {"r", r}} );
  return request;
}

/// One blocking Execute call. Transport failures come back as an unset status.
athrpc::v1::ExecuteReply call( athrpc::v1::SequenceExecutor::Stub& stub,
                               const athrpc::v1::ExecuteRequest& request )
{
  grpc::ClientContext context;
  athrpc::v1::ExecuteReply reply;
  const grpc::Status status = stub.Execute( &context, request, &reply );
  if ( !status.ok() ) {
    std::printf( "transport error on request %llu: %s\n",
                 static_cast<unsigned long long>( request.request_id() ),
                 status.error_message().c_str() );
  }
  return reply;
}

bool waitUntilReady( athrpc::v1::SequenceExecutor::Stub& stub,
                     double timeoutSeconds )
{
  const auto deadline =
      std::chrono::steady_clock::now() +
      std::chrono::milliseconds( static_cast<long>( timeoutSeconds * 1000 ) );
  while ( std::chrono::steady_clock::now() < deadline ) {
    grpc::ClientContext context;
    athrpc::v1::ListSequencesRequest request;
    athrpc::v1::ListSequencesReply reply;
    if ( stub.ListSequences( &context, request, &reply ).ok() ) {
      return true;
    }
    std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
  }
  return false;
}

}  // anonymous namespace

int main( int argc, char* argv[] )
{
  if ( argc < 2 || argc > 3 ) {
    std::printf( "usage: %s <port-file|host:port> [delay-ms]\n", argv[0] );
    return EXIT_FAILURE;
  }
  const long delayMs = argc == 3 ? std::strtol( argv[2], nullptr, 10 ) : 0;

  const std::string target = resolveTarget( argv[1], 120.0 );
  if ( target.empty() ) {
    std::printf( "FAIL: server never published a port\n" );
    return EXIT_FAILURE;
  }
  std::printf( "connecting to %s\n", target.c_str() );

  // Match the server's limit: gRPC's 4 MB default silently turns a large
  // payload into an unexplained transport error.
  grpc::ChannelArguments channelArgs;
  channelArgs.SetMaxReceiveMessageSize( -1 );
  channelArgs.SetMaxSendMessageSize( -1 );
  const std::shared_ptr<grpc::Channel> channel = grpc::CreateCustomChannel(
      target, grpc::InsecureChannelCredentials(), channelArgs );
  const std::unique_ptr<athrpc::v1::SequenceExecutor::Stub> stub =
      athrpc::v1::SequenceExecutor::NewStub( channel );

  if ( !waitUntilReady( *stub, 120.0 ) ) {
    std::printf( "FAIL: server did not become ready\n" );
    return EXIT_FAILURE;
  }

  // The menu is discoverable with no out-of-band knowledge beyond the schema.
  {
    grpc::ClientContext context;
    athrpc::v1::ListSequencesRequest request;
    athrpc::v1::ListSequencesReply reply;
    check( stub->ListSequences( &context, request, &reply ).ok(),
           "ListSequences succeeds" );

    // Presence rather than count: the menu grows as fragments are added, and a
    // count would turn every addition into a spurious failure here.
    std::set<std::string> offered;
    for ( const athrpc::v1::SequenceInfo& info : reply.sequences() ) {
      offered.insert( info.name() );
      if ( info.name() == "RpcSeqChain" ) {
        check( info.inputs_size() == 1 && info.inputs( 0 ).key() == "terms" &&
                   info.inputs( 0 ).encoding() == "protobuf" &&
                   info.inputs( 0 ).schema() == kInts,
               "RpcSeqChain advertises its input, with encoding and schema" );
        check( info.outputs_size() == 1 &&
                   info.outputs( 0 ).key() == "chainTotal" &&
                   info.outputs( 0 ).schema() == kInts,
               "RpcSeqChain advertises its output the same way" );
      }
    }
    for ( const char* expected : {"RpcSeqSum", "RpcSeqChain", "RpcSeqCond",
                                  "RpcSeqScale", "RpcSeqPing", "RpcSeqFail"} ) {
      check( offered.count( expected ) == 1,
             std::string( "the menu offers " ) + expected );
    }
  }

  // Mixed requests, fired at once, run concurrently. Each Chain request costs
  // `delay` on the server, so if they were serialised the batch could not
  // finish in less than nChain*delay.
  {
    constexpr int nChain = 4;
    constexpr int nSum = 4;
    std::vector<athrpc::v1::ExecuteReply> replies( nChain + nSum );
    std::vector<std::thread> threads;
    threads.reserve( replies.size() );

    const auto started = std::chrono::steady_clock::now();
    for ( int i = 0; i < nChain; ++i ) {
      threads.emplace_back( [&stub, &replies, i] {
        replies[i] = call(
            *stub, chainRequest( i, 10, 100, static_cast<uint64_t>( 100 + i ) ) );
      } );
    }
    for ( int i = 0; i < nSum; ++i ) {
      threads.emplace_back( [&stub, &replies, i] {
        replies[nChain + i] =
            call( *stub, sumRequest( i, 1000, static_cast<uint64_t>( 200 + i ) ) );
      } );
    }
    for ( std::thread& thread : threads ) {
      thread.join();
    }
    const double elapsedMs =
        std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - started )
            .count();
    std::printf( "%d concurrent requests took %.0f ms (serial lower bound "
                 "would be %ld ms)\n",
                 nChain + nSum, elapsedMs, nChain * delayMs );

    bool allOk = true;
    for ( int i = 0; i < nChain; ++i ) {
      const std::optional<int64_t> chain =
          intOf( replies[i], "chainTotal", "chain" );
      allOk &= replies[i].status() == athrpc::v1::STATUS_OK &&
               chain.has_value() && *chain == i + 110 &&
               replies[i].request_id() == static_cast<uint64_t>( 100 + i );
    }
    for ( int i = 0; i < nSum; ++i ) {
      const std::optional<int64_t> sum =
          intOf( replies[nChain + i], "total", "sum" );
      allOk &= replies[nChain + i].status() == athrpc::v1::STATUS_OK &&
               sum.has_value() && *sum == i + 1000 &&
               replies[nChain + i].request_id() ==
                   static_cast<uint64_t>( 200 + i );
    }
    check( allOk, "every concurrent request got its own correct answer" );

    if ( delayMs > 0 ) {
      // Strictly less than the serial lower bound is the property being
      // claimed; the 0.8 keeps it from passing on a rounding error without
      // turning the check into a benchmark of the machine.
      check( elapsedMs < 0.8 * nChain * delayMs,
             "concurrent requests overlapped rather than queuing up" );
    }
  }

  // A fragment with no inputs at all, and one carrying an array rather than
  // scalars. Same envelope, same machinery, different schema.
  {
    athrpc::v1::ExecuteRequest request;
    request.set_sequence( "RpcSeqPing" );
    request.set_request_id( 310 );
    setEventId( request, 310 );
    const athrpc::v1::ExecuteReply reply = call( *stub, request );
    check( reply.status() == athrpc::v1::STATUS_OK &&
               intOf( reply, "pong", "tick" ) == 310,
           "an input-free fragment answers, with the event it was asked about" );
  }
  {
    athrpc::v1::ExecuteRequest request;
    request.set_sequence( "RpcSeqScale" );
    request.set_request_id( 311 );
    setEventId( request, 311 );
    addDoubles( request, "points", "values", {1.5, -2.0, 0.0, 4.25} );
    const athrpc::v1::ExecuteReply reply = call( *stub, request );
    const std::vector<double> scaled = doublesOf( reply, "scaledPoints" );
    check( reply.status() == athrpc::v1::STATUS_OK && scaled.size() == 4 &&
               scaled[0] == 3.0 && scaled[1] == -4.0 && scaled[2] == 0.0 &&
               scaled[3] == 8.5,
           "an array payload crosses both ways and comes back scaled" );
  }

  // Everything that can go wrong comes back as a reply, with the status that
  // says whose fault it was.
  {
    athrpc::v1::ExecuteRequest request;
    request.set_sequence( "NoSuchSequence" );
    request.set_request_id( 300 );
    setEventId( request, 300 );
    const athrpc::v1::ExecuteReply reply = call( *stub, request );
    check( reply.status() == athrpc::v1::STATUS_UNKNOWN_SEQUENCE,
           "an unknown sequence is rejected" );
    check( !reply.detail().empty(), "the rejection explains itself" );
  }
  {
    athrpc::v1::ExecuteRequest request;
    request.set_sequence( "RpcSeqSum" );
    request.set_request_id( 301 );
    setEventId( request, 301 );  // no payload at all
    const athrpc::v1::ExecuteReply reply = call( *stub, request );
    check( reply.status() == athrpc::v1::STATUS_INVALID_REQUEST,
           "a request missing its input is rejected before it costs a slot" );
  }
  {
    // Right key, wrong schema. The server refuses without looking at the
    // bytes: the boundary says what this key is, and reinterpreting a Doubles
    // message as an Ints message is exactly the silent failure the declaration
    // exists to prevent -- protobuf would parse it, and yield nothing.
    athrpc::v1::ExecuteRequest request;
    request.set_sequence( "RpcSeqSum" );
    request.set_request_id( 302 );
    setEventId( request, 302 );
    addDoubles( request, "addends", "addends", {1.0, 2.0} );
    const athrpc::v1::ExecuteReply reply = call( *stub, request );
    check( reply.status() == athrpc::v1::STATUS_INVALID_REQUEST,
           "a payload whose schema is not the declared one is refused" );
    check( reply.detail().find( kInts ) != std::string::npos,
           "and the refusal says what was expected" );
  }
  {
    // A key the fragment never declared. Running anyway would answer this
    // client with a result computed from something else.
    athrpc::v1::ExecuteRequest request = sumRequest( 1, 2, 312 );
    addInts( request, "notAnInput", {{"a", 1}} );
    const athrpc::v1::ExecuteReply reply = call( *stub, request );
    check( reply.status() == athrpc::v1::STATUS_INVALID_REQUEST,
           "an undeclared input key is refused" );
  }
  {
    // The same key twice: the gate would take the first and never look at the
    // second, which may well be the one the client meant.
    athrpc::v1::ExecuteRequest request = sumRequest( 1, 2, 313 );
    addInts( request, "addends", {{"a", 9}, {"b", 9}} );
    const athrpc::v1::ExecuteReply reply = call( *stub, request );
    check( reply.status() == athrpc::v1::STATUS_INVALID_REQUEST,
           "a duplicated input key is refused" );
  }
  {
    // Declared correctly, but the bytes are not a message. This one *is* the
    // fragment's business -- the framework carried opaque bytes faithfully and
    // the fragment's own adapter is what refuses them.
    athrpc::v1::ExecuteRequest request;
    request.set_sequence( "RpcSeqSum" );
    request.set_request_id( 314 );
    setEventId( request, 314 );
    addPayload( request, "addends", kInts, std::string( 16, '\xff' ) );
    const athrpc::v1::ExecuteReply reply = call( *stub, request );
    check( reply.status() == athrpc::v1::STATUS_ALG_FAILURE,
           "an unparseable payload fails the fragment, not the server" );
    check( reply.detail().find( "RpcSeqSumDecode" ) != std::string::npos,
           "and the error names the fragment's own adapter" );
  }
  {
    athrpc::v1::ExecuteRequest request;
    request.set_sequence( "RpcSeqFail" );
    request.set_request_id( 303 );
    setEventId( request, 303 );
    const athrpc::v1::ExecuteReply reply = call( *stub, request );
    check( reply.status() == athrpc::v1::STATUS_ALG_FAILURE,
           "a failing algorithm produces an error reply" );
    check( reply.detail().find( "RpcSeqFailAlg" ) != std::string::npos,
           "the error names the algorithm that failed" );
  }
  {
    // A request with no event identity must be refused, and told why. The
    // fields are mandatory precisely because the alternative is not an error
    // but a wrong answer: conditions would resolve against whatever the
    // default happened to be.
    athrpc::v1::ExecuteRequest request;
    request.set_sequence( "RpcSeqSum" );
    request.set_request_id( 306 );
    addInts( request, "addends", {{"a", 1}, {"b", 2}} );
    const athrpc::v1::ExecuteReply reply = call( *stub, request );
    check( reply.status() == athrpc::v1::STATUS_INVALID_REQUEST,
           "a request with no event identity is rejected" );
    check( reply.detail().find( "run_number" ) != std::string::npos &&
               reply.detail().find( "event_number" ) != std::string::npos,
           "and the rejection names the fields that are missing" );
  }
  {
    // Half an identity is no better than none.
    athrpc::v1::ExecuteRequest request;
    request.set_sequence( "RpcSeqSum" );
    request.set_request_id( 307 );
    request.mutable_event_id()->set_run_number( 1 );
    addInts( request, "addends", {{"a", 1}, {"b", 2}} );
    const athrpc::v1::ExecuteReply reply = call( *stub, request );
    check( reply.status() == athrpc::v1::STATUS_INVALID_REQUEST,
           "a partial event identity is rejected too" );
  }

  // Conditions, over the wire, for two different runs *in the same job*. The
  // offset the conditions algorithm produces is 100 + the run number, so the
  // two answers must differ -- which is the whole point of the identity coming
  // from the client. Before that, the run number was a job property and this
  // needed two server processes to test at all.
  {
    athrpc::v1::ExecuteRequest first;
    first.set_sequence( "RpcSeqCond" );
    first.set_request_id( 304 );
    setEventId( first, 304, 1 );
    addInts( first, "value", {{"x", 5}} );
    const std::optional<int64_t> one =
        intOf( call( *stub, first ), "offsetResult", "offsetted" );

    athrpc::v1::ExecuteRequest second;
    second.set_sequence( "RpcSeqCond" );
    second.set_request_id( 305 );
    setEventId( second, 305, 7 );
    addInts( second, "value", {{"x", 5}} );
    const std::optional<int64_t> seven =
        intOf( call( *stub, second ), "offsetResult", "offsetted" );

    check( one.has_value() && *one == 106,
           "a fragment reading conditions works on a synthetic event" );
    check( seven.has_value() && *seven == 112,
           "and a second run, in the same job, gets its own conditions" );
  }

  // The server survived all of that and still works.
  {
    const athrpc::v1::ExecuteReply reply = call( *stub, sumRequest( 1, 1, 308 ) );
    check( reply.status() == athrpc::v1::STATUS_OK &&
               intOf( reply, "total", "sum" ) == 2,
           "the server keeps serving after failures" );
  }

  if ( s_failures != 0 ) {
    std::printf( "rpc_menu_client: %d check(s) failed\n", s_failures.load() );
    return EXIT_FAILURE;
  }
  std::printf( "rpc_menu_client: all checks passed\n" );
  return EXIT_SUCCESS;
}
