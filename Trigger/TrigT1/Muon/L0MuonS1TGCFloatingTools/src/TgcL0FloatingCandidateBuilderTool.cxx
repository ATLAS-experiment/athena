/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0FloatingCandidateBuilderTool.h"

namespace L0Muon {

StatusCode TgcL0FloatingCandidateBuilderTool::build(const TgcRdoContainer& rdos, TgcL0CandidateContainer& candidates, const EventContext& ctx) const {
  (void)rdos;
  (void)candidates;
  (void)ctx;
  return StatusCode::SUCCESS;
}

}  // namespace L0Muon
