/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_DELTARMATCHINGTOOL_H
#define INDETTRACKPERFMON_DELTARMATCHINGTOOL_H

/**
 * @file   DeltaRMatchingTool.h
 * @author Marco Aparo <marco.aparo@cern.ch>, Thomas Strebler <thomas.strebler@cern.ch>
 * @date   30 March 2024
 * @brief  Tool to perform matching of tracks and/or truth particles
 *         based on their ditance in DeltaR or in their pT reslution
 */

/// Athena include(s)
#include "AsgTools/AsgTool.h"

/// Local include(s)
#include "ITrackMatchingTool.h"


namespace IDTPM {

  /// ------------------------------------------
  /// --------- Base (templated) class ---------
  /// ------------------------------------------
  template< typename T, typename R=T > 
  class DeltaRMatchingToolBase :
      public asg::AsgTool {

  public:

    /// Constructor
    DeltaRMatchingToolBase( const std::string& name ) :
        asg::AsgTool( name ) { };

    /// Initialize
    virtual StatusCode initialize() override
    {
      ATH_MSG_DEBUG( "Initializing " << name() );
      ATH_CHECK( asg::AsgTool::initialize() );

      if( m_dRmax<0 and m_pTResMax<0 ) {
        ATH_MSG_ERROR( "No DeltaRMax or pTresMax criteria requested" ); 
        return StatusCode::FAILURE;
      } 

      return StatusCode::SUCCESS;
    }

    /// matchVectors
    StatusCode matchVectors(
        const std::vector< const T* >& vTest,
        const std::vector< const R* >& vRef,
        ITrackMatchingLookup& matches ) const;

    /// Get the reference track matched to the given test
    const R* getMatchedRef(
        const T& t, const std::vector< const R* >& vRef ) const;

  protected:

    FloatProperty m_dRmax { this, "dRmax", 0.05, "Maximum DeltaR cone size for DeltaR-matching (disabled if <0)" };
    FloatProperty m_pTResMax { this, "pTResMax", -9.9, "Maximum relative pT distance allowed for matching (disabled if <0)" };

  }; // class DeltaRMatchingToolBase


  /// --------------------------------------------
  /// ------ Track -> Track DeltaR matching ------
  /// --------------------------------------------
  class DeltaRMatchingTool_trk :
      public DeltaRMatchingToolBase< xAOD::TrackParticle >,
      public virtual ITrackMatchingTool {

  public:

    ASG_TOOL_CLASS( DeltaRMatchingTool_trk, ITrackMatchingTool );

    /// Constructor
    DeltaRMatchingTool_trk( const std::string& name ) :
        DeltaRMatchingToolBase< xAOD::TrackParticle >( name ) { }

    /// General matching method, via TrackAnalysisCollections
    virtual StatusCode match( 
        TrackAnalysisCollections& trkAnaColls,
        const std::string& chainRoIName,
        const std::string& roiStr ) const override;

    /// Specific matching methods, via test/reference vectors
    /// track -> track matching
    virtual StatusCode match(
        const std::vector< const xAOD::TrackParticle* >& vTest,
        const std::vector< const xAOD::TrackParticle* >& vRef,
        ITrackMatchingLookup& matches ) const override
    {
      ATH_MSG_DEBUG( "Doing Track->Track DeltaR matching" );
      ATH_CHECK( matchVectors( vTest, vRef, matches ) );
      return StatusCode::SUCCESS;
    }

    /// track -> truth matching (disabled)
    virtual StatusCode match(
        const std::vector< const xAOD::TrackParticle* >&,
        const std::vector< const xAOD::TruthParticle* >&,
        ITrackMatchingLookup& ) const override
    {
      ATH_MSG_WARNING( "Track->Truth matching disabled" );
      return StatusCode::SUCCESS;
    }

    /// truth -> track matching (disabled)
    virtual StatusCode match(
        const std::vector< const xAOD::TruthParticle* >&,
        const std::vector< const xAOD::TrackParticle* >&,
        ITrackMatchingLookup& ) const override
    {
      ATH_MSG_WARNING( "Truth->Track matching disabled" );
      return StatusCode::SUCCESS;
    }

  }; // class DeltaRMatchingTool_trk


  /// --------------------------------------------
  /// ------ Track -> Truth DeltaR matching ------
  /// --------------------------------------------
  class DeltaRMatchingTool_trkTruth :
      public DeltaRMatchingToolBase< xAOD::TrackParticle, xAOD::TruthParticle >,
      public virtual ITrackMatchingTool {

  public:

    ASG_TOOL_CLASS( DeltaRMatchingTool_trkTruth, ITrackMatchingTool );

    /// Constructor
    DeltaRMatchingTool_trkTruth( const std::string& name ) :
        DeltaRMatchingToolBase< xAOD::TrackParticle, xAOD::TruthParticle >( name ) { }

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
      ATH_MSG_WARNING( "Track->Track matching disabled" );
      return StatusCode::SUCCESS;
    }

    /// track -> truth matching
    virtual StatusCode match(
        const std::vector< const xAOD::TrackParticle* >& vTest,
        const std::vector< const xAOD::TruthParticle* >& vRef,
        ITrackMatchingLookup& matches ) const override
    {
      ATH_MSG_DEBUG( "Doing Track->Truth DeltaR matching" );
      ATH_CHECK( matchVectors( vTest, vRef, matches ) );
      return StatusCode::SUCCESS;
    }

    /// truth -> track matching (disabled)
    virtual StatusCode match(
        const std::vector< const xAOD::TruthParticle* >&,
        const std::vector< const xAOD::TrackParticle* >&,
        ITrackMatchingLookup& ) const override
    {
      ATH_MSG_WARNING( "Truth->Track matching disabled" );
      return StatusCode::SUCCESS;
    }

  }; // class DeltaRMatchingTool_trkTruth


  /// --------------------------------------------
  /// ------ Truth -> Track DeltaR matching ------
  /// --------------------------------------------
  class DeltaRMatchingTool_truthTrk :
      public DeltaRMatchingToolBase< xAOD::TruthParticle, xAOD::TrackParticle >,
      public virtual ITrackMatchingTool {

  public:

    ASG_TOOL_CLASS( DeltaRMatchingTool_truthTrk, ITrackMatchingTool );

    /// Constructor
    DeltaRMatchingTool_truthTrk( const std::string& name ) :
        DeltaRMatchingToolBase< xAOD::TruthParticle, xAOD::TrackParticle >( name ) { }

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
      ATH_MSG_WARNING( "Track->Track matching disabled" );
      return StatusCode::SUCCESS;
    }

    /// track -> truth matching (disabled)
    virtual StatusCode match(
        const std::vector< const xAOD::TrackParticle* >&,
        const std::vector< const xAOD::TruthParticle* >&,
        ITrackMatchingLookup& ) const override
    {
      ATH_MSG_WARNING( "Track->Truth matching disabled" );
      return StatusCode::SUCCESS;
    }

    /// truth -> track matching
    virtual StatusCode match(
        const std::vector< const xAOD::TruthParticle* >& vTest,
        const std::vector< const xAOD::TrackParticle* >& vRef,
        ITrackMatchingLookup& matches ) const override
    {
      ATH_MSG_DEBUG( "Doing Truth->Track DeltaR matching" );
      ATH_CHECK( matchVectors( vTest, vRef, matches ) );
      return StatusCode::SUCCESS;
    }

  }; // class DeltaRMatchingTool_truthTrk

} // namespace IDTPM

#include "DeltaRMatchingTool.icc"

#endif // > !INDETTRACKPERFMON_DELTARMATCHINGTOOL_H
