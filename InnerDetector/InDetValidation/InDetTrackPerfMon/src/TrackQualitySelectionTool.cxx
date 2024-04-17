/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    TrackQualitySelectionTool.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch>
 **/

/// Local include(s)
#include "TrackQualitySelectionTool.h"
#include "TrackAnalysisCollections.h"

/// Gaudi includes
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Service.h"


///----------------------------------------
///------- Parametrized constructor -------
///----------------------------------------
IDTPM::TrackQualitySelectionTool::TrackQualitySelectionTool( 
    const std::string& name ) :
  asg::AsgTool( name ) { }


///--------------------------
///------- Initialize -------
///--------------------------
StatusCode IDTPM::TrackQualitySelectionTool::initialize() {

  ATH_CHECK( asg::AsgTool::initialize() );

  ATH_MSG_INFO( "Initializing " << name() );

  ATH_CHECK( m_objSelectionTool.retrieve( EnableTool{ m_doObjSelection.value() } ) );

  return StatusCode::SUCCESS;
}


///-------------------------
///----- selectTracks ------
///-------------------------
StatusCode IDTPM::TrackQualitySelectionTool::selectTracks(
    TrackAnalysisCollections& trkAnaColls ) {

  ATH_MSG_DEBUG( "Initially copying collections to FullScan vectors" );

  ITrackAnalysisDefinitionSvc* trkAnaDefSvc( nullptr );
  ISvcLocator* svcLoc = Gaudi::svcLocator();
  ATH_CHECK( svcLoc->service( "TrkAnaDefSvc" + trkAnaColls.anaTag(), trkAnaDefSvc ) );

  /// First copy the full collections vectors to the selected vectors (Full-Scan)
  if( trkAnaDefSvc->useOffline() ) {
    ATH_CHECK( trkAnaColls.fillOfflTrackVec(
        trkAnaColls.offlTrackVec( TrackAnalysisCollections::FULL ),
        TrackAnalysisCollections::FS ) );
  }

  if( trkAnaDefSvc->useTruth() ) {
    ATH_CHECK( trkAnaColls.fillTruthPartVec(
        trkAnaColls.truthPartVec( TrackAnalysisCollections::FULL ),
        TrackAnalysisCollections::FS ) );
  }

  /// Debug printout
  ATH_MSG_DEBUG( "Tracks after initial FullScan copy: " << 
      trkAnaColls.printInfo( TrackAnalysisCollections::FS ) );

  /// Select offline tracks matched to offline objects
  if( trkAnaDefSvc->useOffline() and m_doObjSelection.value() ) {
    ATH_CHECK( m_objSelectionTool->selectTracks( trkAnaColls ) );
  }

  /// TODO - put offline and truth selections here...

  return StatusCode::SUCCESS;
}
