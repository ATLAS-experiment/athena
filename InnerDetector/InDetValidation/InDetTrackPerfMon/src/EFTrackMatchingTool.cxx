/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file EFTrackMatchingTool.cxx
 * @author Marco Aparo, Federica Piazza
 **/

/// local includes
#include "EFTrackMatchingTool.h"
#include "TrackAnalysisCollections.h"
#include "TrackMatchingLookup.h"
#include "OfflineObjectDecorHelper.h"
#include "TrackParametersHelper.h"

/// STD include(s)
#include <algorithm> // for std::find

///---------------------------
///------- Constructor -------
///---------------------------
IDTPM::EFTrackMatchingTool::EFTrackMatchingTool(
    const std::string& name ) :
        asg::AsgTool( name ) { }


///--------------------------
///------- Initialize -------
///--------------------------
StatusCode IDTPM::EFTrackMatchingTool::initialize()
{
  ATH_CHECK( asg::AsgTool::initialize() );

  ATH_MSG_DEBUG( "Initializing " << name() << "..." );

  return StatusCode::SUCCESS;
}


///---------------------------
///----- match (general) -----
///---------------------------
StatusCode IDTPM::EFTrackMatchingTool::match(
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
      trkAnaColls.refTrackVec( TrackAnalysisCollections::InRoI ),
      trkAnaColls.matches() ) );

  ATH_MSG_DEBUG( trkAnaColls.printMatchInfo() );
  

  return StatusCode::SUCCESS;
}

///----------------------------
///----- match (specific) -----
///----------------------------
StatusCode IDTPM::EFTrackMatchingTool::match(
    const std::vector< const xAOD::TrackParticle* >& vTest,
    const std::vector< const xAOD::TrackParticle* >& vRef,
    ITrackMatchingLookup& matches ) const
{
  ATH_MSG_DEBUG( "Doing EFtrack matching through truthParticleLink" );

  for( const xAOD::TrackParticle* track_particle_test : vTest ) {

    bool firstMatch = true;
    const xAOD::TruthParticle* truth_particle_test = nullptr;

    /// Compute test track->truth link
    truth_particle_test = getLinkedTruth(
                        *track_particle_test, m_truthProbCut.value() );

    /// Skip if no truth particle is found
    if( not truth_particle_test ) {
      ATH_MSG_WARNING( "Test track not linked to truth. Skipping." );
      continue;
    }

    float prob_test = getTruthMatchProb( *track_particle_test );

    ATH_MSG_DEBUG( "Test track with pT = " << pT( *track_particle_test ) <<
                   " is linked to truth with pT = " << pT( *truth_particle_test ) << " and prob = " << prob_test );

    /// Find best offline track matched to same truth
    const xAOD::TrackParticle* track_particle_ref = nullptr;
    float prob_ref = -99.;

    for( const xAOD::TrackParticle* track_particle_tmp : vRef ) {

      /// Compute ref track->truth link
      const xAOD::TruthParticle* truth_particle_tmp =
          getLinkedTruth( *track_particle_tmp, m_truthProbCut.value() );

      /// Skip if no truth particle is found or
      /// if linked truth particle does not
      /// correspond to the original test one
      if( not truth_particle_tmp ) {
        ATH_MSG_WARNING( "Reference track not linked to truth. Skipping." );
        continue;
      }

      float prob_tmp = getTruthMatchProb( *track_particle_tmp );

      ATH_MSG_DEBUG( "Reference track with pT = " << pT( *track_particle_tmp ) <<
                     " is linked to truth with pT = " << pT( *truth_particle_tmp ) << " and prob = " << prob_tmp );

      if( truth_particle_tmp != truth_particle_test ) continue;

      ATH_MSG_DEBUG( "Test track with pT = " << pT( *track_particle_test ) << 
                     " matches with reference track with pT = " << pT( *track_particle_tmp ) );

      /// update track_particle_ref, selecting
      /// the best match based on the matching prob
      if( prob_tmp > prob_ref ) {
        if( not firstMatch ) ATH_MSG_DEBUG( "Found better match. Updating." );
        firstMatch = false;
        track_particle_ref = track_particle_tmp;
        prob_ref = prob_tmp;
      }

    } // loop over vRef

    if( not track_particle_ref ) {
      ATH_MSG_DEBUG( "No match found for test track with pT = " << pT( *track_particle_test ) );
    } else {
      ATH_MSG_DEBUG( "Found best match: test track with pT = " << pT( *track_particle_test ) << 
                     " matches with reference track with pT = " << pT( *track_particle_ref ) );
  
      /// Updating lookup table with new match
      float dist = 1 - prob_test * prob_ref;
      ATH_CHECK( matches.update( *track_particle_test, *track_particle_ref, dist ) );
    }

  } // loop over vTest



  return StatusCode::SUCCESS;
}
