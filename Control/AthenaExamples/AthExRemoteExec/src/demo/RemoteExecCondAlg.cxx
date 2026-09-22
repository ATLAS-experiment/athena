/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RemoteExecCondAlg.h"

#include "GaudiKernel/EventIDBase.h"
#include "GaudiKernel/EventIDRange.h"
#include "StoreGate/WriteCondHandle.h"

#include <memory>

namespace AthExRemoteExec {

StatusCode RemoteExecCondAlg::initialize()
{
  ATH_CHECK( m_key.initialize() );
  return StatusCode::SUCCESS;
}

StatusCode RemoteExecCondAlg::execute( const EventContext& ctx ) const
{
  SG::WriteCondHandle<RemoteExecCondData> handle( m_key, ctx );
  if ( handle.isValid() ) {
    // Already covered for this event. With several slots in flight the
    // scheduler can still land here; AthCondAlgorithm's isReEntrant()==false
    // makes it rare rather than impossible.
    return StatusCode::SUCCESS;
  }

  const EventIDBase::number_type run = ctx.eventID().run_number();

  // Valid for this run, and only this run. See the header for why this is not
  // IOVInfiniteRange.
  const EventIDBase start{run, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM,
                          EventIDBase::UNDEFNUM, 0, 0};
  const EventIDBase stop{run, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM,
                         EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM - 1, 0};

  const int64_t offset = m_base + static_cast<int64_t>( run );
  ATH_CHECK( handle.record( EventIDRange( start, stop ),
                            std::make_unique<RemoteExecCondData>( offset ) ) );

  ATH_MSG_INFO( "Recorded " << handle.key() << " = " << offset << " for run "
                            << run );
  return StatusCode::SUCCESS;
}

}  // namespace AthExRemoteExec
