/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "DemoDoublesCodec.h"

#include "DemoSchema.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include <memory>

namespace AthExRpc {

const std::string& DemoDoublesCodec::encoding() const
{
  static const std::string tag = "protobuf";
  return tag;
}

StatusCode DemoDoublesCodec::initializeBoundary()
{
  if ( schema() != DemoSchema::doubles ) {
    ATH_MSG_ERROR( "Schema is '" << schema() << "'; this codec implements '"
                                 << DemoSchema::doubles << "'" );
    return StatusCode::FAILURE;
  }

  ATH_CHECK( m_decoded.initialize( writing() ) );
  ATH_CHECK( m_encoded.initialize( !writing() ) );

  const bool haveKey =
      writing() ? !m_decoded.key().empty() : !m_encoded.key().empty();
  if ( !haveKey ) {
    ATH_MSG_ERROR( "Direction is '" << ( writing() ? "Write" : "Read" )
                                    << "' but the matching key is not set; "
                                       "this codec would carry nothing" );
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode DemoDoublesCodec::doRecord( const EventContext& ctx,
                                       IProxyDict& /*store*/,
                                       const Payload& payload ) const
{
  auto values = std::make_unique<std::vector<double>>();
  std::string name;
  std::string error;
  if ( !DemoSchema::decodeDoubles( payload.data, name, *values, error ) ) {
    ATH_MSG_ERROR( "'" << key() << "': " << error );
    return StatusCode::FAILURE;
  }

  SG::WriteHandle<std::vector<double>> handle( m_decoded, ctx );
  ATH_MSG_DEBUG( "Unpacked " << values->size() << " value(s) named '" << name
                             << "' into '" << handle.key() << "'" );
  ATH_CHECK( handle.record( std::move( values ) ) );
  return StatusCode::SUCCESS;
}

StatusCode DemoDoublesCodec::doRead(
    const EventContext& ctx, IProxyDict& /*store*/, Payload& payload,
    const std::vector<const IPayloadCodec*>& /*alsoCrossing*/,
    std::vector<std::string>& /*dangling*/ ) const
{
  SG::ReadHandle<std::vector<double>> values( m_encoded, ctx );
  if ( !values.isValid() ) {
    ATH_MSG_ERROR( "'" << key() << "': nothing under '" << m_encoded.key()
                       << "'" );
    return StatusCode::FAILURE;
  }

  std::string error;
  if ( !DemoSchema::encodeDoubles( m_encoded.key(), *values, payload.data,
                                   error ) ) {
    ATH_MSG_ERROR( "'" << key() << "': " << error );
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG( "Packed " << values->size() << " value(s) into '" << key()
                           << "'" );
  return StatusCode::SUCCESS;
}

}  // namespace AthExRpc
