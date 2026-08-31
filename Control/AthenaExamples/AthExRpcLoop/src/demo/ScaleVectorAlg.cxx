/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ScaleVectorAlg.h"

#include "SGTools/StlVectorClids.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include <algorithm>
#include <memory>

namespace AthExRpc {

StatusCode ScaleVectorAlg::initialize()
{
  ATH_CHECK( m_input.initialize() );
  ATH_CHECK( m_output.initialize() );
  return StatusCode::SUCCESS;
}

StatusCode ScaleVectorAlg::execute( const EventContext& ctx ) const
{
  SG::ReadHandle<std::vector<double>> input( m_input, ctx );
  if ( !input.isValid() ) {
    ATH_MSG_ERROR( "Missing input " << m_input.key() );
    return StatusCode::FAILURE;
  }

  auto scaled = std::make_unique<std::vector<double>>( input->size() );
  const double factor = m_factor;
  std::transform( input->begin(), input->end(), scaled->begin(),
                  [factor]( double value ) { return value * factor; } );

  ATH_MSG_DEBUG( "Scaled " << input->size() << " value(s) by " << factor );
  SG::WriteHandle<std::vector<double>> output( m_output, ctx );
  ATH_CHECK( output.record( std::move( scaled ) ) );
  return StatusCode::SUCCESS;
}

}  // namespace AthExRpc
