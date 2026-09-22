/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RemoteExecPackAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include <memory>
#include <utility>

namespace AthExRemoteExec {

StatusCode RemoteExecPackAlg::initialize()
{
  if ( m_reply.key().empty() ) {
    ATH_MSG_ERROR( "Reply is not set; the loop manager would never find this "
                   "fragment's result" );
    return StatusCode::FAILURE;
  }
  ATH_CHECK( m_request.initialize() );
  ATH_CHECK( m_reply.initialize() );

  ATH_CHECK( m_outputs.retrieve() );
  m_crossing.reserve( m_outputs.size() );
  for ( const ToolHandle<IPayloadCodec>& codec : m_outputs ) {
    m_crossing.push_back( codec.get() );
  }
  return StatusCode::SUCCESS;
}

StatusCode RemoteExecPackAlg::execute( const EventContext& ctx ) const
{
  SG::ReadHandle<RemoteExecRequestDescriptor> request( m_request, ctx );
  if ( !request.isValid() ) {
    ATH_MSG_ERROR( "No request under '" << m_request.key() << "'" );
    return StatusCode::FAILURE;
  }

  ExecuteReply reply;
  reply.status = AthExRemoteExec::Status::Ok;
  reply.requestId = request->requestId();

  if ( !m_outputs.empty() ) {
    reply.outputs.reserve( m_outputs.size() );
    std::vector<std::string> dangling;
    for ( const ToolHandle<IPayloadCodec>& codec : m_outputs ) {
      Payload payload;
      ATH_CHECK( codec->read( ctx, payload, m_crossing, dangling ) );
      reply.outputs.push_back( std::move( payload ) );
    }
    // See RemoteExecRequestAlg for why this is a warning reported once rather than an
    // error: a reply may legitimately carry references nobody is meant to
    // follow, and the client cannot tell that case from a forgotten output.
    if ( !dangling.empty() &&
         !m_warnedDangling.exchange( true, std::memory_order_relaxed ) ) {
      ATH_MSG_WARNING( "This reply refers to keys it does not carry: "
                       << describeDangling( dangling )
                       << ". They will not resolve on the client. Add them to "
                          "this fragment's outputs, or ignore this if they are "
                          "not meant to be followed there. Reported once" );
    }
  }

  SG::WriteHandle<RemoteExecReplyStaging> staged( m_reply, ctx );
  // recordNonConst, not record: the loop manager is the sole reader and moves
  // the payload out of this object rather than copying it, which needs a
  // non-const proxy. record() locks the object, and the retrieval then fails
  // with "No valid proxy" -- correctly, but at run time and per request.
  ATH_CHECK( staged.recordNonConst(
      std::make_unique<RemoteExecReplyStaging>( std::move( reply ) ) ) );
  ATH_MSG_DEBUG( "Staged reply for request " << request->requestId()
                                             << " on slot " << ctx.slot() );
  return StatusCode::SUCCESS;
}

}  // namespace AthExRemoteExec
