/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0FloatingInnerCoincidenceTool.h"

#include "TgcL0InnerCoincidence.h"

namespace L0Muon {

StatusCode TgcL0FloatingInnerCoincidenceTool::apply(
    TgcL0CandidateContainer &candidates, const EventContext &ctx) const {
  (void)ctx;
  const TgcL0Floating::InnerCoincidence innerCoincidence;
  innerCoincidence.apply(candidates);
  return StatusCode::SUCCESS;
}

}  // namespace L0Muon
