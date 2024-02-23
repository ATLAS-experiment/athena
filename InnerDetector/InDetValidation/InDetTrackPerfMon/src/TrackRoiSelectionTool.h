/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_TRACKROISELECTIONTOOL_H
#define INDETTRACKPERFMON_TRACKROISELECTIONTOOL_H

/**
 * @file TrackRoiSelectionTool.h
 * header file for class of same name
 * @author marco aparo
 * @date 16 February 2023
**/

/// Athena include(s)
#include "AsgTools/AsgTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

/// Local include(s)
#include "InDetTrackPerfMon/ITrackSelectionTool.h"

/// STD includes
#include <string>
#include <vector>

class TrigRoiDescriptor;

typedef struct {
  float z;
  float r;
  float tantheta;
} exitPoint_t;


namespace IDTPM {

  class TrackRoiSelectionTool :
      public virtual IDTPM::ITrackSelectionTool,
      public asg::AsgTool {

  public:

    ASG_TOOL_CLASS( TrackRoiSelectionTool, ITrackSelectionTool );

    /// Constructor
    TrackRoiSelectionTool( const std::string& name );

    /// Destructor
    virtual ~TrackRoiSelectionTool() = default;

    /// Initialize
    virtual StatusCode initialize() override;

    /// Main Track selection method
    virtual StatusCode selectTracksInRoI(
        IDTPM::TrackAnalysisCollections& trkAnaColls,
        const ElementLink< TrigRoiDescriptorCollection >& roiLink ) override;

    /// Dummy method - Disabled
    virtual StatusCode selectTracks(
        IDTPM::TrackAnalysisCollections& ) override {
      ATH_MSG_WARNING( "selectTracks method is disabled" );
      return StatusCode::SUCCESS;
    }
  
    /// geometric RoI filters - for non-trigger tracks (e.g. offline, truth, etc.)
    template< class T >
    bool accept( const T& t, const TrigRoiDescriptor* r ) const;

    /// track getter function (for offline tracks or truth particles)
    template< class T >
    std::vector< const T* > getTracks(
        std::vector< const T* > tvec, const TrigRoiDescriptor* r ) const;

    /// TrigDecTool- and EventView-based getter function for trigger tracks
    std::vector< const xAOD::TrackParticle* > getTrigTracks( 
        SG::ReadHandleKey< xAOD::TrackParticleContainer >& handleKey, 
        const ElementLink< TrigRoiDescriptorCollection >& roiLink ) const;

  private:

    /// Geometric utility methods for track-RoI association
    exitPoint_t getExitPoint( float tz0, float teta ) const;

    float getOuterPhi( float pt, float phi, float r=1000. ) const;

    /// Trigger TrackParticleContainer's name
    SG::ReadHandleKey< xAOD::TrackParticleContainer > m_triggerTrkParticleName{
        this, "TriggerTrkParticleContainerName", "HLT_IDTrack_Electron_IDTrig", "Name of container of trigger tracks" };

    /// TrigDecTool
    PublicToolHandle< Trig::TrigDecisionTool > m_trigDecTool{
        this, "TrigDecisionTool", "Trig::TrigDecisionTool/TrigDecisionTool", "" };

  }; // class TrackRoiSelectionTool

} // namespace IDTPM


#endif // > !INDETTRACKPERFMON_TRACKROISELECTIONTOOL_H
