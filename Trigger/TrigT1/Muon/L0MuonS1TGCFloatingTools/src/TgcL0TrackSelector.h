/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCFLOATINGTOOLS_TGCL0TRACKSELECTOR_H
#define L0MUONS1TGCFLOATINGTOOLS_TGCL0TRACKSELECTOR_H

#include <cstddef>

#include "L1MuonS1TGCToolInterfaces/TgcL0Candidate.h"
#include "xAODL0MuonCand/TGCCandDataContainer.h"

namespace L0Muon {
namespace TgcL0Floating {

/** @brief Select and convert floating-point TGC candidates. */
class TrackSelector {
 public:
  /**
   * @brief Fill the xAOD output with the six highest-threshold candidates per
   * sector for one BC processing chain.
   * @param candidates Input candidates after Inner Coincidence and BC-chain
   * selection.
   * @param output Output TGC candidate container.
   */
  void select(const TgcL0CandidateContainer &candidates,
              xAOD::TGCCandDataContainer &output) const;

 private:
  // Fixed by the TGC Sector Logic output interface; this is not a
  // configurable cut.
  static constexpr std::size_t s_maxCandidatesPerSector{6U};
};

}  // namespace TgcL0Floating
}  // namespace L0Muon

#endif
