/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "BitwiseTrackSelectorTool.h"

namespace L1Muon::L1TGC {

StatusCode BitwiseTrackSelectorTool::select(const TgcSLCandidateContainer& candidates, xAOD::L1TGCCandDataContainer& output, const EventContext& ctx) const {
  (void)candidates;
  (void)output;
  (void)ctx;
  return StatusCode::SUCCESS;
}

}  // namespace L1Muon::L1TGC
