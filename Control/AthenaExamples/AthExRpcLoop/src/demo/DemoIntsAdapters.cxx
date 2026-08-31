/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "DemoIntsAdapters.h"

#include "DemoSchema.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include <algorithm>
#include <memory>

namespace AthExRpc {

StatusCode DemoIntsUnpackAlg::initialize()
{
  ATH_CHECK( m_payload.initialize() );
  ATH_CHECK( m_values.initialize() );
  if ( m_values.empty() ) {
    ATH_MSG_ERROR( "Values is empty; this algorithm would decode a payload "
                   "and record nothing" );
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode DemoIntsUnpackAlg::execute( const EventContext& ctx ) const
{
  SG::ReadHandle<RpcBlob> payload( m_payload, ctx );
  if ( !payload.isValid() ) {
    ATH_MSG_ERROR( "No payload under '" << m_payload.key() << "'" );
    return StatusCode::FAILURE;
  }
  // Protobuf will happily parse one message as another and hand back something
  // empty, so the schema is checked rather than assumed. The blob carries what
  // the boundary declared, which is what the sender encoded against.
  if ( payload->schema() != DemoSchema::ints ) {
    ATH_MSG_ERROR( "'" << m_payload.key() << "' holds a '" << payload->schema()
                       << "', not a '" << DemoSchema::ints << "'" );
    return StatusCode::FAILURE;
  }

  std::vector<DemoSchema::NamedInt> values;
  std::string error;
  if ( !DemoSchema::decodeInts( payload->bytes(), values, error ) ) {
    ATH_MSG_ERROR( "'" << m_payload.key() << "': " << error );
    return StatusCode::FAILURE;
  }

  for ( SG::WriteHandle<int64_t>& handle : m_values.makeHandles( ctx ) ) {
    const auto found =
        std::find_if( values.begin(), values.end(),
                      [&handle]( const DemoSchema::NamedInt& candidate ) {
                        return candidate.name == handle.key();
                      } );
    if ( found == values.end() ) {
      ATH_MSG_ERROR( "'" << m_payload.key() << "' carries no entry named '"
                         << handle.key() << "'" );
      return StatusCode::FAILURE;
    }
    ATH_CHECK( handle.record( std::make_unique<int64_t>( found->value ) ) );
    ATH_MSG_DEBUG( "Unpacked " << handle.key() << " = " << found->value );
  }
  return StatusCode::SUCCESS;
}

StatusCode DemoIntsPackAlg::initialize()
{
  ATH_CHECK( m_values.initialize() );
  ATH_CHECK( m_payload.initialize() );
  if ( m_values.empty() ) {
    ATH_MSG_ERROR( "Values is empty; this algorithm would send nothing" );
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode DemoIntsPackAlg::execute( const EventContext& ctx ) const
{
  std::vector<DemoSchema::NamedInt> values;
  for ( SG::ReadHandle<int64_t>& handle : m_values.makeHandles( ctx ) ) {
    if ( !handle.isValid() ) {
      ATH_MSG_ERROR( "Nothing under '" << handle.key() << "'" );
      return StatusCode::FAILURE;
    }
    values.push_back( DemoSchema::NamedInt{handle.key(), *handle} );
  }

  std::string bytes;
  std::string error;
  if ( !DemoSchema::encodeInts( values, bytes, error ) ) {
    ATH_MSG_ERROR( error );
    return StatusCode::FAILURE;
  }

  SG::WriteHandle<RpcBlob> payload( m_payload, ctx );
  ATH_CHECK( payload.record( std::make_unique<RpcBlob>(
      DemoSchema::ints, std::move( bytes ) ) ) );
  ATH_MSG_DEBUG( "Packed " << values.size() << " integer(s) into '"
                           << m_payload.key() << "'" );
  return StatusCode::SUCCESS;
}

}  // namespace AthExRpc
