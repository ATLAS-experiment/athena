/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file TrackTruthMatchingTool.cxx
 * @author Marco Aparo, Thomas Strebler
 **/

/// local includes
#include "TrackTruthMatchingTool.h"
#include "TrackAnalysisCollections.h"
#include "TrackMatchingLookup.h"
#include "OfflineObjectDecorHelper.h"
#include "TrackParmetersHelper.h"

/// STD include(s)
#include <algorithm> // for std::find

///---------------------------
///------- Constructor -------
///---------------------------
IDTPM::TrackTruthMatchingTool::TrackTruthMatchingTool(
    const std::string& name ) :
        asg::AsgTool( name ) { }


///--------------------------
///------- Initialize -------
///--------------------------
StatusCode IDTPM::TrackTruthMatchingTool::initialize()
{
  ATH_CHECK( asg::AsgTool::initialize() );

  ATH_MSG_DEBUG( "Initializing " << name() << "..." );

  return StatusCode::SUCCESS;
}


///---------------------------
///----- match (general) -----
///---------------------------
StatusCode IDTPM::TrackTruthMatchingTool::match(
    TrackAnalysisCollections& trkAnaColls,
    const std::string& chainRoIName,
    const std::string& roiStr ) const
{
  /// Inserting new chainRoIName
  bool doMatch = trkAnaColls.updateChainRois( chainRoIName, roiStr );

  /// checking if matching for chainRoIName has already been processed
  if( not doMatch ) {
    ATH_MSG_WARNING( "Matching for " << chainRoIName <<
                     " was already done. Skipping" );
    return StatusCode::SUCCESS;
  }

  /// New test-reference matching
  ATH_CHECK( match(
      trkAnaColls.testTrackVec( TrackAnalysisCollections::InRoI ),
      trkAnaColls.refTruthVec( TrackAnalysisCollections::InRoI ),
      trkAnaColls.matches() ) );

  ATH_MSG_DEBUG( trkAnaColls.printMatchInfo() );

  return StatusCode::SUCCESS;
}


///----------------------------
///----- match (specific) -----
///----------------------------
StatusCode IDTPM::TrackTruthMatchingTool::match(
    const std::vector< const xAOD::TrackParticle* >& vTest,
    const std::vector< const xAOD::TruthParticle* >& vRef,
    ITrackMatchingLookup& matches ) const
{
  ATH_MSG_DEBUG( "Doing Track->Truth matching via truthParticleLink" );

  for( const xAOD::TrackParticle* track_particle : vTest ) {

    /// Compute track->truth link
    const xAOD::TruthParticle* truth_particle = getLinkedTruth(
                        *track_particle, m_truthProbCut.value() );

    /// Skip if no truth particle is found
    if( not truth_particle ) continue;

    ATH_MSG_DEBUG( "Found matched truth particle with pT = " <<
                   pT( *truth_particle ) << " and prob = " <<
                   getTruthMatchProb( *track_particle ) );

    /// Check if linked truth particle is in the
    /// selected (quality and in-RoI selection) reference truths
    if( std::find( vRef.begin(), vRef.end(),
                   truth_particle ) == vRef.end() ) {
      ATH_MSG_DEBUG( "Truth particle is not in selected reference. Skipping." );
      continue;
    }

    /// Updating lookup table with new match
    ATH_CHECK( matches.update( *track_particle, *truth_particle ) );
  } // loop over vTest

  return StatusCode::SUCCESS;
}
