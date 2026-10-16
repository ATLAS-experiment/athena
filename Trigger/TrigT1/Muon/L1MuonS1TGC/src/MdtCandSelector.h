/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGC_MDTCANDSELECTOR_H
#define L1MUONS1TGC_MDTCANDSELECTOR_H

#include <cstddef>
#include <memory>

#include "xAODTrigL1Muon/L1TGCCandDataContainer.h"

namespace L1Muon::L1TGC {

/** @brief Select the TGC candidates sent to MDTTP. */
class MdtCandSelector {
 public:
  /**
   * @brief Create a non-owning view of the three candidates with the highest
   * encoded pT per sector for one BC processing chain.
   * @param candidates Candidate collection retained by the Sector Logic for
   * one already-selected BC chain. The container is non-const because
   * DataVector views store non-const pointers; its elements are not modified.
   * @return Non-owning MDTTP candidate view.
   */
  std::unique_ptr<xAOD::L1TGCCandDataContainer> select(
      xAOD::L1TGCCandDataContainer &candidates) const;

 private:
  // Fixed by the TGC-to-MDTTP interface; this is not a configurable cut.
  static constexpr std::size_t s_maxCandidatesPerSector{3U};
};

}  // namespace L1Muon::L1TGC

#endif
