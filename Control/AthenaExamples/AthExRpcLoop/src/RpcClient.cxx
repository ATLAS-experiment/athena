/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RpcClient.h"

#include "RpcWireConvert.h"

#include <grpcpp/grpcpp.h>

#include "athrpc.grpc.pb.h"

#include <chrono>
#include <thread>

namespace AthExRpc {

namespace {

/// The inverse of RpcServer::refuse(): a refusal arrives as a status code, so
/// recognise the two the server uses and leave everything else Unspecified,
/// which is what a genuine transport failure is.
///
/// UNAVAILABLE is also what gRPC reports for a server that is not there at all,
/// so an unreachable server and one winding down come back the same way. That
/// conflation is deliberate rather than sloppy: for the only decision a client
/// makes on this -- retry, or try another host -- they are the same answer. The
/// detail string still says which it was.
Status fromGrpc( grpc::StatusCode code )
{
  switch ( code ) {
  case grpc::StatusCode::RESOURCE_EXHAUSTED:
    return Status::Overloaded;
  case grpc::StatusCode::UNAVAILABLE:
    return Status::ShuttingDown;
  default:
    return Status::Unspecified;
  }
}

}  // anonymous namespace

class RpcClient::Impl {
public:
  explicit Impl( const std::string& target )
      : m_channel( grpc::CreateCustomChannel(
            target, grpc::InsecureChannelCredentials(), unlimitedMessages() ) ),
        m_stub( athrpc::v1::SequenceExecutor::NewStub( m_channel ) )
  {
  }

  /// gRPC's 4 MB default silently turns a large payload into an unexplained
  /// transport error, which is not a useful failure mode for a payload whose
  /// size is the client's business.
  static grpc::ChannelArguments unlimitedMessages()
  {
    grpc::ChannelArguments args;
    args.SetMaxReceiveMessageSize( -1 );
    args.SetMaxSendMessageSize( -1 );
    return args;
  }

  ExecuteReply execute( const ExecuteRequest& request )
  {
    athrpc::v1::ExecuteRequest wireRequest;
    toProto( request, wireRequest );

    grpc::ClientContext context;
    athrpc::v1::ExecuteReply wireReply;
    const grpc::Status status = m_stub->Execute( &context, wireRequest, &wireReply );

    ExecuteReply reply;
    if ( !status.ok() ) {
      // A refusal arrives as a status code and nothing else -- gRPC does not
      // send a response message alongside a non-OK status. Map it back so a
      // caller sees the same Status vocabulary whichever way the answer came,
      // and can tell "the server was busy" from "the call broke".
      reply.status = fromGrpc( status.error_code() );
      reply.detail = isRefusal( reply.status )
                         ? status.error_message()
                         : "gRPC call failed: " + status.error_message();
      reply.requestId = request.requestId;
      return reply;
    }
    fromProto( wireReply, reply );
    return reply;
  }

  std::vector<SequenceInfo> listSequences( std::string* error )
  {
    grpc::ClientContext context;
    const athrpc::v1::ListSequencesRequest wireRequest;
    athrpc::v1::ListSequencesReply wireReply;
    const grpc::Status status =
        m_stub->ListSequences( &context, wireRequest, &wireReply );
    if ( !status.ok() ) {
      if ( error ) {
        *error = status.error_message();
      }
      return {};
    }

    std::vector<SequenceInfo> menu;
    menu.reserve( wireReply.sequences_size() );
    for ( const athrpc::v1::SequenceInfo& entry : wireReply.sequences() ) {
      SequenceInfo info;
      info.name = entry.name();
      const auto convert = []( const athrpc::v1::BoundaryInfo& in ) {
        return BoundaryInfo{in.key(), in.encoding(), in.schema()};
      };
      for ( const athrpc::v1::BoundaryInfo& boundary : entry.inputs() ) {
        info.inputs.push_back( convert( boundary ) );
      }
      for ( const athrpc::v1::BoundaryInfo& boundary : entry.outputs() ) {
        info.outputs.push_back( convert( boundary ) );
      }
      menu.push_back( std::move( info ) );
    }
    return menu;
  }

  bool waitUntilReady( double timeoutSeconds )
  {
    const auto deadline =
        std::chrono::steady_clock::now() +
        std::chrono::milliseconds( static_cast<long>( timeoutSeconds * 1000 ) );
    while ( std::chrono::steady_clock::now() < deadline ) {
      std::string error;
      listSequences( &error );
      if ( error.empty() ) {
        return true;
      }
      std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    }
    return false;
  }

private:
  std::shared_ptr<grpc::Channel> m_channel;
  std::unique_ptr<athrpc::v1::SequenceExecutor::Stub> m_stub;
};

RpcClient::RpcClient( const std::string& target )
    : m_impl( std::make_unique<Impl>( target ) )
{
}

RpcClient::~RpcClient() = default;

ExecuteReply RpcClient::execute( const ExecuteRequest& request )
{
  return m_impl->execute( request );
}

std::vector<SequenceInfo> RpcClient::listSequences( std::string* error )
{
  return m_impl->listSequences( error );
}

bool RpcClient::waitUntilReady( double timeoutSeconds )
{
  return m_impl->waitUntilReady( timeoutSeconds );
}

}  // namespace AthExRpc
