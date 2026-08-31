/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "OffsetAlg.h"

#include "SGTools/BuiltinsClids.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include <memory>

namespace AthExRpc {

StatusCode OffsetAlg::initialize()
{
  ATH_CHECK( m_input.initialize() );
  ATH_CHECK( m_offset.initialize() );
  ATH_CHECK( m_output.initialize() );
  return StatusCode::SUCCESS;
}

StatusCode OffsetAlg::execute( const EventContext& ctx ) const
{
  SG::ReadHandle<int64_t> input( m_input, ctx );
  if ( !input.isValid() ) {
    ATH_MSG_ERROR( "Missing input " << m_input.key() );
    return StatusCode::FAILURE;
  }

  SG::ReadCondHandle<RpcCondData> offset( m_offset, ctx );
  if ( !offset.isValid() ) {
    ATH_MSG_ERROR( "No conditions object " << m_offset.key() << " for "
                                           << ctx.eventID() );
    return StatusCode::FAILURE;
  }

  SG::WriteHandle<int64_t> output( m_output, ctx );
  ATH_CHECK( output.record(
      std::make_unique<int64_t>( *input + offset->offset() ) ) );
  ATH_MSG_DEBUG( *input << " + " << offset->offset() << " = " << *output );
  return StatusCode::SUCCESS;
}

}  // namespace AthExRpc
