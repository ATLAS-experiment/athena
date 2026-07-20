/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCINTERFACES_TGCL0CANDIDATE_H
#define L0MUONS1TGCINTERFACES_TGCL0CANDIDATE_H

#include <cstdint>
#include <vector>

namespace L0Muon {

/**
 * @brief Lightweight transient candidate exchanged between TGC tool stages.
 *
 * This type represents internal pre- and post-Inner-Coincidence state.  It is
 * deliberately separate from xAOD::TGCCandData, which is the finalized
 * downstream output of L0MuonS1TGC for MDTTP and L0MuonEndcap.
 */
struct TgcL0Candidate {
  /// Candidate identifier within one trigger sector. Zero denotes no candidate.
  std::uint8_t tcId{0};
  /// TGC pT-threshold index before Inner Coincidence.
  std::uint8_t preInnerCoincidencePtThreshold{0};
  /// Operative TGC pT-threshold index after Inner Coincidence.
  std::uint8_t ptThreshold{0};
};

/// Transient collection passed between the TGC processing tools.
using TgcL0CandidateContainer = std::vector<TgcL0Candidate>;

}  // namespace L0Muon

#endif
