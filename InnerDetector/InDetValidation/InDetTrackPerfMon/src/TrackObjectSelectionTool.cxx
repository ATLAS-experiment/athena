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
#include "TrackParametersHelper.h"


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

  ATH_MSG_DEBUG( "Initializing " << name() << "..." );

  return StatusCode::SUCCESS;
}


///-------------------------
///----- selectTracks ------
///-------------------------
StatusCode IDTPM::TrackObjectSelectionTool::selectTracks(
    TrackAnalysisCollections& trkAnaColls ) {

  ATH_MSG_DEBUG( "Selecting offline tracks matched to an offline " << m_objectType.value() );

  ISvcLocator* svcLoc = Gaudi::svcLocator();
  SmartIF<ITrackAnalysisDefinitionSvc> trkAnaDefSvc( svcLoc->service( "TrkAnaDefSvc" + trkAnaColls.anaTag() ) );
  ATH_CHECK( trkAnaDefSvc.isValid() );

  if( not trkAnaDefSvc->useOffline() ) {
    ATH_MSG_DEBUG( "Tool not enabled if offline tracks are not used." );
    return StatusCode::SUCCESS;
  }

  /// started loop over offline tracks
  std::vector< const xAOD::TrackParticle* > newVec;
  for( const xAOD::TrackParticle* thisTrack :
       trkAnaColls.offlTrackVec( TrackAnalysisCollections::FS ) ) {
    if( accept( *thisTrack,
                trkAnaColls.truthPartVec( TrackAnalysisCollections::FS ) ) ) newVec.push_back( thisTrack );
  }

  /// update selected Full-Scan offline track vector
  ATH_CHECK( trkAnaColls.fillOfflTrackVec( newVec,
                TrackAnalysisCollections::FS ) );

  /// Do the truth-link selection (EF) Trigger, too
  if( trkAnaDefSvc->useEFTrigger() and
      m_objectType.value().find("Truth") != std::string::npos ) {

    ATH_MSG_DEBUG( "Now doing EFTrack-truth selection" );

    /// started loop over trigger tracks
    newVec.clear();
    for( const xAOD::TrackParticle* thisTrack :
         trkAnaColls.trigTrackVec( TrackAnalysisCollections::FS ) ) {
      if( accept( *thisTrack,
                  trkAnaColls.truthPartVec( TrackAnalysisCollections::FS ) ) ) newVec.push_back( thisTrack );
    }

    /// update selected Full-Scan offline track vector
    ATH_CHECK( trkAnaColls.fillTrigTrackVec( newVec,
                  TrackAnalysisCollections::FS ) );
  } // end if

  return StatusCode::SUCCESS;
}


///-----------------------
///------- accept --------
///-----------------------
bool IDTPM::TrackObjectSelectionTool::accept(
    const xAOD::TrackParticle& offTrack,
    const std::vector< const xAOD::TruthParticle* >& truthVec ) const {

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

    if( truthVec.empty() ) {
      ATH_MSG_DEBUG( "Truth vector is empty" );
      return false;
    }

    ATH_MSG_DEBUG( "Offline Track with pt = " << pT( offTrack ) <<
                   " matches with truth particle with pt = " << pT( *truth ) );

    /// Skip linked not-selected truth particles 
    if( std::find( truthVec.begin(), truthVec.end(),
                   truth ) == truthVec.end() ) return false;

    return true;
  }

  ATH_MSG_WARNING( "Type " << m_objectType.value() <<
                   " is not supported for track-object selection" );
  return false;
}
