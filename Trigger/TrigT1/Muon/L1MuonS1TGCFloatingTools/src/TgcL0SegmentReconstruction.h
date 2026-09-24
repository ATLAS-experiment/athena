/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCFLOATINGTOOLS_TGCL0SEGMENTRECONSTRUCTION_H
#define L1MUONS1TGCFLOATINGTOOLS_TGCL0SEGMENTRECONSTRUCTION_H

#include "GaudiKernel/StatusCode.h"
#include "L1MuonS1TGCToolInterfaces/TgcL0Candidate.h"
#include "L1MuonS1TGCToolInterfaces/TgcL0Segment.h"
#include "TgcL0FloatingData.h"

#include <cstddef>

namespace L0Muon {
namespace TgcL0Floating {

struct SegmentReconstructionConfig {
  /// Broad efficiency-first Floating candidate windows.
  float maxCandidateDThetaAbs{0.10F};
  float maxCandidateDPhiAbs{0.10F};

  /// Common-pivot wire-strip association. Candidate eta is measured by wire
  /// and candidate phi by strip. The auxiliary phi check reproduces the loose
  /// geometry fallback of the validated Floating simulation.
  float maxPivotWireStripDeltaEta{-1.F};
  float maxPivotWireStripDeltaPhi{0.35F};

  /// Validated Floating working-set size per side/Trigger-Sector/BC chain.
  /// This is independent of the later six-candidate Track Selector limit.
  std::size_t maxSegmentCombinationsPerGroup{8U};

  /// Old Floating local eta-phi-pivot candidate-bin pruning.
  std::size_t maxCandidatesPerLocalBin{8U};
  float localCandidateEtaBinWidth{0.01F};
  float localCandidatePhiBinWidth{0.01F};

  /// Same-chamber duplicate suppression after wire-strip association.
  std::size_t maxCandidatesPerLocalPosition{1U};
  float localDuplicateEtaWindow{0.02F};
  float localDuplicatePhiWindow{0.03F};
};

struct SegmentStatistics {
  std::size_t nWireSegments{0};
  std::size_t nStripSegments{0};
  std::size_t nM1M2M3Candidates{0};
  std::size_t nM1M2Candidates{0};
  std::size_t nM1M3Candidates{0};
  std::size_t nM2M3Candidates{0};
  std::size_t nM1OnlyPositionCandidates{0};
  std::size_t nM2OnlyPositionCandidates{0};
  std::size_t nM3OnlyPositionCandidates{0};
  std::size_t nRejectedProjectionCombinations{0};
  std::size_t nDuplicateProjectionSegments{0};
  std::size_t nLimitedProjectionSegments{0};
  std::size_t nRejectedPairs{0};
  std::size_t nDuplicateCandidates{0};
  std::size_t nLocalDuplicateCandidates{0};
  std::size_t nLimitedCandidates{0};
};

/** @brief Reconstruct same-BC inter-station segments and transient candidates. */
class SegmentReconstruction {
 public:
  explicit SegmentReconstruction(
      SegmentReconstructionConfig config = SegmentReconstructionConfig{});

  StatusCode build(const StationCoincidenceContainer& coincidences,
                   TgcL0CandidateContainer& candidates,
                   SegmentStatistics& statistics,
                   TgcL0SegmentContainer* validationSegments = nullptr) const;

 private:
  SegmentReconstructionConfig m_config{};
};

}  // namespace TgcL0Floating
}  // namespace L0Muon

#endif
