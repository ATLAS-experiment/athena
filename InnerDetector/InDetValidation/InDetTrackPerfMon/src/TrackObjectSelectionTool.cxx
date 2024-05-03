/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    InDetTrackObjectSelectionTool.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch>
 **/

/// Gaudi includes
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Service.h"

/// Local includes
#include "TrackObjectSelectionTool.h"
#include "TrackAnalysisCollections.h"
#include "OfflineObjectDecorHelper.h"
#include "TrackParmetersHelper.h"


///----------------------------------------
///------- Parametrized constructor -------
///----------------------------------------
IDTPM::TrackObjectSelectionTool::TrackObjectSelectionTool( 
    const std::string& name ) :
  asg::AsgTool( name ) { }


///--------------------------
///------- Initialize -------
///--------------------------
StatusCode IDTPM::TrackObjectSelectionTool::initialize() {

  ATH_CHECK( asg::AsgTool::initialize() );

  ATH_MSG_INFO( "Initializing " << name() << "..." );

  return StatusCode::SUCCESS;
}


///-------------------------
///----- selectTracks ------
///-------------------------
StatusCode IDTPM::TrackObjectSelectionTool::selectTracks(
    TrackAnalysisCollections& trkAnaColls ) {

  ATH_MSG_DEBUG( "Selecting offline tracks matched to an offline " << m_objectType.value() );

  ITrackAnalysisDefinitionSvc* trkAnaDefSvc( nullptr );
  ISvcLocator* svcLoc = Gaudi::svcLocator();
  ATH_CHECK( svcLoc->service( "TrkAnaDefSvc" + trkAnaColls.anaTag(), trkAnaDefSvc ) );

  if( not trkAnaDefSvc->useOffline() ) {
    ATH_MSG_DEBUG( "Tool not enabled if offline tracks are not used." );
    return StatusCode::SUCCESS;
  }

  /// started loop over offline tracks
  std::vector< const xAOD::TrackParticle* > newVec;
  for( const xAOD::TrackParticle* thisTrack :
       trkAnaColls.offlTrackVec( TrackAnalysisCollections::FS ) ) {
    if( accept( *thisTrack ) ) newVec.push_back( thisTrack );
  }

  /// update selected Full-Scan offline track vector
  ATH_CHECK( trkAnaColls.fillOfflTrackVec( newVec,
                TrackAnalysisCollections::FS ) );

  /// Debug printout
  ATH_MSG_DEBUG( "Tracks after offline object-matching: " << 
      trkAnaColls.printInfo( TrackAnalysisCollections::FS ) );

  return StatusCode::SUCCESS;
}


///-----------------------
///------- accept --------
///-----------------------
bool IDTPM::TrackObjectSelectionTool::accept(
    const xAOD::TrackParticle& offTrack ) const {

  /// Electron
  if( m_objectType.value().find("Electron") != std::string::npos ) {

    const xAOD::Electron* ele = getLinkedElectron(
        offTrack, m_objectQuality.value() );

    if( not ele ) return false;

    ATH_MSG_DEBUG( "Offline Track with pt = " << pT( offTrack ) <<
                   " matches with " << m_objectQuality.value() << 
                   " Electron with transverse energy = " << eT( *ele ) );

    return true;
  }

  /// Muon
  if( m_objectType.value().find("Muon") != std::string::npos ) {
    const xAOD::Muon* mu = getLinkedMuon(
        offTrack, m_objectQuality.value() );

    if( not mu ) return false;

    ATH_MSG_DEBUG( "Offline Track with pt = " << pT( offTrack ) <<
                   " matches with " << m_objectQuality.value() <<
                   " Muon with transverse energy = " << eT( *mu ) );

    return true;
  }

  /// Tau
  if( m_objectType.value().find("Tau") != std::string::npos ) {

    const xAOD::TauJet* tau = getLinkedTau(
        offTrack, m_tauNprongs.value(),
        m_tauType.value(), m_objectQuality.value() );

    if( not tau ) return false;

    ATH_MSG_DEBUG( "Offline Track with pt = " << pT( offTrack ) <<
                   " matches with " << m_objectQuality.value() <<
                   " hadronic " << m_tauNprongs.value() << "prong " <<
                   m_tauType.value() << " Tau with transverse energy = " <<
                   eT( *tau ) );

    return true;
  }

  /// Truth
  if( m_objectType.value().find("Truth") != std::string::npos ) {

    const xAOD::TruthParticle* truth = getLinkedTruth(
        offTrack, m_truthProbCut.value() );

    if( not truth ) return false;

    ATH_MSG_DEBUG( "Offline Track with pt = " << pT( offTrack ) <<
                   " matches with truth particle with pt = " << pT( *truth ) );

    return true;
  }

  ATH_MSG_WARNING( "Type " << m_objectType.value() <<
                   " is not supported for track-object selection" );
  return false;
}
