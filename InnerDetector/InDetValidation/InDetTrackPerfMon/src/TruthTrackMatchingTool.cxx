/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file TruthTrackMatchingTool.cxx
 * @author Marco Aparo
 **/

/// local includes
#include "TruthTrackMatchingTool.h"
#include "TrackAnalysisCollections.h"
#include "TrackMatchingLookup.h"
#include "OfflineObjectDecorHelper.h"
#include "TrackParmetersHelper.h"

///---------------------------
///------- Constructor -------
///---------------------------
IDTPM::TruthTrackMatchingTool::TruthTrackMatchingTool(
    const std::string& name ) :
        asg::AsgTool( name ) { }


///--------------------------
///------- Initialize -------
///--------------------------
StatusCode IDTPM::TruthTrackMatchingTool::initialize()
{
  ATH_CHECK( asg::AsgTool::initialize() );

  ATH_MSG_DEBUG( "Initializing " << name() << "..." );

  return StatusCode::SUCCESS;
}


///---------------------------
///----- match (general) -----
///---------------------------
StatusCode IDTPM::TruthTrackMatchingTool::match(
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
      trkAnaColls.testTruthVec( TrackAnalysisCollections::InRoI ),
      trkAnaColls.refTrackVec( TrackAnalysisCollections::InRoI ),
      trkAnaColls.matches() ) );

  ATH_MSG_DEBUG( trkAnaColls.printMatchInfo() );

  return StatusCode::SUCCESS;
}


///----------------------------
///----- match (specific) -----
///----------------------------
StatusCode IDTPM::TruthTrackMatchingTool::match(
    const std::vector< const xAOD::TruthParticle* >& vTest,
    const std::vector< const xAOD::TrackParticle* >& vRef,
    ITrackMatchingLookup& matches ) const
{
  ATH_MSG_DEBUG( "Doing Truth->Track matching via truthParticleLink" );

  for( const xAOD::TruthParticle* truth_particle : vTest ) {

    const xAOD::TrackParticle* matched_track_particle = nullptr;
    float prob = -99.;

    for( const xAOD::TrackParticle* track_particle : vRef ) {

      /// Compute track->truth link
      const xAOD::TruthParticle* truth_particle_tmp =
          getLinkedTruth( *track_particle, m_truthProbCut.value() );

      /// Skip if no truth particle is found or
      /// if linked truth particle does not
      /// correspond to the original test one
      if( ( not truth_particle_tmp ) or
          ( truth_particle_tmp != truth_particle ) ) continue;

      float prob_tmp = getTruthMatchProb( *track_particle );

      /// update matched_track_particle
      if( prob_tmp > prob ) {
        matched_track_particle = track_particle;
        prob = prob_tmp;
        /// it's actually very rare that a track particle
        /// is linked to more then onetruth particles, but
        /// this check also accounts for such occurrence
      }

    } // loop over vRef

    /// skip if no matched_track_particle for this truth particle is found
    if( not matched_track_particle ) continue;

    ATH_MSG_DEBUG( "Found matched track particle with pT = " <<
                   pT( *matched_track_particle ) <<
                   " and prob = " << prob );

    /// Updating lookup table with new match
    ATH_CHECK( matches.update( *truth_particle, *matched_track_particle ) );

  } // loop over vTest

  return StatusCode::SUCCESS;
}
