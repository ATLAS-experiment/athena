/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCFLOATINGTOOLS_FLOATINGTRACKSELECTOR_H
#define L1MUONS1TGCFLOATINGTOOLS_FLOATINGTRACKSELECTOR_H

#include <cstddef>

#include "L1MuonS1TGCToolInterfaces/TgcSLCandidate.h"
#include "xAODTrigL1Muon/L1TGCCandDataContainer.h"

namespace L1Muon::L1TGC {

/** @brief Select and convert floating-point TGC candidates. */
class FloatingTrackSelector {
 public:
  /**
   * @brief Fill the xAOD output with the six highest-threshold candidates per
   * sector for one BC processing chain.
   * @param candidates Input candidates after Inner Coincidence and BC-chain
   * selection.
   * @param output Output TGC candidate container.
   */
  void select(const TgcSLCandidateContainer &candidates,
              xAOD::L1TGCCandDataContainer &output) const;

 private:
  // Fixed by the TGC Sector Logic output interface; this is not a
  // configurable cut.
  static constexpr std::size_t s_maxCandidatesPerSector{6U};
};

}  // namespace L1Muon::L1TGC

#endif
