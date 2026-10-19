/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCFLOATINGTOOLS_FLOATINGOVERLAPCLASSIFICATION_H
#define L1MUONS1TGCFLOATINGTOOLS_FLOATINGOVERLAPCLASSIFICATION_H

#include "L1MuonS1TGCToolInterfaces/TgcSLCandidate.h"

#include <cstddef>

namespace L1Muon::L1TGC {

struct FloatingOverlapClassificationConfig {
  /// Candidate-position windows used only to identify possible chamber overlap.
  float maxDeltaEta{0.02F};
  float maxDeltaPhi{0.03F};
  /// Require this many common reconstructed station positions.
  std::size_t minCommonStations{2U};
};

struct FloatingOverlapClassificationStatistics {
  std::size_t nGroups{0};
  std::size_t nCandidates{0};
  std::size_t maxMultiplicity{0};
};

/** @brief Tag nearby candidates from distinct chamber paths.
 *
 * No candidate is removed or ranked. The tags provide event-local provenance
 * for a later overlap-removal stage, after pT and Inner-Coincidence information
 * are available.
 */
class FloatingOverlapClassification {
 public:
  explicit FloatingOverlapClassification(
      FloatingOverlapClassificationConfig config = FloatingOverlapClassificationConfig{});

  void classify(TgcSLCandidateContainer& candidates,
                FloatingOverlapClassificationStatistics& statistics) const;

 private:
  FloatingOverlapClassificationConfig m_config{};
};

}  // namespace L1Muon::L1TGC

#endif
