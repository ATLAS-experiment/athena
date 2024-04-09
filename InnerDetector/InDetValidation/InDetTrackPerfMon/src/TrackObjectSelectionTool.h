/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_TRACKOBJECTSELECTIONTOOL_H
#define INDETTRACKPERFMON_TRACKOBJECTSELECTIONTOOL_H

/**
 * @file    TrackObjectSelectionTool.h
 * @author  Marco Aparo <marco.aparo@cern.ch>
 * @date    02 October 2023
 * @brief   Tool to select offline ID tracks which asociated
 *          with fully-reconstructed object with specific
 *          quality criteria
 */

/// Athena include(s)
#include "AsgTools/AsgTool.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"

/// Local include(s)
#include "InDetTrackPerfMon/ITrackSelectionTool.h"

/// EDM includes
#include "xAODTracking/TrackParticle.h"


namespace IDTPM {

  class TrackObjectSelectionTool :
      public virtual IDTPM::ITrackSelectionTool,
      public asg::AsgTool {

  public:

    ASG_TOOL_CLASS( TrackObjectSelectionTool, ITrackSelectionTool );
   
    /// Constructor 
    TrackObjectSelectionTool( const std::string& name );

    /// Destructor
    virtual ~TrackObjectSelectionTool() = default;

    /// Initialize
    virtual StatusCode initialize() override;

    /// Main Track selection method
    virtual StatusCode selectTracks(
        TrackAnalysisCollections& trkAnaColls ) override;

    /// Dummy method - unused
    virtual StatusCode selectTracksInRoI(
        TrackAnalysisCollections&,
        const ElementLink< TrigRoiDescriptorCollection >& ) override {
      ATH_MSG_WARNING( "selectTracksInRoI method is disabled" );
      return StatusCode::SUCCESS;
    }

    bool accept( const xAOD::TrackParticle& offTrack ) const;

  private:

    StringProperty m_objectType {
        this, "ObjectType", "Electron", "Offline object type requested" };

    StringProperty m_objectQuality {
        this, "ObjectQuality", "", "Quality-based object selection" };

    StringProperty m_tauType {
        this, "TauType", "RNN", "Request RNN or BDT taus" };

    IntegerProperty m_tauNprongs {
        this, "TauNprongs", 1, "Request 1- or 3- prong taus" };

    FloatProperty m_truthProbCut {
        this, "MatchingTruthProb", 0.5, "Minimal truthProbability for valid matching" };

  }; // class TrackObjectSelectionTool

} // namespace IDTPM



#endif // > ! INDETTRACKPERFMON_INDETTRACKOBJECTSELECTIONTOOL_H
