/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RpcGateAlg.h"

#include "AthenaKernel/ExtendedEventContext.h"
#include "AthenaKernel/IProxyDict.h"
#include "StoreGate/ReadHandle.h"

namespace AthExRpc {

StatusCode RpcGateAlg::initialize()
{
  if ( m_sequence.empty() ) {
    ATH_MSG_ERROR( "SequenceName is not set; this gate would never select "
                   "anything" );
    return StatusCode::FAILURE;
  }
  ATH_CHECK( m_request.initialize() );

  if ( !m_inputs.empty() ) {
    ATH_CHECK( m_clidSvc.retrieve() );
    std::string error;
    // Resolved once here rather than per request, so a typo in a fragment
    // declaration stops the job instead of failing every request.
    if ( PayloadStore::resolve( m_inputs, *m_clidSvc, m_boundaries, error )
             .isFailure() ) {
      ATH_MSG_ERROR( error );
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode RpcGateAlg::execute( const EventContext& ctx ) const
{
  SG::ReadHandle<RpcRequestDescriptor> request( m_request, ctx );
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

  if ( m_boundaries.empty() ) {
    return StatusCode::SUCCESS;
  }

  // The store comes from the context rather than from evtStore(), so this does
  // not depend on which slot happens to be selected on this thread.
  IProxyDict* store = Atlas::getExtendedEventContext( ctx ).proxy();
  if ( store == nullptr ) {
    ATH_MSG_ERROR( "No proxy dictionary in the event context" );
    return StatusCode::FAILURE;
  }

  for ( const PayloadStore::Resolved& boundary : m_boundaries ) {
    const Payload* payload = request->find( boundary.boundary.key );
    if ( payload == nullptr ) {
      ATH_MSG_ERROR( "Request " << request->requestId() << " for '"
                                << request->sequence() << "' does not supply '"
                                << boundary.boundary.key << "'" );
      return StatusCode::FAILURE;
    }
    std::string error;
    if ( PayloadStore::record( *store, *m_clidSvc, boundary, *payload, error )
             .isFailure() ) {
      ATH_MSG_ERROR( error );
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG( "Unpacked " << describe( *payload ) << " into '"
                               << boundary.boundary.key << "'" );
  }
  return StatusCode::SUCCESS;
}

}  // namespace AthExRpc
