/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    VertexRoiSelectionTool.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch>
 **/

/// Athena include(s)
#include "TrigSteeringEvent/TrigRoiDescriptor.h"

/// Gaudi includes
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Service.h"

/// Local include(s)
#include "InDetTrackPerfMon/VertexRoiSelectionTool.h"
#include "InDetTrackPerfMon/TrackAnalysisCollections.h"
#include "InDetTrackPerfMon/TrackParametersHelper.h" // includes VertexParametersHelper.h

/// STD includes
#include <algorithm> // for std::find


///----------------------------------------
///------- Parametrized constructor -------
///----------------------------------------
IDTPM::VertexRoiSelectionTool::VertexRoiSelectionTool(
    const std::string& name ) :
  asg::AsgTool( name ) { }


///--------------------------
///------- Initialize -------
///--------------------------
StatusCode IDTPM::VertexRoiSelectionTool::initialize() {

  ATH_CHECK( asg::AsgTool::initialize() );

  ATH_MSG_DEBUG( "Initializing " << name() );

  ATH_CHECK( m_triggerVertexContainerName.initialize( 
      ! m_triggerVertexContainerName.key().empty() ) );

  ATH_CHECK( m_trigDecTool.retrieve() );

  return StatusCode::SUCCESS;
}


///-----------------------
///------- accept --------
///-----------------------
template< class V >
bool IDTPM::VertexRoiSelectionTool::accept(
    const V& v, const TrigRoiDescriptor* r ) const
{
  if( r==0 ) {
    ATH_MSG_ERROR( "Called with null RoiDescriptor" );
    return true;
  }

  if( r->composite() ) {
    for( unsigned i=r->size() ; i-- ; )
      if( accept( v, (const TrigRoiDescriptor*)r->at(i) ) ) {
        return true;
      }

  } else {
    if( r->isFullscan() ) return true;

    bool contained_zed = posZ(v) >= r->zedMinus() && 
                         posZ(v) <= r->zedPlus();

    if( contained_zed ) return true;
  }

  return false;
}

/// accept method for offline vertices
template bool IDTPM::VertexRoiSelectionTool::accept(
    const xAOD::Vertex& v, const TrigRoiDescriptor* r ) const;

/// accept method for truth vertices
template bool IDTPM::VertexRoiSelectionTool::accept(
    const xAOD::TruthVertex& v, const TrigRoiDescriptor* r ) const;


///-----------------------
///----- getVertices -----
///-----------------------
template< class V >
std::vector< const V* > IDTPM::VertexRoiSelectionTool::getVertices(
    const std::vector< const V* >& vvec,
    const TrigRoiDescriptor* r ) const {

  std::vector< const V* > selectedVertices;

  for( const V* thisVertex : vvec ) {
    if( accept<V>( *thisVertex, r ) ) {
      selectedVertices.push_back( thisVertex );
    }
  }

  return selectedVertices;
}

/// getVertices method for offline vertices
template std::vector< const xAOD::Vertex* >
IDTPM:: VertexRoiSelectionTool::getVertices< xAOD::Vertex >(
    const std::vector< const xAOD::Vertex* >& vvec,
    const TrigRoiDescriptor* r ) const;

/// getVertices method for truth vertices
template std::vector< const xAOD::TruthVertex* >
IDTPM::VertexRoiSelectionTool::getVertices< xAOD::TruthVertex >(
    const std::vector< const xAOD::TruthVertex* >& vvec,
    const TrigRoiDescriptor* r ) const;


///---------------------------
///----- getTrigVertices -----
///---------------------------
std::vector< const xAOD::Vertex* >
IDTPM::VertexRoiSelectionTool::getTrigVertices( 
    const std::vector< const xAOD::Vertex* >& vvec,
    const ElementLink< TrigRoiDescriptorCollection >& roiLink ) const
{
  /// Getting trigger vertex collection handle
  SG::ReadHandle< xAOD::VertexContainer > handle( m_triggerVertexContainerName );

  /// Retrieving ALL trigger vertices within the RoI
  std::pair< xAOD::VertexContainer::const_iterator,
             xAOD::VertexContainer::const_iterator > trigVtxItrPair =
                 m_trigDecTool->associateToEventView( handle, roiLink );

  /// Getting SELECTED trigger vertices within the RoI
  std::vector< const xAOD::Vertex* > selectedTrigVertices;
  xAOD::VertexContainer::const_iterator vtxItr;
  for( vtxItr = trigVtxItrPair.first ; vtxItr != trigVtxItrPair.second ; ++vtxItr ) {
    /// Check if in-RoI vertex is also in the selected (full-scan) trigger vertex vector
    /// i.e. if it passes the quality selection (if any)
    if( std::find( vvec.begin(), vvec.end(), *vtxItr ) == vvec.end() ) {
      ATH_MSG_DEBUG( "Trigger vertex does ! pass quality selection. Skipping." );
      continue;
    }
    selectedTrigVertices.push_back( *vtxItr );
  }

  return selectedTrigVertices;
}


///---------------------------
///--- selectVerticesInRoI ---
///---------------------------
StatusCode IDTPM::VertexRoiSelectionTool::selectVerticesInRoI(
    TrackAnalysisCollections& trkAnaColls,
    const ElementLink< TrigRoiDescriptorCollection >& roiLink ) {

  ATH_MSG_DEBUG( "Selecting vertices in RoI" );

  /// retrieving TrkAnaDefSvc
  ISvcLocator* svcLoc = Gaudi::svcLocator();
  SmartIF< ITrackAnalysisDefinitionSvc > trkAnaDefSvc(
      svcLoc->service( "TrkAnaDefSvc"+trkAnaColls.anaTag() ) );
  ATH_CHECK( trkAnaDefSvc.isValid() );

  const TrigRoiDescriptor* const* roi = roiLink.cptr();

  /// Trigger vertices RoI selection
  /// Filled only if trigger vertex collection is ! empty
  if( ! m_triggerVertexContainerName.key().empty() ) {
    ATH_CHECK( trkAnaColls.fillTrigVertexVec(
        getTrigVertices(
            trkAnaColls.trigVertexVec( TrackAnalysisCollections::FS ),
            roiLink ),
        TrackAnalysisCollections::InRoI ) );
  }

  /// Offline vertices RoI selection
  if( trkAnaDefSvc->useOffline() ) {
    ATH_CHECK( trkAnaColls.fillOfflVertexVec(
        getVertices(
            trkAnaColls.offlVertexVec( TrackAnalysisCollections::FS ),
            *roi ),
        TrackAnalysisCollections::InRoI ) );
  }

  /// Truth vertices RoI selection 
  if( trkAnaDefSvc->useTruth() ) {
    ATH_CHECK( trkAnaColls.fillTruthVertexVec(
        getVertices(
            trkAnaColls.truthVertexVec( TrackAnalysisCollections::FS ),
            *roi ),
        TrackAnalysisCollections::InRoI ) );
  }

  /// Debug printout
  ATH_MSG_DEBUG( "Vertices after RoI selection: " << 
      trkAnaColls.printVertexInfo( TrackAnalysisCollections::InRoI ) );
 
  return StatusCode::SUCCESS;
}
