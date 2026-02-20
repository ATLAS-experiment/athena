/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   StableDeltaRMatchingTool.cxx
 * @author Harry Simpson <harry.simpson@cern.ch>
 * @date   30 March 2024
 * @brief  Derived tool to perform Delta R matching of tracks based on the stable marriage algorithm
 */

/// Local include(s)
#include "StableDeltaRMatchingTool.h"
#include "TrackAnalysisCollections.h"
#include "TrackMatchingLookup.h"

namespace IDTPM {

  /// --------------------------------------------
  /// ------ Track -> Track Stable Matching ------
  /// --------------------------------------------

  StatusCode StableDeltaRMatchingTool_trk::match(TrackAnalysisCollections& trkAnaColls,
							 const std::string& chainRoIName,
							 const std::string& roiStr) const {
    bool doMatch = trkAnaColls.updateChainRois(chainRoIName, roiStr);

    if ( not doMatch ) {
      ATH_MSG_WARNING("Matching for " << chainRoIName << " was already done. Skipping");
      return StatusCode::SUCCESS;
    }
    
    ATH_CHECK(match(trkAnaColls.testTrackVec(TrackAnalysisCollections::InRoI),
		    trkAnaColls.refTrackVec(TrackAnalysisCollections::InRoI),
		    trkAnaColls.matches()));

    ATH_MSG_DEBUG(trkAnaColls.printMatchInfo());
    return StatusCode::SUCCESS;
  }

  /// --------------------------------------------
  /// ------ Track -> Truth Stable Matching ------
  /// --------------------------------------------

  StatusCode StableDeltaRMatchingTool_trkTruth::match(TrackAnalysisCollections& trkAnaColls,
							      const std::string& chainRoIName,
							      const std::string& roiStr ) const {
    bool doMatch = trkAnaColls.updateChainRois( chainRoIName, roiStr );

    if( not doMatch ) {
      ATH_MSG_WARNING( "Matching for " << chainRoIName << " was already done. Skipping" );
      return StatusCode::SUCCESS;
    }

    ATH_CHECK( match(trkAnaColls.testTrackVec( TrackAnalysisCollections::InRoI ),
		     trkAnaColls.refTruthVec( TrackAnalysisCollections::InRoI ),
		     trkAnaColls.matches() ) );

    ATH_MSG_DEBUG( trkAnaColls.printMatchInfo() );
    return StatusCode::SUCCESS;
  }

  /// --------------------------------------------
  /// ------ Truth -> Track Stable Matching ------
  /// --------------------------------------------

  StatusCode StableDeltaRMatchingTool_truthTrk::match(TrackAnalysisCollections& trkAnaColls,
							      const std::string& chainRoIName,
							      const std::string& roiStr ) const {
    bool doMatch = trkAnaColls.updateChainRois( chainRoIName, roiStr );

    if( not doMatch ) {
      ATH_MSG_WARNING( "Matching for " << chainRoIName << " was already done. Skipping" );
      return StatusCode::SUCCESS;
    }

    ATH_CHECK( match(trkAnaColls.testTruthVec( TrackAnalysisCollections::InRoI ),
		     trkAnaColls.refTrackVec( TrackAnalysisCollections::InRoI ),
		     trkAnaColls.matches() ) );

    ATH_MSG_DEBUG( trkAnaColls.printMatchInfo() );
    return StatusCode::SUCCESS;
  }

} // end of namespace IDTPM

