/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    PlotsDefinitionSvc.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch>, Shaun Roe <shaun.roe@cern.ch>
 * @date    19 June 2023
**/

/// Local include(s)
#include "PlotsDefinitionSvc.h"


/// -------------------
/// --- Constructor ---
/// -------------------
IDTPM::PlotsDefinitionSvc::PlotsDefinitionSvc(
    const std::string& name, ISvcLocator* pSvcLocator ) :
        AsgService( name, pSvcLocator ),
        m_plotsDefMap{}, m_nullDef()
{
  declareServiceInterface< IPlotsDefinitionSvc >();
}


/// ------------------
/// --- initialize ---
/// ------------------
StatusCode IDTPM::PlotsDefinitionSvc::initialize() {

  ATH_MSG_DEBUG( "Initialising " << name() );

  ATH_CHECK( m_plotsDefReadTool.retrieve() );

  /// Updating plots definition map
  for( const SinglePlotDefinition& plotDef :
          m_plotsDefReadTool->getPlotsDefinitions() ) {
    ATH_CHECK( update( plotDef ) );
  }

  ATH_MSG_DEBUG( "Number of plots being booked = " << m_plotsDefMap.size() );

  /// Checking validity of plots definitions
  bool allDefsOk( true );
  plotsDefMap_t::iterator map_it;
  for( map_it = m_plotsDefMap.begin(); map_it != m_plotsDefMap.end(); map_it++ ) {
    if( not map_it->second.isValid() ) {
      ATH_MSG_WARNING( "Invalid plot definition: " << map_it->second.plotDigest() );
      allDefsOk = false;
    }
  }
  if( not allDefsOk ) {
    ATH_MSG_WARNING( "Some plots definitions were bad" );
    return StatusCode::RECOVERABLE;
  }

  return StatusCode::SUCCESS;
}


/// ----------------
/// --- finalize ---
/// ----------------
StatusCode IDTPM::PlotsDefinitionSvc::finalize() {
  ATH_MSG_DEBUG( "Finalized " << name() );
  return StatusCode::SUCCESS;
}


/// ------------------
/// --- definition ---
/// ------------------
const IDTPM::SinglePlotDefinition& IDTPM::PlotsDefinitionSvc::definition(
    const std::string& identifier ) const
{
  plotsDefMap_t::const_iterator map_it = m_plotsDefMap.find( identifier );
  if( map_it != m_plotsDefMap.end() ) return map_it->second;
  return m_nullDef; // null plot definition 
}


/// ------------------
/// ----- update -----
/// ------------------
StatusCode IDTPM::PlotsDefinitionSvc::update(
    const IDTPM::SinglePlotDefinition& def )
{
  ATH_MSG_DEBUG( "Adding new plot definition: " << def.identifier() );

  std::pair< plotsDefMap_t::iterator, bool > result =
    m_plotsDefMap.insert( plotsDefMap_t::value_type( def.identifier(), def ) );

  if( not result.second ) {
    ATH_MSG_DEBUG( "Plot definition is already in map. Not added" );
  }

  return StatusCode::SUCCESS;
}
