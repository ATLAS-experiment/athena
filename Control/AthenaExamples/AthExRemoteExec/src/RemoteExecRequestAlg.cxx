/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RemoteExecRequestAlg.h"


#include <chrono>
#include <fstream>
#include <thread>

namespace AthExRemoteExec {

namespace {

/// A target is either "host:port" or the path of a file the server writes its
/// port into once it is listening. The second form exists so a test can start
/// client and server in either order and let the port be chosen by the OS; it
/// is the same convention the C++ test clients use.
std::string resolveTarget( const std::string& target, double timeoutSeconds )
{
  if ( target.find( ':' ) != std::string::npos ) {
    return target;
  }
  const auto deadline =
      std::chrono::steady_clock::now() +
      std::chrono::milliseconds( static_cast<long>( timeoutSeconds * 1000 ) );
  do {
    std::ifstream in( target );
    int port = 0;
    if ( in >> port && port > 0 ) {
      return "localhost:" + std::to_string( port );
    }
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
  } while ( std::chrono::steady_clock::now() < deadline );
  return {};
}

}  // anonymous namespace

StatusCode RemoteExecRequestAlg::initialize()
{
  if ( m_sequence.empty() ) {
    ATH_MSG_ERROR( "SequenceName is not set; there is nothing to ask for" );
    return StatusCode::FAILURE;
  }
  if ( m_target.empty() ) {
    ATH_MSG_ERROR( "Target is not set" );
    return StatusCode::FAILURE;
  }

  // Retrieved here rather than per event, so a codec that cannot resolve its
  // boundary stops the job instead of failing every request -- the same
  // reasoning as RemoteExecGateAlg::initialize.
  ATH_CHECK( m_inputs.retrieve() );
  ATH_CHECK( m_outputs.retrieve() );
  m_crossing.reserve( m_inputs.size() );
  for ( const ToolHandle<IPayloadCodec>& codec : m_inputs ) {
    m_crossing.push_back( codec.get() );
  }

  const std::string target = resolveTarget( m_target.value(), m_readyTimeout );
  if ( target.empty() ) {
    ATH_MSG_ERROR( "No server port appeared in '" << m_target.value()
                                                  << "' within "
                                                  << m_readyTimeout.value()
                                                  << " s" );
    return StatusCode::FAILURE;
  }

  m_client = std::make_unique<RemoteExecClient>( target );
  if ( !m_client->waitUntilReady( m_readyTimeout ) ) {
    ATH_MSG_ERROR( "Server at " << target << " did not answer within "
                                << m_readyTimeout.value() << " s" );
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO( "Will call '" << m_sequence.value() << "' at " << target );
  return StatusCode::SUCCESS;
}

StatusCode RemoteExecRequestAlg::execute( const EventContext& ctx ) const
{
  ExecuteRequest request;
  request.sequence = m_sequence;
  request.requestId = ctx.evt();

  // The server resolves conditions against this, so it is copied from the real
  // event rather than invented. A client with no event of its own would have to
  // make one up explicitly; a client that is an Athena job need not.
  const EventIDBase& id = ctx.eventID();
  request.eventId.runNumber = id.run_number();
  request.eventId.eventNumber = id.event_number();
  request.eventId.lumiBlock = id.lumi_block();
  request.eventId.timeStamp = id.time_stamp();
  request.eventId.timeStampNsOffset = id.time_stamp_ns_offset();
  request.eventId.bunchCrossingId = id.bunch_crossing_id();

  request.inputs.reserve( m_inputs.size() );
  std::vector<std::string> dangling;
  for ( const ToolHandle<IPayloadCodec>& codec : m_inputs ) {
    Payload payload;
    ATH_CHECK( codec->read( ctx, payload, m_crossing, dangling ) );
    ATH_MSG_DEBUG( "Sending " << describe( payload ) );
    request.inputs.push_back( std::move( payload ) );
  }

  // Once, because it is a property of the configuration rather than of the
  // event: the same references point at the same missing key on every one. Not
  // an error, because a reference that is not meant to be followed on the far
  // side is a legitimate thing to send -- but a fragment that meant to carry
  // the target and did not gets no other signal at all, since an unresolvable
  // reference reads downstream as an absence.
  if ( !dangling.empty() &&
       !m_warnedDangling.exchange( true, std::memory_order_relaxed ) ) {
    ATH_MSG_WARNING( "This request refers to keys it does not carry: "
                     << describeDangling( dangling )
                     << ". They will not resolve on the server. Declare them "
                        "as inputs too, or ignore this if they are not meant "
                        "to be followed there. Reported once" );
  }

  const ExecuteReply reply = m_client->execute( request );

  if ( reply.status != AthExRemoteExec::Status::Ok ) {
    // A refusal means the server never looked at the request, which is a
    // different thing from the fragment having failed, and worth saying so.
    ATH_MSG_ERROR( ( isRefusal( reply.status ) ? "Server refused request "
                                               : "Request " )
                   << request.requestId << " for '" << request.sequence
                   << "': " << toString( reply.status ) << " -- "
                   << reply.detail );
    return StatusCode::FAILURE;
  }

  for ( const ToolHandle<IPayloadCodec>& codec : m_outputs ) {
    const Payload* payload = nullptr;
    for ( const Payload& candidate : reply.outputs ) {
      if ( candidate.key == codec->key() ) {
        payload = &candidate;
        break;
      }
    }
    if ( payload == nullptr ) {
      ATH_MSG_ERROR( "Reply to request " << reply.requestId
                                         << " does not carry '" << codec->key()
                                         << "'" );
      return StatusCode::FAILURE;
    }
    ATH_CHECK( codec->record( ctx, *payload ) );
    ATH_MSG_DEBUG( "Received " << describe( *payload ) << " into '"
                               << codec->key() << "'" );
  }
  return StatusCode::SUCCESS;
}

}  // namespace AthExRemoteExec
