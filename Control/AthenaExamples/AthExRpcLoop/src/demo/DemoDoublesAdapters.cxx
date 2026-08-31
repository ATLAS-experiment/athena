/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "DemoDoublesAdapters.h"

#include "DemoSchema.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include <memory>
#include <utility>

namespace AthExRpc {

StatusCode DemoDoublesUnpackAlg::initialize()
{
  ATH_CHECK( m_payload.initialize() );
  ATH_CHECK( m_values.initialize() );
  return StatusCode::SUCCESS;
}

StatusCode DemoDoublesUnpackAlg::execute( const EventContext& ctx ) const
{
  SG::ReadHandle<RpcBlob> payload( m_payload, ctx );
  if ( !payload.isValid() ) {
    ATH_MSG_ERROR( "No payload under '" << m_payload.key() << "'" );
    return StatusCode::FAILURE;
  }
  if ( payload->schema() != DemoSchema::doubles ) {
    ATH_MSG_ERROR( "'" << m_payload.key() << "' holds a '" << payload->schema()
                       << "', not a '" << DemoSchema::doubles << "'" );
    return StatusCode::FAILURE;
  }

  std::string name;
  auto values = std::make_unique<std::vector<double>>();
  std::string error;
  if ( !DemoSchema::decodeDoubles( payload->bytes(), name, *values, error ) ) {
    ATH_MSG_ERROR( "'" << m_payload.key() << "': " << error );
    return StatusCode::FAILURE;
  }

  SG::WriteHandle<std::vector<double>> handle( m_values, ctx );
  ATH_MSG_DEBUG( "Unpacked " << values->size() << " value(s) named '" << name
                             << "' into '" << handle.key() << "'" );
  ATH_CHECK( handle.record( std::move( values ) ) );
  return StatusCode::SUCCESS;
}

StatusCode DemoDoublesPackAlg::initialize()
{
  ATH_CHECK( m_values.initialize() );
  ATH_CHECK( m_payload.initialize() );
  return StatusCode::SUCCESS;
}

StatusCode DemoDoublesPackAlg::execute( const EventContext& ctx ) const
{
  SG::ReadHandle<std::vector<double>> values( m_values, ctx );
  if ( !values.isValid() ) {
    ATH_MSG_ERROR( "Nothing under '" << m_values.key() << "'" );
    return StatusCode::FAILURE;
  }

  std::string bytes;
  std::string error;
  if ( !DemoSchema::encodeDoubles( m_values.key(), *values, bytes, error ) ) {
    ATH_MSG_ERROR( error );
    return StatusCode::FAILURE;
  }

  SG::WriteHandle<RpcBlob> payload( m_payload, ctx );
  ATH_CHECK( payload.record( std::make_unique<RpcBlob>(
      DemoSchema::doubles, std::move( bytes ) ) ) );
  ATH_MSG_DEBUG( "Packed " << values->size() << " value(s) into '"
                           << m_payload.key() << "'" );
  return StatusCode::SUCCESS;
}

}  // namespace AthExRpc
