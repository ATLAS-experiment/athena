/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file rpc_load_client.cxx
 * @brief Fires more requests at the server than it can possibly serve.
 *
 * Usage: rpc_load_client <port-file|host:port> --requests N --concurrency C
 *
 * Exists for one property: a server under more load than it has slots must
 * *refuse*, not absorb. So this counts three outcomes separately and treats
 * them differently -- an answered request, a refused one, and a failed one --
 * because that distinction is the whole point of carrying refusals as gRPC
 * status codes rather than in the reply body. A refusal is a normal outcome
 * here and a failure is not.
 *
 * Like rpc_menu_client it is built from the .proto files alone, so what it
 * proves about the refusal is what a real client would see.
 */

#include <grpcpp/grpcpp.h>

#include "athexrpc_demo.pb.h"
#include "athrpc.grpc.pb.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

namespace {

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

athrpc::v1::ExecuteRequest chainRequest( uint64_t id )
{
  athrpc::v1::ExecuteRequest request;
  request.set_sequence( "RpcSeqChain" );
  request.set_request_id( id );
  athrpc::v1::EventId* event = request.mutable_event_id();
  event->set_run_number( 1 );
  event->set_event_number( id );
  event->set_lumi_block( 1 );
  event->set_time_stamp( 1735689600 );

  athexrpc::demo::v1::Ints message;
  for ( const char* name : {"p", "q", "r"} ) {
    athexrpc::demo::v1::Ints_Entry* entry = message.add_entry();
    entry->set_name( name );
    entry->set_value( 1 );
  }
  athrpc::v1::Payload* payload = request.add_inputs();
  payload->set_key( "terms" );
  payload->set_encoding( "protobuf" );
  payload->set_schema( "athexrpc.demo.v1.Ints" );
  payload->set_data( message.SerializeAsString() );
  return request;
}

}  // anonymous namespace

int main( int argc, char* argv[] )
{
  if ( argc < 2 ) {
    std::printf( "usage: %s <port-file|host:port> [--requests N] "
                 "[--concurrency C]\n",
                 argv[0] );
    return EXIT_FAILURE;
  }
  int requests = 100;
  int concurrency = 16;
  for ( int i = 2; i + 1 < argc; i += 2 ) {
    if ( std::strcmp( argv[i], "--requests" ) == 0 ) {
      requests = std::atoi( argv[i + 1] );
    } else if ( std::strcmp( argv[i], "--concurrency" ) == 0 ) {
      concurrency = std::atoi( argv[i + 1] );
    } else {
      std::printf( "unknown option %s\n", argv[i] );
      return EXIT_FAILURE;
    }
  }

  const std::string target = resolveTarget( argv[1], 120.0 );
  if ( target.empty() ) {
    std::printf( "FAIL: server never published a port\n" );
    return EXIT_FAILURE;
  }

  const std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(
      target, grpc::InsecureChannelCredentials() );
  const std::unique_ptr<athrpc::v1::SequenceExecutor::Stub> stub =
      athrpc::v1::SequenceExecutor::NewStub( channel );

  // Wait for the server before starting the clock, so "refused" means the
  // queue was full and not that the server had not finished starting.
  {
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds( 120 );
    bool ready = false;
    while ( !ready && std::chrono::steady_clock::now() < deadline ) {
      grpc::ClientContext context;
      athrpc::v1::ListSequencesRequest request;
      athrpc::v1::ListSequencesReply reply;
      ready = stub->ListSequences( &context, request, &reply ).ok();
      if ( !ready ) {
        std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
      }
    }
    if ( !ready ) {
      std::printf( "FAIL: server did not become ready\n" );
      return EXIT_FAILURE;
    }
  }

  std::atomic<int> answered{0};
  std::atomic<int> refused{0};
  std::atomic<int> failed{0};
  std::atomic<int> next{0};

  std::vector<std::thread> threads;
  threads.reserve( concurrency );
  for ( int t = 0; t < concurrency; ++t ) {
    threads.emplace_back( [&] {
      for ( int id = next++; id < requests; id = next++ ) {
        grpc::ClientContext context;
        athrpc::v1::ExecuteReply reply;
        const grpc::Status status = stub->Execute(
            &context, chainRequest( static_cast<uint64_t>( id ) + 1 ), &reply );
        if ( status.error_code() == grpc::StatusCode::RESOURCE_EXHAUSTED ||
             status.error_code() == grpc::StatusCode::UNAVAILABLE ) {
          // The server never looked at it. That is a normal answer to "are you
          // busy", and the client's business is to back off, not to fail.
          ++refused;
        } else if ( !status.ok() ||
                    reply.status() != athrpc::v1::STATUS_OK ) {
          ++failed;
          std::printf( "request %d failed: %s / %s\n", id,
                       status.error_message().c_str(),
                       reply.detail().c_str() );
        } else {
          ++answered;
        }
      }
    } );
  }
  for ( std::thread& thread : threads ) {
    thread.join();
  }

  std::printf( "answered %d\nrefused %d\nfailed %d\n", answered.load(),
               refused.load(), failed.load() );
  if ( failed != 0 ) {
    std::printf( "rpc_load_client: FAILURES\n" );
    return EXIT_FAILURE;
  }
  if ( answered == 0 ) {
    std::printf( "rpc_load_client: nothing was answered at all\n" );
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
