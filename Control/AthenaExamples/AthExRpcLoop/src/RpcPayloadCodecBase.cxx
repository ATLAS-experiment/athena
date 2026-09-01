/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RpcPayloadCodecBase.h"

#include "AthenaKernel/ExtendedEventContext.h"
#include "AthenaKernel/IProxyDict.h"

namespace AthExRpc {

std::string describeDangling( const std::vector<std::string>& dangling )
{
  std::string list;
  for ( const std::string& entry : dangling ) {
    list += ( list.empty() ? "" : ", " ) + entry;
  }
  return list;
}

StatusCode RpcPayloadCodecBase::initialize()
{
  if ( m_key.empty() ) {
    ATH_MSG_ERROR( "Key is not set; this codec serves one boundary and would "
                   "not know which" );
    return StatusCode::FAILURE;
  }

  // Not a default, because there is no direction that is right more often than
  // the other and a wrong guess declares the boundary to the scheduler the
  // wrong way round -- which shows up as an unmet input somewhere else.
  if ( m_direction == "Write" ) {
    m_writing = true;
  } else if ( m_direction == "Read" ) {
    m_writing = false;
  } else {
    ATH_MSG_ERROR( "Direction is '" << m_direction.value()
                                    << "'; it must be 'Read' or 'Write'" );
    return StatusCode::FAILURE;
  }

  ATH_CHECK( initializeBoundary() );

  ATH_MSG_DEBUG( "Boundary '" << m_key.value() << "' on " << encoding()
                              << ( m_schema.empty()
                                       ? std::string()
                                       : " as " + m_schema.value() )
                              << ", " << m_direction.value() );
  return StatusCode::SUCCESS;
}

StatusCode RpcPayloadCodecBase::record( const EventContext& ctx,
                                        const Payload& payload ) const
{
  if ( payload.encoding != encoding() ) {
    ATH_MSG_ERROR( "'" << m_key.value() << "' arrived with encoding '"
                       << payload.encoding << "' but this boundary declares '"
                       << encoding() << "'" );
    return StatusCode::FAILURE;
  }
  // An empty schema is not a wildcard: it means this encoding names its payload
  // completely, so both ends leave the field empty and this still compares.
  if ( payload.schema != m_schema ) {
    ATH_MSG_ERROR( "'" << m_key.value() << "' arrived as '" << payload.schema
                       << "' but this boundary declares '" << m_schema.value()
                       << "'" );
    return StatusCode::FAILURE;
  }
  IProxyDict* store = storeOf( ctx );
  if ( store == nullptr ) { return StatusCode::FAILURE; }
  return doRecord( ctx, *store, payload );
}

StatusCode RpcPayloadCodecBase::read(
    const EventContext& ctx, Payload& payload,
    const std::vector<const IPayloadCodec*>& alsoCrossing,
    std::vector<std::string>& dangling ) const
{
  IProxyDict* store = storeOf( ctx );
  if ( store == nullptr ) { return StatusCode::FAILURE; }

  payload.key = m_key;
  payload.encoding = encoding();
  payload.schema = m_schema;
  return doRead( ctx, *store, payload, alsoCrossing, dangling );
}

IProxyDict* RpcPayloadCodecBase::storeOf( const EventContext& ctx ) const
{
  // From the context rather than from evtStore(), so this does not depend on
  // which slot happens to be selected on this thread.
  IProxyDict* store = Atlas::getExtendedEventContext( ctx ).proxy();
  if ( store == nullptr ) {
    ATH_MSG_ERROR( "No proxy dictionary in the event context" );
  }
  return store;
}

}  // namespace AthExRpc
