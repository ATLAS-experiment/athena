/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef INDETTRACKPERFMON_IDTPM_OFFLINETRACKQUALITYSELECTIONTOOL_H
#define INDETTRACKPERFMON_IDTPM_OFFLINETRACKQUALITYSELECTIONTOOL_H

// Package includes
#include "InDetTrackPerfMon/ITrackSelectionTool.h"

// Framework includes
#include "AsgTools/AsgTool.h"
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"
#include "AthLinks/ElementLink.h"
// STL includes
#include <string>

namespace IDTPM {

/**
 * @class OfflineTrackQualitySelectionTool
 * @brief Uses InDetTrackSelection tool and working points defined there for tracks quality selection
 * In future it is possible that the selection cuts & logic will be moved here
 **/
class OfflineTrackQualitySelectionTool : public virtual IDTPM::ITrackSelectionTool, public asg::AsgTool {
public:
  ASG_TOOL_CLASS( OfflineTrackQualitySelectionTool, ITrackSelectionTool );
  OfflineTrackQualitySelectionTool(const std::string& name);

  virtual StatusCode initialize() override;

  virtual StatusCode selectTracks(
      TrackAnalysisCollections& trkAnaColls ) override;

  virtual StatusCode selectTracksInRoI(
      TrackAnalysisCollections& trkAnaColls,
      const ElementLink< TrigRoiDescriptorCollection >& roiLink ) override;

private:
  ToolHandle<InDet::IInDetTrackSelectionTool> m_offlineTool{this, "offlineTool", "", "Instance name of track selection tool"};

};

} // namespace IDTPM

#endif // INDETTRACKPERFMON_IDTPM_OFFLINETRACKQUALITYSELECTIONTOOL_H
