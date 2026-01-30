/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   TrackAnalysisCollections.cxx
 * @author Marco Aparo <marco.aparo@cern.ch>
 */

/// local includes
#include "InDetTrackPerfMon/TrackAnalysisCollections.h"
#include "InDetTrackPerfMon/TrackParametersHelper.h" // also includes VertexParametersHelper.h
#include "InDetTrackPerfMon/TrackMatchingLookup.h"

/// STD include(s)
#include <algorithm> // for std::find


/// -------------------
/// --- Constructor ---
/// -------------------
IDTPM::TrackAnalysisCollections::TrackAnalysisCollections( 
  const std::string& anaTag ) :
    AthMessaging( "TrackAnalysisCollections"+anaTag ),
    m_anaTag( anaTag ), m_newChain( true ), m_newRoi( true )
{
  /// tracks
  m_truthPartVec.resize( NStages );
  m_offlTrackVec.resize( NStages );
  m_trigTrackVec.resize( NStages );

  /// vertices
  m_truthVertexVec.resize( NStages );
  m_offlVertexVec.resize( NStages );
  m_trigVertexVec.resize( NStages );
}

/// ------------------
/// --- initialize ---
/// ------------------
StatusCode IDTPM::TrackAnalysisCollections::initialize()
{
  /// load trkAnaDefSvc
  m_trkAnaDefSvc = Gaudi::svcLocator()->service( "TrkAnaDefSvc"+m_anaTag );
  ATH_CHECK( m_trkAnaDefSvc.isValid() );

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

/// ---------------------------
/// ----- Fill Event Info -----
/// ---------------------------
StatusCode IDTPM::TrackAnalysisCollections::fillEventInfo(
  const SG::ReadHandleKey<xAOD::EventInfo>& eventInfoHandleKey,
  const SG::ReadHandleKey< xAOD::TruthEventContainer >& truthEventHandleKey,
  const SG::ReadHandleKey< xAOD::TruthPileupEventContainer >& truthPUEventHandleKey )
{
  m_eventInfo = nullptr;
  m_truthEventContainer = nullptr;
  m_truthPUEventContainer = nullptr;

  /// EventInfo
  SG::ReadHandle< xAOD::EventInfo > pie( eventInfoHandleKey );
  if( ! pie.isValid() ) {
    ATH_MSG_WARNING( "Shouldn't happen. EventInfo is buggy" );
  } else m_eventInfo = pie.ptr();

  if( ! m_trkAnaDefSvc->useTruth() ) return StatusCode::SUCCESS;

  /// TruthEvent
  SG::ReadHandle< xAOD::TruthEventContainer > pTruthEventCont( truthEventHandleKey );
  if( ! pTruthEventCont.isValid() ) {
    ATH_MSG_WARNING( "Non valid truth event collection: " << truthEventHandleKey.key() );
  }
  else m_truthEventContainer = pTruthEventCont.ptr();

  /// TruthPileupEvent
  if( m_trkAnaDefSvc->hasFullPileupTruth() ) {
    SG::ReadHandle< xAOD::TruthPileupEventContainer > pTruthPUEventCont( truthPUEventHandleKey );
    if( ! pTruthPUEventCont.isValid() ) {
      ATH_MSG_WARNING( "Non valid truth pile up event collection: " << truthPUEventHandleKey.key() );
    }
    else m_truthPUEventContainer = pTruthPUEventCont.ptr();
  }

  return StatusCode::SUCCESS;
}

/// ----------------------------------
/// --- Fill FULL Track containers ---
/// ----------------------------------
/// Truth particles
StatusCode IDTPM::TrackAnalysisCollections::fillTruthPartContainer(
  const SG::ReadHandleKey< xAOD::TruthParticleContainer >& truthPartHandleKey )
{
  if( m_trkAnaDefSvc->useTruth() ) {
    ATH_MSG_DEBUG( "Loading collection: " << truthPartHandleKey.key() );
    SG::ReadHandle< xAOD::TruthParticleContainer > pTruthColl( truthPartHandleKey );
    if( ! pTruthColl.isValid() ) {
      ATH_MSG_ERROR( "Non valid truth particles collection: " << truthPartHandleKey.key() );
      return StatusCode::FAILURE;
    }

    /// Fill container
    m_truthPartContainer = pTruthColl.ptr();

    /// Fill FULL vector
    m_truthPartVec[ FULL ].clear(); // clear to initialize it

    /// Grab the entire truth particle collection
    if( m_trkAnaDefSvc->pileupSwitch() == "All" ) {
      m_truthPartVec[ FULL ].insert(
        m_truthPartVec[ FULL ].begin(), pTruthColl->begin(), pTruthColl->end() );

    /// Grab only truth particles from Hard Scatter
    } else if( m_trkAnaDefSvc->pileupSwitch() == "HardScatter" ) {
      if( m_truthEventContainer ) {
        const xAOD::TruthEvent* event = m_truthEventContainer->at(0);
        const auto& links = event->truthParticleLinks();
        for( const auto& link : links ) {
          if( link.isValid() ) m_truthPartVec[ FULL ].push_back( *link );
        }
      }

    /// Grab only truth particles from Pile Up
    } else if( m_trkAnaDefSvc->pileupSwitch() == "PileUp" ) {
      if( m_truthPUEventContainer ) {
        // loop over all pile up events
        for( size_t ipu=0; ipu < m_truthPUEventContainer->size(); ipu++ ) {
          const xAOD::TruthPileupEvent* eventPU = m_truthPUEventContainer->at( ipu );
          const auto& links = eventPU->truthParticleLinks();
          for( const auto& link : links ) {
            if( link.isValid() ) m_truthPartVec[ FULL ].push_back( *link );
          }
        }
      }

    } else {
      ATH_MSG_ERROR( "Invalid pileupSwitch: " << m_trkAnaDefSvc->pileupSwitch() );
      return StatusCode::FAILURE;
    }
  } else {
    m_truthPartContainer = nullptr;
    m_truthPartVec[ FULL ].clear();
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

    if( ! pColl.isValid() ) {
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
  if( m_trkAnaDefSvc->useTrigger()) {
    ATH_MSG_DEBUG( "Loading collection: " << handleKey.key() );

    SG::ReadHandle< xAOD::TrackParticleContainer > pColl( handleKey );

    if( ! pColl.isValid() ) {
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





/// Offline track particles
StatusCode IDTPM::TrackAnalysisCollections::fillOfflTrackContainer( const std::string& key )
								   
{
  if( m_trkAnaDefSvc->useOffline() ) {
    ATH_MSG_DEBUG( "Loading collection: " << key );

    SG::ReadHandle< xAOD::TrackParticleContainer > pColl( key );

    if( ! pColl.isValid() ) {
      ATH_MSG_ERROR( "Non valid offline tracks collection: " << key );
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
StatusCode IDTPM::TrackAnalysisCollections::fillTrigTrackContainer( const std::string& key )
{
  if( m_trkAnaDefSvc->useTrigger()) {
    ATH_MSG_DEBUG( "Loading collection: " << key );

    SG::ReadHandle< xAOD::TrackParticleContainer > pColl( key );

    if( ! pColl.isValid() ) {
      ATH_MSG_ERROR( "Non valid trigger tracks collection: " << key );
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







/// -------------------------------
/// --- Fill TEST track vectors ---
/// -------------------------------
/// TEST = truth
StatusCode IDTPM::TrackAnalysisCollections::fillTestTruthVec(
  const std::vector< const xAOD::TruthParticle* >& vec,
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->isTestTruth() ) {
    m_truthPartVec[ stage ].clear(); 
    m_truthPartVec[ stage ].insert( 
      m_truthPartVec[ stage ].begin(),
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

/// ------------------------------------
/// --- Fill REFERENCE track vectors ---
/// ------------------------------------
/// REFERENCE = truth
StatusCode IDTPM::TrackAnalysisCollections::fillRefTruthVec(
  const std::vector< const xAOD::TruthParticle* >& vec,
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->isReferenceTruth() ) {
    m_truthPartVec[ stage ].clear(); 
    m_truthPartVec[ stage ].insert( 
      m_truthPartVec[ stage ].begin(),
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

/// --------------------------
/// --- Fill track vectors ---
/// --------------------------
/// Truth tracks
StatusCode IDTPM::TrackAnalysisCollections::fillTruthPartVec(
  const std::vector< const xAOD::TruthParticle* >& vec,
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->useTruth() ) {
    m_truthPartVec[ stage ].clear();
    m_truthPartVec[ stage ].insert(
      m_truthPartVec[ stage ].begin(),
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
  if( m_trkAnaDefSvc->useTrigger() ||  m_trkAnaDefSvc->useEFTrigger() ) {
    m_trigTrackVec[ stage ].clear();
    m_trigTrackVec[ stage ].insert(
      m_trigTrackVec[ stage ].begin(),
      vec.begin(), vec.end() );
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG( "No Trigger TrackParticle vector" );
  return StatusCode::SUCCESS;
}

/// -----------------------------------
/// --- Fill FULL Vertex containers ---
/// -----------------------------------
/// Truth vertices
StatusCode IDTPM::TrackAnalysisCollections::fillTruthVertexContainer(
  const SG::ReadHandleKey< xAOD::TruthVertexContainer >& truthVertexHandleKey )
{
  m_truthVertexContainer = nullptr;
  m_truthVertexVec[ FULL ].clear();

  /// Skip if ! useTruth || if handle key is empty
  if( ! m_trkAnaDefSvc->useTruth() or
      truthVertexHandleKey.key().empty() ) return StatusCode::SUCCESS;

  ATH_MSG_DEBUG( "Loading collection: " << truthVertexHandleKey.key() );
  SG::ReadHandle< xAOD::TruthVertexContainer > pTruthVertColl( truthVertexHandleKey );
  if( ! pTruthVertColl.isValid() ) {
    ATH_MSG_ERROR( "Non valid truth vertex collection: " << truthVertexHandleKey.key() );
    return StatusCode::FAILURE;
  }

  /// Fill container
  m_truthVertexContainer = pTruthVertColl.ptr();

  /// Grab the entire truth Vertex collection
  m_truthVertexVec[ FULL ].insert(
    m_truthVertexVec[ FULL ].begin(),
    pTruthVertColl->begin(), pTruthVertColl->end() );

  /// TODO - select truth vertices based on PU switch

  return StatusCode::SUCCESS; 
}

/// Offline vertices
StatusCode IDTPM::TrackAnalysisCollections::fillOfflVertexContainer(
  const SG::ReadHandleKey< xAOD::VertexContainer >& handleKey )
{ 
  m_offlVertexContainer = nullptr;
  m_offlVertexVec[ FULL ].clear();

  /// Skip if ! useOffline || if handle key is empty
  if( ! m_trkAnaDefSvc->useOffline() or
      handleKey.key().empty() ) return StatusCode::SUCCESS;

  ATH_MSG_DEBUG( "Loading collection: " << handleKey.key() );
  SG::ReadHandle< xAOD::VertexContainer > pColl( handleKey );
  if( ! pColl.isValid() ) {
    ATH_MSG_ERROR( "Non valid offline vertex collection: " << handleKey.key() );
    return StatusCode::FAILURE;
  }

  /// Fill container
  m_offlVertexContainer = pColl.ptr();

  /// Fill FULL vector
  m_offlVertexVec[ FULL ].insert( 
    m_offlVertexVec[ FULL ].begin(),
    pColl->begin(), pColl->end() );

  return StatusCode::SUCCESS; 
}

/// Trigger vertices
StatusCode IDTPM::TrackAnalysisCollections::fillTrigVertexContainer(
  const SG::ReadHandleKey< xAOD::VertexContainer >& handleKey )
{ 
  m_trigVertexContainer = nullptr;
  m_trigVertexVec[ FULL ].clear();

  /// Skip if ! useTrigger || if handle key is empty
  if( ! m_trkAnaDefSvc->useTrigger() or
      handleKey.key().empty() ) return StatusCode::SUCCESS;

  ATH_MSG_DEBUG( "Loading collection: " << handleKey.key() );
  SG::ReadHandle< xAOD::VertexContainer > pColl( handleKey );
  if( ! pColl.isValid() ) {
    ATH_MSG_ERROR( "Non valid trigger vertex collection: " << handleKey.key() );
    return StatusCode::FAILURE;
  }

  /// Fill container
  m_trigVertexContainer = pColl.ptr();

  /// Fill FULL vector
  m_trigVertexVec[ FULL ].insert( 
    m_trigVertexVec[ FULL ].begin(),
    pColl->begin(), pColl->end() );

  return StatusCode::SUCCESS; 
}

/// --------------------------------
/// --- Fill TEST vertex vectors ---
/// --------------------------------
/// TEST = truth
StatusCode IDTPM::TrackAnalysisCollections::fillTestTruthVertexVec(
  const std::vector< const xAOD::TruthVertex* >& vec,
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->isTestTruth() ) {
    m_truthVertexVec[ stage ].clear();
    m_truthVertexVec[ stage ].insert(
      m_truthVertexVec[ stage ].begin(),
      vec.begin(), vec.end() );
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG( "No TEST Truth vertex vector" );
  return StatusCode::SUCCESS;
}

/// TEST = reco
StatusCode IDTPM::TrackAnalysisCollections::fillTestRecoVertexVec(
  const std::vector< const xAOD::Vertex* >& vec,
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->isTestOffline() ) {
    m_offlVertexVec[ stage ].clear();
    m_offlVertexVec[ stage ].insert(
      m_offlVertexVec[ stage ].begin(),
      vec.begin(), vec.end() );
    return StatusCode::SUCCESS;
 
  } else if( m_trkAnaDefSvc->isTestTrigger() ) {
    m_trigVertexVec[ stage ].clear();
    m_trigVertexVec[ stage ].insert(
      m_trigVertexVec[ stage ].begin(),
      vec.begin(), vec.end() );
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG( "No TEST Reco vertex vector");
  return StatusCode::SUCCESS;
}

/// -------------------------------------
/// --- Fill REFERENCE vertex vectors ---
/// -------------------------------------
/// REFERENCE = truth
StatusCode IDTPM::TrackAnalysisCollections::fillRefTruthVertexVec(
  const std::vector< const xAOD::TruthVertex* >& vec,
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->isReferenceTruth() ) {
    m_truthVertexVec[ stage ].clear();
    m_truthVertexVec[ stage ].insert(
      m_truthVertexVec[ stage ].begin(),
      vec.begin(), vec.end() );
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG( "No REFERENCE Truth vertex vector" );
  return StatusCode::SUCCESS;
}

/// REFERENCE = reco
StatusCode IDTPM::TrackAnalysisCollections::fillRefRecoVertexVec(
  const std::vector< const xAOD::Vertex* >& vec,
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->isReferenceOffline() ) {
    m_offlVertexVec[ stage ].clear();
    m_offlVertexVec[ stage ].insert(
      m_offlVertexVec[ stage ].begin(),
      vec.begin(), vec.end() );
    return StatusCode::SUCCESS;

  } else if( m_trkAnaDefSvc->isReferenceTrigger() ) {
    m_trigVertexVec[ stage ].clear();
    m_trigVertexVec[ stage ].insert(
      m_trigVertexVec[ stage ].begin(),
      vec.begin(), vec.end() );
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG( "No REFERENCE Reco vertex vector" );
  return StatusCode::SUCCESS; 
}

/// ---------------------------
/// --- Fill vertex vectors ---
/// ---------------------------
/// Truth tracks
StatusCode IDTPM::TrackAnalysisCollections::fillTruthVertexVec(
  const std::vector< const xAOD::TruthVertex* >& vec,
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->useTruth() ) {
    m_truthVertexVec[ stage ].clear();
    m_truthVertexVec[ stage ].insert(
      m_truthVertexVec[ stage ].begin(),
      vec.begin(), vec.end() );
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG( "No Truth vertex vector" );
  return StatusCode::SUCCESS;
}

/// Offline tracks
StatusCode IDTPM::TrackAnalysisCollections::fillOfflVertexVec(
  const std::vector< const xAOD::Vertex* >& vec,
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->useOffline() ) {
    m_offlVertexVec[ stage ].clear();
    m_offlVertexVec[ stage ].insert(
      m_offlVertexVec[ stage ].begin(),
      vec.begin(), vec.end() );
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG( "No Offline vertex vector" );
  return StatusCode::SUCCESS;
}

/// Trigger tracks
StatusCode IDTPM::TrackAnalysisCollections::fillTrigVertexVec(
  const std::vector< const xAOD::Vertex* >& vec,
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->useTrigger() || m_trkAnaDefSvc->useEFTrigger() ) {
    m_trigVertexVec[ stage ].clear();
    m_trigVertexVec[ stage ].insert(
      m_trigVertexVec[ stage ].begin(),
      vec.begin(), vec.end() );
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG( "No Trigger vertex vector" );
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
  bool isTrigEmpty  = m_trkAnaDefSvc->useTrigger() && ! m_trkAnaDefSvc->useEFTrigger() && 
                      (stage != IDTPM::TrackAnalysisCollections::FS) ?
                      m_trigTrackVec[ stage ].empty() : false;
  bool isEFTrigEmpty  = m_trkAnaDefSvc->useEFTrigger() ?
                      m_trigTrackVec[ stage ].empty() : false;
  bool isOfflEmpty  = m_trkAnaDefSvc->useOffline() ?
                      m_offlTrackVec[ stage ].empty() : false;
  bool isTruthEmpty = m_trkAnaDefSvc->useTruth() ?
                      m_truthPartVec[ stage ].empty() : false;

  if( isTrigEmpty || isEFTrigEmpty || isOfflEmpty || isTruthEmpty ) return true;

  return false;
}

/// clear collections
void IDTPM::TrackAnalysisCollections::clear(
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( stage == FULL ) {
    /// tracks
    m_truthPartVec[ FULL ].clear();
    m_offlTrackVec[ FULL ].clear();
    m_trigTrackVec[ FULL ].clear();
    /// vertices
    m_truthVertexVec[ FULL ].clear();
    m_offlVertexVec[ FULL ].clear();
    m_trigVertexVec[ FULL ].clear();
  }
  if( stage == FULL || stage == FS ) {
    /// tracks
    m_truthPartVec[ FS ].clear();
    m_offlTrackVec[ FS ].clear();
    m_trigTrackVec[ FS ].clear();
    /// vertices
    m_truthVertexVec[ FS ].clear();
    m_offlVertexVec[ FS ].clear();
    m_trigVertexVec[ FS ].clear();
  }
  if( stage == FULL || stage == FS || stage == InRoI ) {
    /// tracks
    m_truthPartVec[ InRoI ].clear();
    m_offlTrackVec[ InRoI ].clear();
    m_trigTrackVec[ InRoI ].clear();
    /// vertices
    m_truthVertexVec[ InRoI ].clear();
    m_offlVertexVec[ InRoI ].clear();
    m_trigVertexVec[ InRoI ].clear();
  }
}

/// copy inRoI collections from FullScan collections
void IDTPM::TrackAnalysisCollections::copyFS()
{
  ATH_MSG_DEBUG( "Copying tracks in RoI" );

  /// --- Offline ---
  /// tracks
  m_offlTrackVec[ InRoI ].clear(); 
  m_offlTrackVec[ InRoI ].insert( 
    m_offlTrackVec[ InRoI ].begin(),
    m_offlTrackVec[ FS ].begin(),
    m_offlTrackVec[ FS ].end() );
  /// vertices
  m_offlVertexVec[ InRoI ].clear(); 
  m_offlVertexVec[ InRoI ].insert( 
    m_offlVertexVec[ InRoI ].begin(),
    m_offlVertexVec[ FS ].begin(),
    m_offlVertexVec[ FS ].end() );

  /// -- Truth ---
  /// tracks
  m_truthPartVec[ InRoI ].clear(); 
  m_truthPartVec[ InRoI ].insert( 
    m_truthPartVec[ InRoI ].begin(),
    m_truthPartVec[ FS ].begin(),
    m_truthPartVec[ FS ].end() );
  /// vertices
  m_truthVertexVec[ InRoI ].clear(); 
  m_truthVertexVec[ InRoI ].insert( 
    m_truthVertexVec[ InRoI ].begin(),
    m_truthVertexVec[ FS ].begin(),
    m_truthVertexVec[ FS ].end() );

  /// --- EF trigger ---
  if( m_trkAnaDefSvc->useEFTrigger() ) {
    /// tracks
    m_trigTrackVec[ InRoI ].clear(); 
    m_trigTrackVec[ InRoI ].insert( 
      m_trigTrackVec[ InRoI ].begin(),
      m_trigTrackVec[ FS ].begin(),
      m_trigTrackVec[ FS ].end() );
    /// vertices
    m_trigVertexVec[ InRoI ].clear(); 
    m_trigVertexVec[ InRoI ].insert( 
      m_trigVertexVec[ InRoI ].begin(),
      m_trigVertexVec[ FS ].begin(),
      m_trigVertexVec[ FS ].end() );
  }

  /// Debug printout
  ATH_MSG_DEBUG( "Tracks and vertices after in RoI copy: " << printInfo( InRoI ) );
}

/// ---------------------------------
/// --- Get FULL Track containers ---
/// ---------------------------------
/// TEST = Truth
const xAOD::TruthParticleContainer*
IDTPM::TrackAnalysisCollections::testTruthContainer()
{
  if( m_trkAnaDefSvc->isTestTruth() ) {
    return m_truthPartContainer;
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
    return m_truthPartContainer;
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
    return m_truthPartVec[ stage ];
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
    return m_truthPartVec[ stage ];
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

/// ------------------------
/// --- Get track counts ---
/// ------------------------
/// TEST
std::vector< size_t >
IDTPM::TrackAnalysisCollections::testTrackCounts()
{
  std::vector< size_t > counts( NCOUNTERS, 0 );
  counts[ ALL ] = m_trkAnaDefSvc->isTestTruth() ?
      testTruthVec( FULL ).size() :
      testTrackVec( FULL ).size();
  counts[ SELECTED ] = m_trkAnaDefSvc->isTestTruth() ?
      testTruthVec( FS ).size() :
      testTrackVec( FS ).size();
  counts[ INROI ] = m_trkAnaDefSvc->isTestTruth() ?
      testTruthVec( InRoI ).size() :
      testTrackVec( InRoI ).size();
  counts[ MATCHED ] = matches().getNmatches();
  return counts;
}

/// REFERENCE
std::vector< size_t >
IDTPM::TrackAnalysisCollections::refTrackCounts()
{
  std::vector< size_t > counts( NCOUNTERS, 0 );
  counts[ ALL ] = m_trkAnaDefSvc->isReferenceTruth() ?
      refTruthVec( FULL ).size() :
      refTrackVec( FULL ).size();
  counts[ SELECTED ] = m_trkAnaDefSvc->isReferenceTruth() ?
      refTruthVec( FS ).size() :
      refTrackVec( FS ).size();
  counts[ INROI ] = m_trkAnaDefSvc->isReferenceTruth() ?
      refTruthVec( InRoI ).size() :
      refTrackVec( InRoI ).size();
  counts[ MATCHED ] = matches().getNmatches( true );
  return counts;
}

/// ----------------------------------
/// --- Get FULL Vertex containers ---
/// ----------------------------------
/// TEST = Truth
const xAOD::TruthVertexContainer*
IDTPM::TrackAnalysisCollections::testTruthVertexContainer()
{
  if( m_trkAnaDefSvc->isTestTruth() ) {
    return m_truthVertexContainer;
  }

  return nullptr;
}

/// TEST = Reco
const xAOD::VertexContainer*
IDTPM::TrackAnalysisCollections::testRecoVertexContainer()
{
  if( m_trkAnaDefSvc->isTestOffline() ) {
    return m_offlVertexContainer;
  }
    
  if( m_trkAnaDefSvc->isTestTrigger() ) {
    return m_trigVertexContainer;
  }

  return nullptr;
}

/// REFERENCE = Truth
const xAOD::TruthVertexContainer*
IDTPM::TrackAnalysisCollections::refTruthVertexContainer()
{
  if( m_trkAnaDefSvc->isReferenceTruth() ) {
    return m_truthVertexContainer;
  }

  return nullptr;
}

/// REFERENCE = Reco
const xAOD::VertexContainer*
IDTPM::TrackAnalysisCollections::refRecoVertexContainer()
{
  if( m_trkAnaDefSvc->isReferenceOffline() ) {
    return m_offlVertexContainer;
  }
    
  if( m_trkAnaDefSvc->isReferenceTrigger() ) {
    return m_trigVertexContainer;
  }

  return nullptr;
}

/// --------------------------
/// --- Get vertex vectors ---
/// --------------------------
/// TEST = Truth
const std::vector< const xAOD::TruthVertex* >&
IDTPM::TrackAnalysisCollections::testTruthVertexVec(
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->isTestTruth() ) {
    return m_truthVertexVec[ stage ];
  }

  ATH_MSG_DEBUG( "No Test truth vertex vector found" );
  return m_nullTruthVertVec;
}

/// TEST = Reco
const std::vector< const xAOD::Vertex* >&
IDTPM::TrackAnalysisCollections::testRecoVertexVec(
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->isTestOffline() ) {
    return m_offlVertexVec[ stage ];
  }

  if( m_trkAnaDefSvc->isTestTrigger() ) {
    return m_trigVertexVec[ stage ];
  }

  ATH_MSG_DEBUG( "No Test reco vertex vector found" );
  return m_nullRecoVertVec;
}

/// REFERENCE = Truth
const std::vector< const xAOD::TruthVertex* >&
IDTPM::TrackAnalysisCollections::refTruthVertexVec(
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->isReferenceTruth() ) {
    return m_truthVertexVec[ stage ];
  }

  ATH_MSG_DEBUG( "No Reference truth vertex vector found" );
  return m_nullTruthVertVec;
}

/// REFERENCE = Reco
const std::vector< const xAOD::Vertex* >&
IDTPM::TrackAnalysisCollections::refRecoVertexVec(
  IDTPM::TrackAnalysisCollections::Stage stage )
{
  if( m_trkAnaDefSvc->isReferenceOffline() ) {
    return m_offlVertexVec[ stage ];
  }

  if( m_trkAnaDefSvc->isReferenceTrigger() ) {
    return m_trigVertexVec[ stage ];
  }

  ATH_MSG_DEBUG( "No Test reco vertex vector found" );
  return m_nullRecoVertVec;
}

/// -------------------------
/// --- Get vertex counts ---
/// -------------------------
/// TEST
std::vector< size_t >
IDTPM::TrackAnalysisCollections::testVertexCounts()
{
  std::vector< size_t > counts( NCOUNTERS, 0 );
  counts[ ALL ] = m_trkAnaDefSvc->isTestTruth() ?
      testTruthVertexVec( FULL ).size() :
      testRecoVertexVec( FULL ).size();
  counts[ SELECTED ] = m_trkAnaDefSvc->isTestTruth() ?
      testTruthVertexVec( FS ).size() :
      testRecoVertexVec( FS ).size();
  counts[ INROI ] = m_trkAnaDefSvc->isTestTruth() ?
      testTruthVertexVec( InRoI ).size() :
      testRecoVertexVec( InRoI ).size();
  counts[ MATCHED ] = 0; /// TODO: add matched vertex count
  return counts;
}

/// REFERENCE
std::vector< size_t >
IDTPM::TrackAnalysisCollections::refVertexCounts()
{
  std::vector< size_t > counts( NCOUNTERS, 0 );
  counts[ ALL ] = m_trkAnaDefSvc->isReferenceTruth() ?
      refTruthVertexVec( FULL ).size() :
      refRecoVertexVec( FULL ).size();
  counts[ SELECTED ] = m_trkAnaDefSvc->isReferenceTruth() ?
      refTruthVertexVec( FS ).size() :
      refRecoVertexVec( FS ).size();
  counts[ INROI ] = m_trkAnaDefSvc->isReferenceTruth() ?
      refTruthVertexVec( InRoI ).size() :
      refRecoVertexVec( InRoI ).size();
  counts[ MATCHED ] = 0; /// TODO: add matched vertex count
  return counts;
}

/// -----------------------------------------
/// --- Print collection info (for debug) ---
/// -----------------------------------------
/// print tracks
std::string IDTPM::TrackAnalysisCollections::printInfo(
  IDTPM::TrackAnalysisCollections::Stage stage, bool printVertex ) const
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

  if( ! m_offlTrackVec[ stage ].empty() )
    ss << "==========================================" << std::endl;

  it = 0;
  for( const xAOD::TruthParticle* thisTruthParticle : m_truthPartVec[ stage ] ) {
    ss << "Truth particle"
       << " : pt = "  << pT( *thisTruthParticle )
       << " : eta = " << eta( *thisTruthParticle )
       << " : phi = " << phi( *thisTruthParticle )
       << std::endl;
    if( it > 20 ) { ss << "et al...." << std::endl; break; }
    it++;
  }

  if( ! m_truthPartVec[ stage ].empty() )
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

  if( ! m_trigTrackVec[ stage ].empty() )
    ss << "==========================================" << std::endl;

  /// print also vertex info
  if( printVertex ) ss << "Vertices: " << printVertexInfo( stage );

  return ss.str();
}

/// printVertexInfo
std::string IDTPM::TrackAnalysisCollections::printVertexInfo(
  IDTPM::TrackAnalysisCollections::Stage stage ) const
{
  std::stringstream ss;
  ss << "\n==========================================" << std::endl;

  size_t it(0);
  for( const xAOD::Vertex* thisOfflineVertex : m_offlVertexVec[ stage ] ) {
    ss << "Offline vertex"
       << " : z = "  << posZ( *thisOfflineVertex )
       << std::endl;
    if( it > 20 ) { ss << "et al...." << std::endl; break; }
    it++;
  }

  if( ! m_offlVertexVec[ stage ].empty() )
    ss << "==========================================" << std::endl;

  it = 0;
  for( const xAOD::TruthVertex* thisTruthVertex : m_truthVertexVec[ stage ] ) {
    ss << "Truth vertex"
       << " : z = "  << posZ( *thisTruthVertex )
       << std::endl;
    if( it > 20 ) { ss << "et al...." << std::endl; break; }
    it++;
  }

  if( ! m_truthVertexVec[ stage ].empty() )
      ss << "==========================================" << std::endl;

  it = 0;
  for( const xAOD::Vertex* thisTriggerVertex : m_trigVertexVec[ stage ] ) {
    ss << "Trigger vertex"
       << " : z = "  << posZ( *thisTriggerVertex )
       << std::endl;
    if( it > 20 ) { ss << "et al...." << std::endl; break; }
    it++;
  }

  if( ! m_trigVertexVec[ stage ].empty() )
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


/// ----------------------------
/// --- update chainRois map ---
/// ----------------------------
bool IDTPM::TrackAnalysisCollections::updateChainRois(
    const std::string& chainRoi, const std::string& roiStr )
{
  ATH_MSG_DEBUG( "Updating TrackAnalysisCollection with ChainRoiName: " << chainRoi );

  std::pair< mapChainRoi_t::iterator, bool > result =
      m_chainRois.insert( mapChainRoi_t::value_type( chainRoi, roiStr ) );

  if( ! result.second ) {
    ATH_MSG_WARNING( "ChainRoiName has already been cached. No update." );
    return false;
  }

  /// Creating new matching lookup table for newChainRoiName
  m_matches->clear();
  m_matches->chainRoiName( chainRoi );
  /// m_matches is now ready to be updated with new entries for the maps...

  return result.second;
}
