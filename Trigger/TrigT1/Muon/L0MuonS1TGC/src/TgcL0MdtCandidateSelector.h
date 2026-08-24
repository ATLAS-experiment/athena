/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGC_TGCL0MDTCANDIDATESELECTOR_H
#define L0MUONS1TGC_TGCL0MDTCANDIDATESELECTOR_H

#include <cstddef>
#include <memory>

#include "xAODL0MuonCand/TGCCandDataContainer.h"

namespace L0Muon {

/** @brief Select the TGC candidates sent to MDTTP. */
class TgcL0MdtCandidateSelector {
 public:
  /**
   * @brief Create a non-owning view of the three candidates with the highest
   * encoded pT per sector for one BC processing chain.
   * @param candidates Candidate collection retained by the Sector Logic for
   * one already-selected BC chain. The container is non-const because
   * DataVector views store non-const pointers; its elements are not modified.
   * @return Non-owning MDTTP candidate view.
   */
  std::unique_ptr<xAOD::TGCCandDataContainer> select(
      xAOD::TGCCandDataContainer &candidates) const;

 private:
  // Fixed by the TGC-to-MDTTP interface; this is not a configurable cut.
  static constexpr std::size_t s_maxCandidatesPerSector{3U};
};

}  // namespace L0Muon

#endif
