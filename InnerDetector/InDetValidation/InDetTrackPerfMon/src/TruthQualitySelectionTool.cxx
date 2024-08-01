/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthQualitySelectionTool.h"
#include <GaudiKernel/StatusCode.h>
#include "TrackAnalysisCollections.h"

IDTPM::TruthQualitySelectionTool::TruthQualitySelectionTool( const std::string& name )
  : asg::AsgTool( name ) { }

StatusCode IDTPM::TruthQualitySelectionTool::initialize() {
  ATH_CHECK(not m_truthTool.empty());
  ATH_CHECK(m_truthTool.retrieve());
  return StatusCode::SUCCESS;
}


StatusCode IDTPM::TruthQualitySelectionTool::selectTracks( TrackAnalysisCollections& trkAnaColls ) {
  std::vector< const xAOD::TruthParticle* > selected;
  for ( auto trk: trkAnaColls.truthPartVec(IDTPM::TrackAnalysisCollections::FS)) {
    if ( m_truthTool->accept(trk)) {
      selected.push_back(trk);
    }
  }
  ATH_MSG_DEBUG("Size before selection: " << trkAnaColls.truthPartVec(IDTPM::TrackAnalysisCollections::FS).size() << "\t Size after selection: " << selected.size());
  ATH_CHECK(trkAnaColls.fillTruthPartVec(selected, IDTPM::TrackAnalysisCollections::FS));
  return StatusCode::SUCCESS;
}

