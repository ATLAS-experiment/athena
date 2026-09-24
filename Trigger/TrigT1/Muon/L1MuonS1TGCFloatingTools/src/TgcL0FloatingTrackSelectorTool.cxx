/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0FloatingTrackSelectorTool.h"

#include "TgcL0TrackSelector.h"

namespace L0Muon {

StatusCode TgcL0FloatingTrackSelectorTool::select(
    const TgcL0CandidateContainer &candidates,
    xAOD::TGCCandDataContainer &output, const EventContext &ctx) const {
  (void)ctx;
  const TgcL0Floating::TrackSelector selector;
  selector.select(candidates, output);
  return StatusCode::SUCCESS;
}

}  // namespace L0Muon
