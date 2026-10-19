/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FloatingInnerCoincidence.h"

namespace L1Muon::L1TGC {

void FloatingInnerCoincidence::apply(TgcSLCandidateContainer &candidates) const {
  for (TgcSLCandidate &candidate : candidates) {
    candidate.pt = candidate.preInnerCoincidencePt;
    candidate.threshold = candidate.preInnerCoincidenceThreshold;
    candidate.hasInnerCoincidence = false;
  }
}

}  // namespace L1Muon::L1TGC
