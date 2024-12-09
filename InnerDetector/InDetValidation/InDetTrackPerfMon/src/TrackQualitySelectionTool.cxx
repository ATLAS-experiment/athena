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

  ATH_CHECK( m_offlineSelectionTool.retrieve( EnableTool{ m_doOfflSelection.value() } ) );
  ATH_CHECK( m_truthSelectionTool.retrieve( EnableTool{ m_doTruthSelection.value() } ) );
  ATH_CHECK( m_objSelectionTool.retrieve( EnableTool{ m_doObjSelection.value() } ) );

  return StatusCode::SUCCESS;
}


///-------------------------
///----- selectTracks ------
///-------------------------
StatusCode IDTPM::TrackQualitySelectionTool::selectTracks(
    TrackAnalysisCollections& trkAnaColls ) {

  ATH_MSG_DEBUG( "Initially copying collections to FullScan vectors" );

  ISvcLocator* svcLoc = Gaudi::svcLocator();
  SmartIF< ITrackAnalysisDefinitionSvc > trkAnaDefSvc(
      svcLoc->service( "TrkAnaDefSvc" + trkAnaColls.anaTag() ) );
  ATH_CHECK( trkAnaDefSvc.isValid() );

  /// First copy the full collections vectors to the selected vectors (Full-Scan)
  if( trkAnaDefSvc->useOffline() ) {
    /// Offline tracks copy
    ATH_CHECK( trkAnaColls.fillOfflTrackVec(
        trkAnaColls.offlTrackVec( TrackAnalysisCollections::FULL ),
        TrackAnalysisCollections::FS ) );
  }

  if( trkAnaDefSvc->useTrigger() or trkAnaDefSvc->useEFTrigger() ) {
    /// Trigger tracks (or EFTrigger, i.e. Trigger tracks without
    /// the trigger navigation / offline-like) copy
    ATH_CHECK( trkAnaColls.fillTrigTrackVec(
        trkAnaColls.trigTrackVec( TrackAnalysisCollections::FULL ),
        TrackAnalysisCollections::FS ) );
  }

  if( trkAnaDefSvc->useTruth() ) {
    /// Truth particles copy
    ATH_CHECK( trkAnaColls.fillTruthPartVec(
        trkAnaColls.truthPartVec( TrackAnalysisCollections::FULL ),
        TrackAnalysisCollections::FS ) );
  }

  /// Debug printout
  ATH_MSG_DEBUG( "Tracks after initial FullScan copy: " << 
      trkAnaColls.printInfo( TrackAnalysisCollections::FS, false ) );

  /// Offline track selection
  if( trkAnaDefSvc->useOffline() and m_doOfflSelection.value() ) {
    ATH_CHECK( m_offlineSelectionTool->selectTracks( trkAnaColls ) );
  }

  /// Truth particles selection
  if( trkAnaDefSvc->useTruth() and m_doTruthSelection.value() ) {
    ATH_CHECK( m_truthSelectionTool->selectTracks( trkAnaColls ) );
  }

  /// TODO: Do Trigger track selection (?)

  /// Select offline tracks matched to offline objects
  if( trkAnaDefSvc->useOffline() and m_doObjSelection.value() ) {
    ATH_CHECK( m_objSelectionTool->selectTracks( trkAnaColls ) );
  }

  /// Debug printout
  ATH_MSG_DEBUG( "Tracks after full quality selection: " << 
      trkAnaColls.printInfo( TrackAnalysisCollections::FS, false ) );

  return StatusCode::SUCCESS;
}
