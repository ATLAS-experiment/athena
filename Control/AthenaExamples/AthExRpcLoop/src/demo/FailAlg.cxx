/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FailAlg.h"

namespace AthExRpc {

StatusCode FailAlg::execute( const EventContext& /*ctx*/ ) const
{
  ATH_MSG_ERROR( "Failing on purpose" );
  return StatusCode::FAILURE;
}

}  // namespace AthExRpc
