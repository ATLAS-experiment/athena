/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FloatingInnerCoincidenceTool.h"

#include "FloatingInnerCoincidence.h"

namespace L1Muon::L1TGC {

StatusCode FloatingInnerCoincidenceTool::apply(
    TgcSLCandidateContainer &candidates, const EventContext &ctx) const {
  (void)ctx;
  const FloatingInnerCoincidence innerCoincidence;
  innerCoincidence.apply(candidates);
  return StatusCode::SUCCESS;
}

}  // namespace L1Muon::L1TGC
