/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCFLOATINGTOOLS_TGCL0OVERLAPCLASSIFICATION_H
#define L0MUONS1TGCFLOATINGTOOLS_TGCL0OVERLAPCLASSIFICATION_H

#include "L0MuonS1TGCToolInterfaces/TgcL0Candidate.h"

#include <cstddef>

namespace L0Muon {
namespace TgcL0Floating {

struct OverlapClassificationConfig {
  /// Candidate-position windows used only to identify possible chamber overlap.
  float maxDeltaEta{0.02F};
  float maxDeltaPhi{0.03F};
  /// Require this many common reconstructed station positions.
  std::size_t minCommonStations{2U};
};

struct OverlapClassificationStatistics {
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
class OverlapClassification {
 public:
  explicit OverlapClassification(
      OverlapClassificationConfig config = OverlapClassificationConfig{});

  void classify(TgcL0CandidateContainer& candidates,
                OverlapClassificationStatistics& statistics) const;

 private:
  OverlapClassificationConfig m_config{};
};

}  // namespace TgcL0Floating
}  // namespace L0Muon

#endif
