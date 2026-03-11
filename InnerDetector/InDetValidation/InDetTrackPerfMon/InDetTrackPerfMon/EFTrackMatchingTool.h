/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_EFTrackMatchingTool_H
#define INDETTRACKPERFMON_EFTrackMatchingTool_H

/**
 * @file   EFTrackMatchingTool.h
 * @author Marco Aparo <marco.aparo@cern.ch>, Federica Piazza <federica.piazza@cern.ch>
 * @date   18 July 2024
 * @brief  Tool to perform matching of EF tracks && Offline tracks matched to the same truth via truthParticleLink decorations
 */

/// Athena include(s).
#include "AsgTools/AsgTool.h"

/// Local include(s)
#include "ITrackMatchingTool.h"

namespace IDTPM {

  class EFTrackMatchingTool : 
      public virtual ITrackMatchingTool,  
      public asg::AsgTool {

  public:

    ASG_TOOL_CLASS( EFTrackMatchingTool, ITrackMatchingTool );

    /// Constructor 
    EFTrackMatchingTool( const std::string& name );

    /// Initialize
    virtual StatusCode initialize() override;

    /// General matching method, via TrackAnalysisCollections
    virtual StatusCode match( 
        TrackAnalysisCollections& trkAnaColls,
        const std::string& chainRoIName,
        const std::string& roiStr ) const override;

    /// Specific matching methods, via test/reference vectors

    /// track -> track matching 
    virtual StatusCode match(
        const std::vector< const xAOD::TrackParticle* >&,
        const std::vector< const xAOD::TrackParticle* >&,
        ITrackMatchingLookup& ) const override;

    /// track -> truth matching (disabled)
    virtual StatusCode match(
        const std::vector< const xAOD::TrackParticle* >&,
        const std::vector< const xAOD::TruthParticle* >&,
        ITrackMatchingLookup& ) const override
    {
      ATH_MSG_DEBUG( "track -> truth matching disabled" );
      return StatusCode::SUCCESS;
    }

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

  }; // class EFTrackMatchingTool

} // namespace IDTPM

#endif // > !INDETTRACKPERFMON_EFTrackMatchingTool_H
