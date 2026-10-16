/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "BitwiseCandidateBuilderTool.h"

namespace L1Muon::L1TGC {

StatusCode BitwiseCandidateBuilderTool::build(const TgcRdoContainer& rdos, TgcSLCandidateContainer& candidates, const EventContext& ctx) const {
  (void)rdos;
  (void)candidates;
  (void)ctx;
  return StatusCode::SUCCESS;
}

}  // namespace L1Muon::L1TGC
