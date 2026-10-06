/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0InnerCoincidence.h"

namespace L1Muon {
namespace TgcL0Floating {

void InnerCoincidence::apply(TgcL0CandidateContainer &candidates) const {
  for (TgcL0Candidate &candidate : candidates) {
    candidate.pt = candidate.preInnerCoincidencePt;
    candidate.threshold = candidate.preInnerCoincidenceThreshold;
    candidate.hasInnerCoincidence = false;
  }
}

}  // namespace TgcL0Floating
}  // namespace L1Muon
