/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "DelayAlg.h"

#include <chrono>
#include <thread>

namespace AthExRpc {

StatusCode DelayAlg::execute( const EventContext& ctx ) const
{
  if ( m_milliseconds > 0 ) {
    ATH_MSG_DEBUG( "Sleeping " << m_milliseconds.value() << " ms on slot "
                               << ctx.slot() );
    std::this_thread::sleep_for( std::chrono::milliseconds( m_milliseconds ) );
  }
  return StatusCode::SUCCESS;
}

}  // namespace AthExRpc
