/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "BitwiseInnerCoincidenceTool.h"

namespace L1Muon::L1TGC {

StatusCode BitwiseInnerCoincidenceTool::apply(TgcSLCandidateContainer& candidates, const EventContext& ctx) const {
  (void)candidates;
  (void)ctx;
  return StatusCode::SUCCESS;
}

}  // namespace L1Muon::L1TGC
