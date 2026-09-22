/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RemoteExecGateAlg.h"

#include "StoreGate/ReadHandle.h"

namespace AthExRemoteExec {

StatusCode RemoteExecGateAlg::initialize()
{
  if ( m_sequence.empty() ) {
    ATH_MSG_ERROR( "SequenceName is not set; this gate would never select "
                   "anything" );
    return StatusCode::FAILURE;
  }
  ATH_CHECK( m_request.initialize() );

  // Retrieved here rather than per request: a codec that cannot resolve its
  // own boundary stops the job instead of failing every request.
  ATH_CHECK( m_inputs.retrieve() );
  return StatusCode::SUCCESS;
}

StatusCode RemoteExecGateAlg::execute( const EventContext& ctx ) const
{
  SG::ReadHandle<RemoteExecRequestDescriptor> request( m_request, ctx );
  if ( !request.isValid() ) {
    ATH_MSG_ERROR( "No request under '" << m_request.key()
                                        << "'; the loop manager should have "
                                           "recorded one" );
    return StatusCode::FAILURE;
  }

  if ( request->sequence() != m_sequence ) {
    // Not for us. The enclosing seqAND stops here, so nothing downstream runs
    // and none of this fragment's inputs are ever produced.
    setFilterPassed( false, ctx );
    return StatusCode::SUCCESS;
  }

  setFilterPassed( true, ctx );
  ATH_MSG_DEBUG( "Selected by request " << request->requestId() << " on slot "
                                        << ctx.slot() );

  if ( m_inputs.empty() ) {
    return StatusCode::SUCCESS;
  }

  for ( const ToolHandle<IPayloadCodec>& codec : m_inputs ) {
    const Payload* payload = request->find( codec->key() );
    if ( payload == nullptr ) {
      ATH_MSG_ERROR( "Request " << request->requestId() << " for '"
                                << request->sequence() << "' does not supply '"
                                << codec->key() << "'" );
      return StatusCode::FAILURE;
    }
    ATH_CHECK( codec->record( ctx, *payload ) );
    ATH_MSG_DEBUG( "Unpacked " << describe( *payload ) << " into '"
                               << codec->key() << "'" );
  }
  return StatusCode::SUCCESS;
}

}  // namespace AthExRemoteExec
