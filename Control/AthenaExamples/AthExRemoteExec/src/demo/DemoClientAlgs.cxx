/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "DemoClientAlgs.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include <memory>

namespace AthExRemoteExec {

StatusCode DemoNumbersAlg::initialize()
{
  ATH_CHECK( m_values.initialize() );
  if ( m_values.size() != m_offsets.size() ) {
    ATH_MSG_ERROR( "Values has " << m_values.size() << " entries and Offsets "
                                 << m_offsets.size() );
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode DemoNumbersAlg::execute( const EventContext& ctx ) const
{
  const int64_t event = static_cast<int64_t>( ctx.eventID().event_number() );
  size_t index = 0;
  for ( SG::WriteHandle<int64_t>& handle : m_values.makeHandles( ctx ) ) {
    ATH_CHECK( handle.record(
        std::make_unique<int64_t>( m_offsets[index] + event ) ) );
    ++index;
  }
  return StatusCode::SUCCESS;
}

StatusCode DemoCheckAlg::initialize()
{
  ATH_CHECK( m_values.initialize() );
  ATH_CHECK( m_result.initialize() );
  return StatusCode::SUCCESS;
}

StatusCode DemoCheckAlg::execute( const EventContext& ctx ) const
{
  int64_t expected = m_offset;
  for ( SG::ReadHandle<int64_t>& handle : m_values.makeHandles( ctx ) ) {
    if ( !handle.isValid() ) {
      ATH_MSG_ERROR( "Nothing under '" << handle.key() << "'" );
      return StatusCode::FAILURE;
    }
    expected += *handle;
  }

  SG::ReadHandle<int64_t> result( m_result, ctx );
  if ( !result.isValid() ) {
    ATH_MSG_ERROR( "Nothing under '" << m_result.key()
                                     << "'; the reply did not decode" );
    return StatusCode::FAILURE;
  }
  if ( *result != expected ) {
    ATH_MSG_ERROR( "'" << m_result.key() << "' is " << *result << ", expected "
                       << expected << " for event "
                       << ctx.eventID().event_number() );
    return StatusCode::FAILURE;
  }
  ++m_checked;
  return StatusCode::SUCCESS;
}

StatusCode DemoCheckAlg::finalize()
{
  // Counted and said out loud, because a job that ran zero events also fails
  // zero checks -- the failure mode a test grepping for errors cannot see.
  ATH_MSG_INFO( "Checked " << m_checked.load() << " reply/replies" );
  if ( m_checked == 0 ) {
    ATH_MSG_ERROR( "No replies were checked" );
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

}  // namespace AthExRemoteExec
