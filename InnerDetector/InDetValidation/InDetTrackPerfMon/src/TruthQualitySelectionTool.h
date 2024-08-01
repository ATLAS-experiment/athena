/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef INDETTRACKPERFMON_TRUTHQUALITYSELECTIONTOOL_H
#define INDETTRACKPERFMON_TRUTHQUALITYSELECTIONTOOL_H

// Package includes
#include "InDetTrackPerfMon/ITrackSelectionTool.h"
#include "TrkTruthTrackInterfaces/IAthSelectionTool.h"
#include "TrackAnalysisCollections.h"

// Framework includes
#include "AsgTools/AsgTool.h"

// STL includes
#include <string>

/**
 * @class TruthQualitySelectionTool
 * @brief
 **/
namespace IDTPM{
class TruthQualitySelectionTool :  
      public virtual IDTPM::ITrackSelectionTool,  
      public asg::AsgTool {
public:
  ASG_TOOL_CLASS( TruthQualitySelectionTool, ITrackSelectionTool );
   
  TruthQualitySelectionTool( const std::string& name );

  virtual StatusCode initialize() override;

  virtual StatusCode selectTracks(
      TrackAnalysisCollections& trkAnaColls ) override;

  /// Dummy method - unused
  virtual StatusCode selectTracksInRoI(
      TrackAnalysisCollections& ,
      const ElementLink< TrigRoiDescriptorCollection >& ) override {
    ATH_MSG_ERROR( "selectTracksInRoI method is disabled" );
    return StatusCode::SUCCESS;
  }



private:
  ToolHandle<IAthSelectionTool> m_truthTool{this, "truthTool", {}, "Truth selection tool to use, has to be setup" };
};
}
#endif // INDETTRACKPERFMON_TRUTHQUALITYSELECTIONTOOL_H
