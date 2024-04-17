/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file DeltaRMatchingTool.cxx
 * @author Marco Aparo, Thomas Strebler
 **/

/// local includes
#include "DeltaRMatchingTool.h"
#include "TrackAnalysisCollections.h"
#include "TrackMatchingLookup.h"


///------------------------------------
///--- class DeltaRMatchingTool_trk ---
///------------------------------------
/// track -> track matching
StatusCode IDTPM::DeltaRMatchingTool_trk::match(
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


///-----------------------------------------
///--- class DeltaRMatchingTool_trkTruth ---
///-----------------------------------------
/// track -> truth matching
StatusCode IDTPM::DeltaRMatchingTool_trkTruth::match(
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


///-----------------------------------------
///--- class DeltaRMatchingTool_truthTrk ---
///-----------------------------------------
/// truth -> track matching
StatusCode IDTPM::DeltaRMatchingTool_truthTrk::match(
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
