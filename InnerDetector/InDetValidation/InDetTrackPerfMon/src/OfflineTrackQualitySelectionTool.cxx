/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "OfflineTrackQualitySelectionTool.h"
#include "TrackAnalysisCollections.h"
namespace IDTPM {

OfflineTrackQualitySelectionTool::OfflineTrackQualitySelectionTool(const std::string& name)
  : asg::AsgTool( name ) {}

StatusCode OfflineTrackQualitySelectionTool::initialize() {
  ATH_CHECK( asg::AsgTool::initialize() );  
  ATH_CHECK( m_offlineTool.retrieve() );
  return StatusCode::SUCCESS;
}

StatusCode OfflineTrackQualitySelectionTool::selectTracks(
    TrackAnalysisCollections& trkAnaColls) {

  std::vector< const xAOD::TrackParticle* > selected;
  for ( auto trkPtr: trkAnaColls.offlTrackVec(TrackAnalysisCollections::FS)) {
    if ( m_offlineTool->accept(trkPtr)) // TODO vertex needs to be provided here
      selected.push_back(trkPtr);
  }
  ATH_MSG_DEBUG("Out of " << trkAnaColls.offlTrackVec(TrackAnalysisCollections::FS).size() << " tracks, selected " << selected.size() );
  ATH_CHECK(trkAnaColls.fillOfflTrackVec(selected, TrackAnalysisCollections::FS));
  return StatusCode::SUCCESS;
}

StatusCode OfflineTrackQualitySelectionTool::selectTracksInRoI(
    TrackAnalysisCollections& /*trkAnaColls*/,
    const ElementLink<TrigRoiDescriptorCollection>& /*roiLink*/) {
  ATH_MSG_FATAL( "using selectTracksInRoI implementation for this tool is an invalid use case" );
  return StatusCode::FAILURE;
}

}  // namespace IDTPM
