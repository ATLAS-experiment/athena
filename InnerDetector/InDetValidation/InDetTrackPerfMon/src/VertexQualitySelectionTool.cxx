/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    VertexQualitySelectionTool.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch>
 **/

/// Local include(s)
#include "VertexQualitySelectionTool.h"
#include "TrackAnalysisCollections.h"

/// Gaudi includes
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Service.h"


///----------------------------------------
///------- Parametrized constructor -------
///----------------------------------------
IDTPM::VertexQualitySelectionTool::VertexQualitySelectionTool( 
    const std::string& name ) :
  asg::AsgTool( name ) { }


///--------------------------
///------- Initialize -------
///--------------------------
StatusCode IDTPM::VertexQualitySelectionTool::initialize() {

  ATH_CHECK( asg::AsgTool::initialize() );

  ATH_MSG_DEBUG( "Initializing " << name() );

  /// TODO: include extra selection tools here

  return StatusCode::SUCCESS;
}


///---------------------------
///----- selectVertices ------
///---------------------------
StatusCode IDTPM::VertexQualitySelectionTool::selectVertices(
    TrackAnalysisCollections& trkAnaColls ) {

  ATH_MSG_DEBUG( "Initially copying vertex collections to FullScan vectors" );

  ISvcLocator* svcLoc = Gaudi::svcLocator();
  SmartIF< ITrackAnalysisDefinitionSvc > trkAnaDefSvc(
      svcLoc->service( "TrkAnaDefSvc" + trkAnaColls.anaTag() ) );
  ATH_CHECK( trkAnaDefSvc.isValid() );

  /// First copy the full collections vectors to the selected vectors (Full-Scan)
  if( trkAnaDefSvc->useOffline() ) {
    /// Offline vertices copy
    ATH_CHECK( trkAnaColls.fillOfflVertexVec(
        trkAnaColls.offlVertexVec( TrackAnalysisCollections::FULL ),
        TrackAnalysisCollections::FS ) );
  }

  if( trkAnaDefSvc->useTrigger() or trkAnaDefSvc->useEFTrigger() ) {
    /// Trigger vertices (or EFTrigger, i.e. Trigger vertices
    /// without the trigger navigation / offline-like) copy
    ATH_CHECK( trkAnaColls.fillTrigVertexVec(
        trkAnaColls.trigVertexVec( TrackAnalysisCollections::FULL ),
        TrackAnalysisCollections::FS ) );
  }

  if( trkAnaDefSvc->useTruth() ) {
    /// Truth vertices copy
    ATH_CHECK( trkAnaColls.fillTruthVertexVec(
        trkAnaColls.truthVertexVec( TrackAnalysisCollections::FULL ),
        TrackAnalysisCollections::FS ) );
  }

  /// Debug printout
  ATH_MSG_DEBUG( "Vertices after initial FullScan copy: " << 
      trkAnaColls.printVertexInfo( IDTPM::TrackAnalysisCollections::FS ) );

  /// TODO: include extra selection tools here

  return StatusCode::SUCCESS;
}
