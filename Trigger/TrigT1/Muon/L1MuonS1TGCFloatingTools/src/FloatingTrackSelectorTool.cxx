/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FloatingTrackSelectorTool.h"

#include "FloatingTrackSelector.h"

namespace L1Muon::L1TGC {

StatusCode FloatingTrackSelectorTool::select(
    const TgcSLCandidateContainer &candidates,
    xAOD::L1TGCCandDataContainer &output, const EventContext &ctx) const {
  (void)ctx;
  const FloatingTrackSelector selector;
  selector.select(candidates, output);
  return StatusCode::SUCCESS;
}

}  // namespace L1Muon::L1TGC
