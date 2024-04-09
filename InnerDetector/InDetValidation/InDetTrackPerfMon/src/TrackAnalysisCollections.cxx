/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   TrackAnalysisCollections.cxx
 * @author Marco Aparo <marco.aparo@cern.ch>
 */

/// local includes
#include "TrackAnalysisCollections.h"
#include "TrackParmetersHelper.h"
#include "TrackMatchingLookup.h"


/// -------------------
/// --- Constructor ---
/// -------------------
IDTPM::TrackAnalysisCollections::TrackAnalysisCollections( 
  const std::string& anaTag ) :
    AthMessaging( "TrackAnalysisCollections"+anaTag ),
    m_anaTag( anaTag ), m_trkAnaDefSvc( nullptr )
{
  m_truthTrackVec.resize( NStages );
  m_offlTrackVec.resize( NStages );
  m_trigTrackVec.resize( NStages );
}

/// ------------------
/// --- initialize ---
/// ------------------
StatusCode IDTPM::TrackAnalysisCollections::initialize()
{
  /// load trkAnaDefSvc
  if( not m_trkAnaDefSvc ) {
    ISvcLocator* svcLoc = Gaudi::svcLocator();
    ATH_CHECK( svcLoc->service( "TrkAnaDefSvc"+m_anaTag, m_trkAnaDefSvc ) );
  }

  /// construct track matching lookup table
  /// based on the types of test and reference
  /// Truth->Track
  if( m_trkAnaDefSvc->isTestTruth() ) {
    m_matches = std::make_unique< TrackMatchingLookup_truthTrk >( m_anaTag );
  }
  /// Track->Truth
  else if( m_trkAnaDefSvc->isReferenceTruth() ) {
    m_matches = std::make_unique< TrackMatchingLookup_trkTruth >( m_anaTag );
  }
  /// Track->Track
  else {
    m_matches = std::make_unique< TrackMatchingLookup_trk >( m_anaTag );
  }

  return StatusCode::SUCCESS;
}

/// ----------------------------
/// --- Fill FULL containers ---
/// ----------------------------
/// Truth particles
StatusCode IDTPM::TrackAnalysisCollections::fillTruthTrackContainer(
  const SG::ReadHandleKey<xAOD::TruthParticleContainer>& handleKey )
{
  if( m_trkAnaDefSvc->useTruth() ) {
    ATH_MSG_DEBUG( "Loading collection: " << handleKey.key() );

    SG::ReadHandle< xAOD::TruthParticleContainer > pColl( handleKey );

    if( not pColl.isValid() ) {
      ATH_MSG_ERROR( "Non valid truth particles collection: " << handleKey.key() );
      return StatusCode::FAILURE;
    }

    /// Fill container
    m_truthTrackContainer = pColl.ptr();

    /// Fill FULL vector
    m_truthTrackVec[ FULL ].clear(); 
    m_truthTrackVec[ FULL ].insert(
      m_truthTrackVec[ FULL ].begin(),
      pColl->begin(), pColl->end() );
  } else {
    m_truthTrackContainer = nullptr;
    m_truthTrackVec[ FULL ].clear();
  }

  return StatusCode::SUCCESS; 
}

/// Offline track particles
StatusCode IDTPM::TrackAnalysisCollections::fillOfflTrackContainer(
  const SG::ReadHandleKey<xAOD::TrackParticleContainer>& handleKey )
{
  if( m_trkAnaDefSvc->useOffline() ) {
    ATH_MSG_DEBUG( "Loading collection: " << handleKey.key() );

    SG::ReadHandle< xAOD::TrackParticleContainer > pColl( handleKey );

    if( not pColl.isValid() ) {
      ATH_MSG_ERROR( "Non valid offline tracks collection: " << handleKey.key() );
      return StatusCode::FAILURE;
    }

    /// Fill container
    m_offlTrackContainer = pColl.ptr();

    /// Fill FULL vector
    m_offlTrackVec[ FULL ].clear(); 
    m_offlTrackVec[ FULL ].insert( 
      m_offlTrackVec[ FULL ].begin(),
      pColl->begin(), pColl->end() );
  } else {
    m_offlTrackContainer = nullptr;
    m_offlTrackVec[ FULL ].clear();
  }

  return StatusCode::SUCCESS; 
}

/// Trigger track particles
StatusCode IDTPM::TrackAnalysisCollections::fillTrigTrackContainer(
  const SG::ReadHandleKey<xAOD::TrackParticleContainer>& handleKey )
{
  if( m_trkAnaDefSvc->useTrigger() ) {
    ATH_MSG_DEBUG( "Loading collection: " << handleKey.key() );

    SG::ReadHandle< xAOD::TrackParticleContainer > pColl( handleKey );

    if( not pColl.isValid() ) {
      ATH_MSG_ERROR( "Non valid trigger tracks collection: " << handleKey.key() );
      return StatusCode::FAILURE;
    }

    /// Fill container
    m_trigTrackContainer = pColl.ptr();

    /// Fill FULL vector
    m_trigTrackVec[ FULL ].clear(); 
    m_trigTrackVec[ FULL ].insert( 
      m_trigTrackVec[ FULL ].begin(),
      pColl->begin(), pColl->end() );
  } else {
    m_trigTrackContainer = nullptr;
    m_trigTrackVec[ FULL ].clear();
  }

  return StatusCode::SUCCESS; 
}

/// -------------------------
/// --- Fill TEST vectors ---
/// -------------------------
/// TEST = truth
StatusCode IDTPM::TrackAnalysisCollections::fillTestTruthVec(
  const std::vector< const xAOD::TruthParticle* >& vec,
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->isTestTruth() ) {
    m_truthTrackVec[ stage ].clear(); 
    m_truthTrackVec[ stage ].insert( 
      m_truthTrackVec[ stage ].begin(),
      vec.begin(), vec.end() );
    return StatusCode::SUCCESS; 
  }

  ATH_MSG_DEBUG( "No TEST TruthParticle vector" );
  return StatusCode::SUCCESS; 
}

/// TEST = tracks
StatusCode IDTPM::TrackAnalysisCollections::fillTestTrackVec(
  const std::vector< const xAOD::TrackParticle* >& vec,
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->isTestOffline() ) {
    m_offlTrackVec[ stage ].clear(); 
    m_offlTrackVec[ stage ].insert( 
      m_offlTrackVec[ stage ].begin(),
      vec.begin(), vec.end() );
    return StatusCode::SUCCESS; 
  }

  if( m_trkAnaDefSvc->isTestTrigger() ) {
    m_trigTrackVec[ stage ].clear(); 
    m_trigTrackVec[ stage ].insert( 
      m_trigTrackVec[ stage ].begin(),
      vec.begin(), vec.end() );
    return StatusCode::SUCCESS; 
  }

  ATH_MSG_DEBUG( "No TEST TrackParticle vector");
  return StatusCode::SUCCESS; 
}

/// REFERENCE = truth
StatusCode IDTPM::TrackAnalysisCollections::fillRefTruthVec(
  const std::vector< const xAOD::TruthParticle* >& vec,
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->isReferenceTruth() ) {
    m_truthTrackVec[ stage ].clear(); 
    m_truthTrackVec[ stage ].insert( 
      m_truthTrackVec[ stage ].begin(),
      vec.begin(), vec.end() );
    return StatusCode::SUCCESS; 
  }

  ATH_MSG_DEBUG( "No REFERENCE TruthParticle vector" );
  return StatusCode::SUCCESS; 
}

/// REFERENCE = tracks
StatusCode IDTPM::TrackAnalysisCollections::fillRefTrackVec(
  const std::vector< const xAOD::TrackParticle* >& vec,
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->isReferenceOffline() ) {
    m_offlTrackVec[ stage ].clear(); 
    m_offlTrackVec[ stage ].insert( 
      m_offlTrackVec[ stage ].begin(),
      vec.begin(), vec.end() );
    return StatusCode::SUCCESS; 
  }

  if( m_trkAnaDefSvc->isReferenceTrigger() ) {
    m_trigTrackVec[ stage ].clear(); 
    m_trigTrackVec[ stage ].insert( 
      m_trigTrackVec[ stage ].begin(),
      vec.begin(), vec.end() );
    return StatusCode::SUCCESS; 
  }

  ATH_MSG_DEBUG( "No REFERENCE TrackParticle vector" );
  return StatusCode::SUCCESS; 
}

/// Truth tracks
StatusCode IDTPM::TrackAnalysisCollections::fillTruthTrackVec(
  const std::vector< const xAOD::TruthParticle* >& vec,
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->useTruth() ) {
    m_truthTrackVec[ stage ].clear();
    m_truthTrackVec[ stage ].insert(
      m_truthTrackVec[ stage ].begin(),
      vec.begin(), vec.end() );
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG( "No TruthParticle vector" );
  return StatusCode::SUCCESS;
}

/// Offline tracks
StatusCode IDTPM::TrackAnalysisCollections::fillOfflTrackVec(
  const std::vector< const xAOD::TrackParticle* >& vec,
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->useOffline() ) {
    m_offlTrackVec[ stage ].clear();
    m_offlTrackVec[ stage ].insert(
      m_offlTrackVec[ stage ].begin(),
      vec.begin(), vec.end() );
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG( "No Offline TrackParticle vector" );
  return StatusCode::SUCCESS;
}

/// Trigger tracks
StatusCode IDTPM::TrackAnalysisCollections::fillTrigTrackVec(
  const std::vector< const xAOD::TrackParticle* >& vec,
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->useTrigger() ) {
    m_trigTrackVec[ stage ].clear();
    m_trigTrackVec[ stage ].insert(
      m_trigTrackVec[ stage ].begin(),
      vec.begin(), vec.end() );
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG( "No Trigger TrackParticle vector" );
  return StatusCode::SUCCESS;
}

/// -----------------------
/// --- Utility methods ---
/// -----------------------
/// empty
bool IDTPM::TrackAnalysisCollections::empty(
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  /// check if empty disabled for FS trigger
  /// track vector (always empty by construction)
  bool isTrigEmpty  = m_trkAnaDefSvc->useTrigger() and 
                      (stage != IDTPM::TrackAnalysisCollections::FS) ?
                      m_trigTrackVec[ stage ].empty() : false;
  bool isOfflEmpty  = m_trkAnaDefSvc->useOffline() ?
                      m_offlTrackVec[ stage ].empty() : false;
  bool isTruthEmpty = m_trkAnaDefSvc->useTruth() ?
                      m_truthTrackVec[ stage ].empty() : false;

  if( isTrigEmpty or isOfflEmpty or isTruthEmpty ) return true;

  return false;
}

/// clear collections
void IDTPM::TrackAnalysisCollections::clear(
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( stage == FULL ) {
    m_truthTrackVec[ FULL ].clear();
    m_offlTrackVec[ FULL ].clear();
    m_trigTrackVec[ FULL ].clear();
  }
  if( stage == FULL or stage == FS ) {
    m_truthTrackVec[ FS ].clear();
    m_offlTrackVec[ FS ].clear();
    m_trigTrackVec[ FS ].clear();
  }
  if( stage == FULL or stage == FS or stage == InRoI ) {
    m_truthTrackVec[ InRoI ].clear();
    m_offlTrackVec[ InRoI ].clear();
    m_trigTrackVec[ InRoI ].clear();
  }
}

/// copy inRoI collections from FullScan collections
void IDTPM::TrackAnalysisCollections::copyFS()
{
  ATH_MSG_DEBUG( "Copying tracks in RoI" );

  /// offline copy
  m_offlTrackVec[ InRoI ].clear(); 
  m_offlTrackVec[ InRoI ].insert( 
    m_offlTrackVec[ InRoI ].begin(),
    m_offlTrackVec[ FS ].begin(),
    m_offlTrackVec[ FS ].end() );

  /// truth copy
  m_truthTrackVec[ InRoI ].clear(); 
  m_truthTrackVec[ InRoI ].insert( 
    m_truthTrackVec[ InRoI ].begin(),
    m_truthTrackVec[ FS ].begin(),
    m_truthTrackVec[ FS ].end() );

  /// Debug printout
  ATH_MSG_DEBUG( "Tracks after in RoI copy: " << printInfo( InRoI ) );
}

/// ---------------------------
/// --- Get FULL containers ---
/// ---------------------------
/// TEST = Truth
const xAOD::TruthParticleContainer*
IDTPM::TrackAnalysisCollections::testTruthContainer()
{
  if( m_trkAnaDefSvc->isTestTruth() ) {
    return m_truthTrackContainer;
  }

  return nullptr;
}

/// TEST = Track
const xAOD::TrackParticleContainer*
IDTPM::TrackAnalysisCollections::testTrackContainer()
{
  if( m_trkAnaDefSvc->isTestOffline() ) {
    return m_offlTrackContainer;
  }
    
  if( m_trkAnaDefSvc->isTestTrigger() ) {
    return m_trigTrackContainer;
  }

  return nullptr;
}

/// REFERENCE = Truth
const xAOD::TruthParticleContainer*
IDTPM::TrackAnalysisCollections::refTruthContainer()
{
  if( m_trkAnaDefSvc->isReferenceTruth() ) {
    return m_truthTrackContainer;
  }

  return nullptr;
}

/// REFERENCE = Track
const xAOD::TrackParticleContainer*
IDTPM::TrackAnalysisCollections::refTrackContainer()
{
  if( m_trkAnaDefSvc->isReferenceOffline() ) {
    return m_offlTrackContainer;
  }
    
  if( m_trkAnaDefSvc->isReferenceTrigger() ) {
    return m_trigTrackContainer;
  }

  return nullptr;
}

/// -------------------------
/// --- Get track vectors ---
/// -------------------------
/// TEST = Truth
const std::vector< const xAOD::TruthParticle* >&
IDTPM::TrackAnalysisCollections::testTruthVec(
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->isTestTruth() ) {
    return m_truthTrackVec[ stage ];
  }

  ATH_MSG_DEBUG( "No Test truth vector found" );
  return m_nullTruthVec;
}

/// TEST = Track
const std::vector< const xAOD::TrackParticle* >&
IDTPM::TrackAnalysisCollections::testTrackVec(
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->isTestOffline() ) {
    return m_offlTrackVec[ stage ];
  }

  if( m_trkAnaDefSvc->isTestTrigger() ) {
    return m_trigTrackVec[ stage ];
  }

  ATH_MSG_DEBUG( "No Test track vector found" );
  return m_nullTrackVec;
}

/// REFERENCE = Truth
const std::vector< const xAOD::TruthParticle* >&
IDTPM::TrackAnalysisCollections::refTruthVec(
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->isReferenceTruth() ) {
    return m_truthTrackVec[ stage ];
  }

  ATH_MSG_DEBUG( "No Reference truth vector found" );
  return m_nullTruthVec;
}

/// TEST = Track
const std::vector< const xAOD::TrackParticle* >&
IDTPM::TrackAnalysisCollections::refTrackVec(
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->isReferenceOffline() ) {
    return m_offlTrackVec[ stage ];
  }

  if( m_trkAnaDefSvc->isReferenceTrigger() ) {
    return m_trigTrackVec[ stage ];
  }

  ATH_MSG_DEBUG( "No Test track vector found" );
  return m_nullTrackVec;
}

/// -----------------------------------------
/// --- Print collection info (for debug) ---
/// -----------------------------------------
/// print tracks
std::string IDTPM::TrackAnalysisCollections::printInfo(
  IDTPM::TrackAnalysisCollections::Stage stage ) const
{
  std::stringstream ss;
  ss << "\n==========================================" << std::endl;

  size_t it(0);
  for( const xAOD::TrackParticle* thisOfflineTrack : m_offlTrackVec[ stage ] ) {
    ss << "Offline track" 
       << " : pt = "  << pT( *thisOfflineTrack )
       << " : eta = " << eta( *thisOfflineTrack )
       << " : phi = " << phi( *thisOfflineTrack )
       << std::endl;
    if( it > 20 ) { ss << "et al...." << std::endl; break; }
    it++;
  }

  if( not m_offlTrackVec[ stage ].empty() )
    ss << "==========================================" << std::endl;

  it = 0;
  for( const xAOD::TruthParticle* thisTruthParticle : m_truthTrackVec[ stage ] ) {
    ss << "Truth particle"
       << " : pt = "  << pT( *thisTruthParticle )
       << " : eta = " << eta( *thisTruthParticle )
       << " : phi = " << phi( *thisTruthParticle )
       << std::endl;
    if( it > 20 ) { ss << "et al...." << std::endl; break; }
    it++;
  }

  if( not m_truthTrackVec[ stage ].empty() )
      ss << "==========================================" << std::endl;

  it = 0;
  for( const xAOD::TrackParticle* thisTriggerTrack : m_trigTrackVec[ stage ] ) {
    ss << "Trigger track"
       << " : pt = "  << pT( *thisTriggerTrack )
       << " : eta = " << eta( *thisTriggerTrack )
       << " : phi = " << phi( *thisTriggerTrack )
       << std::endl;
    if( it > 20 ) { ss << "et al...." << std::endl; break; }
    it++;
  }

  if( not m_trigTrackVec[ stage ].empty() )
    ss << "==========================================" << std::endl;

  return ss.str();
}


/// ----------------------------------------------
/// --- Print matching information (for debug) ---
/// ----------------------------------------------
std::string IDTPM::TrackAnalysisCollections::printMatchInfo()
{
  /// Truth->Track
  if( m_trkAnaDefSvc->isTestTruth() ) {
    return m_matches->printInfo( testTruthVec( InRoI ), refTrackVec( InRoI ) );
  }
  /// Track->Truth
  if( m_trkAnaDefSvc->isReferenceTruth() ) {
    return m_matches->printInfo( testTrackVec( InRoI ), refTruthVec( InRoI ) );
  }
  /// Track->Track
  return m_matches->printInfo( testTrackVec( InRoI ), refTrackVec( InRoI ) );
}


/// --------------------------------
/// --- update chainRoiNames set ---
/// --------------------------------
bool IDTPM::TrackAnalysisCollections::updateChainRois(
    const std::string& chainRoi )
{
  ATH_MSG_DEBUG( "Updating TrackAnalysisCollection with ChainRoiName: " << chainRoi );

  std::pair< std::set<std::string>::iterator, bool > result =
      m_chainRois.insert( chainRoi );

  if( not result.second ) {
    ATH_MSG_WARNING( "ChainRoiName has already been cached. No update." );
    return false;
  }

  /// Creating new matching lookup table for newChainRoiName
  m_matches->clear();
  m_matches->chainRoiName( chainRoi );
  /// m_matches is now ready to be updated with new entries for the maps...

  return result.second;
}
