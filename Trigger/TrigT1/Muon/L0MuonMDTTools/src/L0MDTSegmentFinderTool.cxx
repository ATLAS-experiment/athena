/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "L0MuonMDTTools/L0MDTSegmentFinderTool.h"
#include "L0MuonMDTTools/L0MDTSegment.h"


namespace L0MDT {


  StatusCode L0MDTSegmentFinderTool::initialize() {

    return StatusCode::SUCCESS;
  }
    StatusCode L0MDTSegmentFinderTool::findSegments(const std::vector<const xAOD::MdtDriftCircle*>& driftCircles, const EventContext& /*ctx*/,
                                   std::vector<L0MDT::Segment>& segments) const {
    ATH_MSG_DEBUG("In L0MDTSegmentFinderTool::findSegments()");
    // printing the size of the segments vector to avoid the unused variable warning
    ATH_MSG_DEBUG("Size of drift circles vector: " << driftCircles.size() << ", size of segments vector: " << segments.size());

    return StatusCode::SUCCESS;
  }
} // end of namespace

