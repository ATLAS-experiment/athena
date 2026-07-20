/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCINTERFACES_TGCL0CANDIDATE_H
#define L0MUONS1TGCINTERFACES_TGCL0CANDIDATE_H

#include <cstdint>
#include <vector>

namespace L0Muon {

/** @brief Event-local candidate used by the TGC simulation tools. */
struct TgcL0Candidate {
  /// Subdetector identifier.
  std::uint16_t subdetectorId{0};
  /// Trigger Sector identifier.
  std::uint16_t sectorId{0};
  /// Bunch-crossing tag.
  std::uint16_t bcTag{0};

  /// Pseudorapidity at the TGC pivot plane.
  float eta{0.F};
  /// Azimuth at the TGC pivot plane, in radians.
  float phi{0.F};
  /// Signed polar-angle residual, in radians.
  float deltaTheta{0.F};
  /// Signed azimuthal-angle residual, in radians.
  float deltaPhi{0.F};

  /// TGC pT estimate before Inner Coincidence, in GeV.
  float preInnerCoincidencePt{0.F};
  /// TGC pT estimate after Inner Coincidence, in GeV.
  float pt{0.F};
  /// Highest pT-threshold index before Inner Coincidence.
  std::uint8_t preInnerCoincidenceThreshold{0};
  /// Highest pT-threshold index after Inner Coincidence.
  std::uint8_t threshold{0};
  /// Charge sign: -1, 0, or +1.
  std::int8_t charge{0};
  /// Inner-Coincidence result.
  bool hasInnerCoincidence{false};
  /// GoodMag flag.
  bool goodMagneticField{false};

  /// Trigger Candidate identifier.
  std::uint8_t tcId{0};
  /// Wire quality.
  std::uint8_t wireQuality{0};
  /// Track-Selector priority.
  std::uint8_t selectorPriority{0};
  /// Packed NSW segment information.
  std::uint32_t nswSegment{0};
};

/// Event-local candidate collection.
using TgcL0CandidateContainer = std::vector<TgcL0Candidate>;

}  // namespace L0Muon

#endif
