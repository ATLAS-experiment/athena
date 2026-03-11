/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_TRACKTRUTHMATCHINGTOOL_H
#define INDETTRACKPERFMON_TRACKTRUTHMATCHINGTOOL_H

/**
 * @file   TrackTruthMatchingTool.h
 * @author Marco Aparo <marco.aparo@cern.ch>, Thomas Strebler <thomas.strebler@cern.ch>
 * @date   26 April 2023
 * @brief  Tool to perform matching of tracks && truth particles via truthParticleLink decorations
 */

/// Athena include(s).
#include "AsgTools/AsgTool.h"

/// Local include(s)
#include "ITrackMatchingTool.h"

namespace IDTPM {

  class TrackTruthMatchingTool : 
      public virtual ITrackMatchingTool,  
      public asg::AsgTool {

  public:

    ASG_TOOL_CLASS( TrackTruthMatchingTool, ITrackMatchingTool );

    /// Constructor 
    TrackTruthMatchingTool( const std::string& name );

    /// Initialize
    virtual StatusCode initialize() override;

    /// General matching method, via TrackAnalysisCollections
    virtual StatusCode match( 
        TrackAnalysisCollections& trkAnaColls,
        const std::string& chainRoIName,
        const std::string& roiStr ) const override;

    /// Specific matching methods, via test/reference vectors

    /// track -> track matching (disabled)
    virtual StatusCode match(
        const std::vector< const xAOD::TrackParticle* >&,
        const std::vector< const xAOD::TrackParticle* >&,
        ITrackMatchingLookup& ) const override
    {
      ATH_MSG_DEBUG( "track -> track matching disabled" );
      return StatusCode::SUCCESS;
    }

    /// track -> truth matching
    virtual StatusCode match(
        const std::vector< const xAOD::TrackParticle* >& vTest,
        const std::vector< const xAOD::TruthParticle* >& vRef,
        ITrackMatchingLookup& matches ) const override;

    /// truth -> track matching (disabled)
    virtual StatusCode match(
        const std::vector< const xAOD::TruthParticle* >&,
        const std::vector< const xAOD::TrackParticle* >&,
        ITrackMatchingLookup& ) const override
    {
      ATH_MSG_DEBUG( "truth -> track matching disabled" );
      return StatusCode::SUCCESS;
    }

  private:

    FloatProperty m_truthProbCut { 
        this, "MatchingTruthProb", 0.5, "Minimal truthProbability for valid matching" };

  }; // class TrackTruthMatchingTool

} // namespace IDTPM

#endif // > !INDETTRACKPERFMON_TRACKTRUTHMATCHINGTOOL_H
