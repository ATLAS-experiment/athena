/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_TRUTHTRACKMATCHINGTOOL_H
#define INDETTRACKPERFMON_TRUTHTRACKMATCHINGTOOL_H

/**
 * @file   TruthTrackMatchingTool.h
 * @author Marco Aparo <marco.aparo@cern.ch>
 * @date   04 April 2024
 * @brief  Tool to perform matching of truth particles
 *         and tracks via their truthParticleLink decorations.
 *         The track with the highest truthMatchProbability is chosen
 */

/// Athena include(s).
#include "AsgTools/AsgTool.h"

/// Local include(s)
#include "ITrackMatchingTool.h"

namespace IDTPM {

  class TruthTrackMatchingTool :
      public virtual ITrackMatchingTool,
      public asg::AsgTool {

  public:

    ASG_TOOL_CLASS( TruthTrackMatchingTool, ITrackMatchingTool );

    /// Constructor 
    TruthTrackMatchingTool( const std::string& name );

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

    /// track -> truth matching (disabled)
    virtual StatusCode match(
        const std::vector< const xAOD::TrackParticle* >&,
        const std::vector< const xAOD::TruthParticle* >&,
        ITrackMatchingLookup& ) const override
    {
      ATH_MSG_DEBUG( "track -> truth matching disabled" );
      return StatusCode::SUCCESS;
    }

    /// truth -> track matching
    virtual StatusCode match(
        const std::vector< const xAOD::TruthParticle* >& vTest,
        const std::vector< const xAOD::TrackParticle* >& vRef,
        ITrackMatchingLookup& matches ) const override;

  private:

    FloatProperty m_truthProbCut { 
        this, "MatchingTruthProb", 0.5, "Minimal truthProbability for valid matching" };

  }; // class TruthTrackMatchingTool

} // namespace IDTPM

#endif // > !INDETTRACKPERFMON_TRUTHTRACKMATCHINGTOOL_H
