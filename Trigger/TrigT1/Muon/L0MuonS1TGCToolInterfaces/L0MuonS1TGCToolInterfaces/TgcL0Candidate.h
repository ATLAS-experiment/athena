/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCINTERFACES_TGCL0CANDIDATE_H
#define L0MUONS1TGCINTERFACES_TGCL0CANDIDATE_H

#include "AthenaKernel/CLASS_DEF.h"

#include <cstdint>
#include <vector>

namespace L0Muon {

/** @brief Event-local candidate used by the TGC simulation tools. */
struct TgcL0Candidate {
  /// Subdetector identifier.
  std::uint16_t subdetectorId{0};
  /// Trigger Sector identifier.
  std::uint16_t sectorId{0};
  /// Run-3 detector/readout sector retained for diagnostics.
  std::uint16_t readoutSector{0};
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

  /// Stations with a complete two-dimensional wire-and-strip position.
  std::uint8_t stationMask{0};
  /// Stations used by the wire projection segment.
  std::uint8_t wireStationMask{0};
  /// Stations used by the strip projection segment.
  std::uint8_t stripStationMask{0};
  /// Station-coincidence qualities for wire projections.
  std::uint8_t m1WireQuality{0};
  std::uint8_t m2WireQuality{0};
  std::uint8_t m3WireQuality{0};
  /// Station-coincidence qualities for strip projections.
  std::uint8_t m1StripQuality{0};
  std::uint8_t m2StripQuality{0};
  std::uint8_t m3StripQuality{0};
  /// Representative pivot channels retained for diagnostics.
  std::uint16_t pivotWireChannel{0};
  std::uint16_t pivotStripChannel{0};

  /// Alias of stationMask retained for explicit validation/debug use.
  std::uint8_t positionStationMask{0};
  /// Reconstructed station positions used only by validation/debug code.
  float m1Eta{0.F};
  float m1Phi{0.F};
  float m2Eta{0.F};
  float m2Phi{0.F};
  float m3Eta{0.F};
  float m3Phi{0.F};
  /// Offline chamber identifiers retained for overlap diagnostics.
  std::int16_t m1StationEta{0};
  std::uint16_t m1StationPhi{0};
  std::int16_t m2StationEta{0};
  std::uint16_t m2StationPhi{0};
  std::int16_t m3StationEta{0};
  std::uint16_t m3StationPhi{0};

  /// Event-local overlap group. Zero means no classified overlap partner.
  std::uint16_t overlapGroupId{0};
  /// Number of candidates in the classified overlap group.
  std::uint8_t overlapMultiplicity{1};
  /// True when the candidate has a nearby partner from a different chamber path.
  /// This is a candidate-level classification, not a detector-active-area test.
  bool inChamberOverlap{false};
};

/// Event-local candidate collection.
using TgcL0CandidateContainer = std::vector<TgcL0Candidate>;

}  // namespace L0Muon

CLASS_DEF(L0Muon::TgcL0CandidateContainer, 1316895010, 1)

#endif
