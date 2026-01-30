/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_TRACKQUALITYSELECTIONTOOL_H
#define INDETTRACKPERFMON_TRACKQUALITYSELECTIONTOOL_H

/**
 * @file    TrackQualitySelectionTool.h
 * @author  Marco Aparo <marco.aparo@cern.ch>
 * @date    02 October 2023
 * @brief   Tool to handle all required tracks && truth
 *          particle quality selections in this package
 */

/// Athena include(s)
#include "AsgTools/AsgTool.h"

/// Local include(s)
#include "ITrackSelectionTool.h"

/// STD includes
#include <string>


namespace IDTPM {

  class TrackQualitySelectionTool : 
      public virtual IDTPM::ITrackSelectionTool,  
      public asg::AsgTool {

  public:

    ASG_TOOL_CLASS( TrackQualitySelectionTool, ITrackSelectionTool );
   
    /// Constructor 
    TrackQualitySelectionTool( const std::string& name );

    /// Destructor
    virtual ~TrackQualitySelectionTool() = default;

    /// Initialize
    virtual StatusCode initialize() override;

    /// Main Track selection method
    virtual StatusCode selectTracks(
        TrackAnalysisCollections& trkAnaColls ) override;

  private:

    BooleanProperty m_doOfflSelection {
        this, "DoOfflineSelection", false, "Perform offline tracks quality selection" };

    ToolHandle< ITrackSelectionTool > m_offlineSelectionTool {
        this, "OfflineSelectionTool", "IDTPM::InDetTrackPerfMon/ITrackSelectionTool",
        "Tool to perform track quality selection selection" };

    BooleanProperty m_doTruthSelection {
        this, "DoTruthSelection", false, "Perform truth particles quality selection" };

    ToolHandle< ITrackSelectionTool > m_truthSelectionTool {
        this, "TruthSelectionTool", "IDTPM::InDetTrackPerfMon/ITrackSelectionTool",
        "Tool to perform truth track selection" };

    BooleanProperty m_doObjSelection {
        this, "DoObjectSelection", false, "Perform track-object selection" };

    ToolHandle< ITrackSelectionTool > m_objSelectionTool {
        this, "TrackObjectSelectionTool", "IDTPM::InDetTrackPerfMon/ITrackSelectionTool",
        "Tool to perform track-object selection" };

  }; // class InDetGeneralSelectionTool

} // namespace IDTPM



#endif // > ! INDETTRACKPERFMON_TRACKQUALITYSELECTIONTOOL_H
