/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCFLOATINGTOOLS_FLOATINGINNERCOINCIDENCE_H
#define L1MUONS1TGCFLOATINGTOOLS_FLOATINGINNERCOINCIDENCE_H

#include "L1MuonS1TGCToolInterfaces/TgcSLCandidate.h"

namespace L1Muon::L1TGC {

/** @brief Apply the floating-point Inner-Coincidence response. */
class FloatingInnerCoincidence {
 public:
  /**
   * @brief Propagate the pre-Inner result when no inner-detector input is
   * configured.
   * @param candidates Candidates to update.
   */
  void apply(TgcSLCandidateContainer &candidates) const;
};

}  // namespace L1Muon::L1TGC

#endif
