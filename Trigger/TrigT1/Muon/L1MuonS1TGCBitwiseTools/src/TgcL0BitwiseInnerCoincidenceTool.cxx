/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0BitwiseInnerCoincidenceTool.h"

namespace L1Muon {

StatusCode TgcL0BitwiseInnerCoincidenceTool::apply(TgcL0CandidateContainer& candidates, const EventContext& ctx) const {
  (void)candidates;
  (void)ctx;
  return StatusCode::SUCCESS;
}

}  // namespace L1Muon
