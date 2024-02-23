/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_TRACKQUALITYSELECTIONTOOL_H
#define INDETTRACKPERFMON_TRACKQUALITYSELECTIONTOOL_H

/**
 * @file    TrackQualitySelectionTool.h
 * @author  Marco Aparo <marco.aparo@cern.ch>
 * @date    02 October 2023
 * @brief   Tool to handle all required tracks and truth
 *          particle quality selections in this package
 */

/// Athena include(s)
#include "AsgTools/AsgTool.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"

/// Local include(s)
#include "InDetTrackPerfMon/ITrackSelectionTool.h"

/// STD includes
#include <string>
#include <vector>


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
        IDTPM::TrackAnalysisCollections& trkAnaColls ) override;

    /// Dummy method - unused
    virtual StatusCode selectTracksInRoI(
        IDTPM::TrackAnalysisCollections& ,
        const ElementLink< TrigRoiDescriptorCollection >& ) override {
      ATH_MSG_WARNING( "selectTracksInRoI method is disabled" );
      return StatusCode::SUCCESS;
    }

  private:

    BooleanProperty m_doObjSelection {
        this, "DoObjectSelection", false, "Perform track-object selection" };

    ToolHandle< IDTPM::ITrackSelectionTool > m_objSelectionTool {
        this, "TrackObjectSelectionTool", "IDTPM::InDetTrackPerfMon/ITrackSelectionTool", 
        "Tool to perform track-object selection" };

  }; // class InDetGeneralSelectionTool

} // namespace IDTPM



#endif // > ! INDETTRACKPERFMON_TRACKQUALITYSELECTIONTOOL_H
