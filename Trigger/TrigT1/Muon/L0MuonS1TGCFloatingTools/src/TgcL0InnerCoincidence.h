/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCFLOATINGTOOLS_TGCL0INNERCOINCIDENCE_H
#define L0MUONS1TGCFLOATINGTOOLS_TGCL0INNERCOINCIDENCE_H

#include "L0MuonS1TGCToolInterfaces/TgcL0Candidate.h"

namespace L0Muon {
namespace TgcL0Floating {

/** @brief Apply the floating-point Inner-Coincidence response. */
class InnerCoincidence {
 public:
  /**
   * @brief Propagate the pre-Inner result when no inner-detector input is
   * configured.
   * @param candidates Candidates to update.
   */
  void apply(TgcL0CandidateContainer &candidates) const;
};

}  // namespace TgcL0Floating
}  // namespace L0Muon

#endif
