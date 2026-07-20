/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0BitwiseTrackSelectorTool.h"

namespace L0Muon {

StatusCode TgcL0BitwiseTrackSelectorTool::select(const TgcL0CandidateContainer& candidates, xAOD::TGCCandDataContainer& output, const EventContext& ctx) const {
  (void)candidates;
  (void)output;
  (void)ctx;
  return StatusCode::SUCCESS;
}

}  // namespace L0Muon
