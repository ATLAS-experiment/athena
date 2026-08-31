/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCVALIDATION_TGCL0VALIDATIONEVENT_H
#define L0MUONS1TGCVALIDATION_TGCL0VALIDATIONEVENT_H

#include "AthenaKernel/CLASS_DEF.h"

#include <cstdint>
#include <vector>

namespace L0Muon {

/** @brief Common sentinel for unavailable floating-point validation data. */
static constexpr float TgcL0ValidationInvalidValue = 999.F;

/** @brief Sentinel indicating that a matched truth object has no failure code. */
static constexpr std::uint8_t TgcL0ValidationNoUnmatchedReason = 255U;

/** @brief Projection encoding used in the validation segment block. */
enum class TgcL0ValidationProjection : std::uint8_t {
  Wire = 0U,
  Strip = 1U
};

/** @brief Station used as the reconstructed candidate position reference. */
enum class TgcL0ValidationStation : std::uint8_t {
  M1 = 0U,
  M2 = 1U,
  M3 = 2U,
  Invalid = 255U
};

/** @brief Reason why a selected truth muon has no matched candidate. */
enum class TgcL0ValidationUnmatchedReason : std::uint8_t {
  NotFullyExtrapolated = 0U,
  NoCandidateInWindow = 1U,
  CandidateCompetition = 2U
};

/** @brief Event identifiers used by the validation chain. */
struct TgcL0ValidationEventInfo {
  std::uint32_t runNumber{0};
  std::uint64_t eventNumber{0};
  std::uint32_t lumiBlock{0};
  std::uint32_t bcid{0};
};

/** @brief Truth and truth-to-reconstruction matching quantities. */
struct TgcL0ValidationTruthBlock {
  std::vector<int> pdgId;
  std::vector<int> barcode;
  std::vector<float> pt;
  std::vector<float> eta;
  std::vector<float> phi;
  std::vector<float> charge;
  std::vector<std::uint8_t> extrapolatedStationMask;
  std::vector<float> m1Eta;
  std::vector<float> m1Phi;
  std::vector<float> m2Eta;
  std::vector<float> m2Phi;
  std::vector<float> m3Eta;
  std::vector<float> m3Phi;
  std::vector<std::uint8_t> matched;
  std::vector<int> matchedCandidateIndex;
  std::vector<float> matchMeanDeltaR;
  std::vector<std::uint8_t> unmatchedReason;
  std::vector<std::uint8_t> finalCandidateMatched;
  std::vector<int> matchedFinalCandidateIndex;
  std::vector<float> finalCandidateMatchDeltaR;
  std::vector<std::uint8_t> finalCandidateUnmatchedReason;
  std::vector<std::uint8_t> wireSegmentMatched;
  std::vector<std::uint8_t> stripSegmentMatched;
  std::vector<int> matchedWireSegmentIndex;
  std::vector<int> matchedStripSegmentIndex;
  std::vector<float> wireSegmentMatchResidual;
  std::vector<float> stripSegmentMatchResidual;
};

/** @brief Projection-segment quantities published by reconstruction. */
struct TgcL0ValidationSegmentBlock {
  std::vector<std::uint16_t> subdetectorId;
  std::vector<std::uint16_t> triggerSector;
  std::vector<std::uint16_t> bcTag;
  std::vector<std::uint8_t> projection;
  std::vector<std::uint8_t> stationMask;
  std::vector<std::uint8_t> summedQuality;
  std::vector<std::uint8_t> nStations;
  std::vector<float> eta;
  std::vector<float> phi;
  std::vector<float> residual;
  std::vector<float> outputResidual;
  std::vector<float> consistency;
  std::vector<std::uint16_t> pivotChannel;
  std::vector<int> truthIndex;
  std::vector<float> truthMatchResidual;
};

/** @brief Reconstruction quantities exposed to validation backends. */
struct TgcL0ValidationCandidateBlock {
  std::vector<std::uint16_t> subdetectorId;
  std::vector<std::uint16_t> triggerSector;
  std::vector<std::uint16_t> readoutSector;
  std::vector<std::uint16_t> bcTag;
  std::vector<std::uint8_t> stationMask;
  std::vector<std::uint8_t> wireStationMask;
  std::vector<std::uint8_t> stripStationMask;
  std::vector<float> eta;
  std::vector<float> phi;
  std::vector<float> deltaTheta;
  std::vector<float> deltaPhi;
  std::vector<float> pt;
  std::vector<std::uint8_t> threshold;
  std::vector<std::int8_t> charge;
  std::vector<std::uint8_t> goodMagneticField;
  std::vector<int> truthIndex;
};

/** @brief Final post-selection TGC candidates and their truth matching. */
struct TgcL0ValidationFinalCandidateBlock {
  std::vector<int> sourceCandidateIndex;
  std::vector<std::uint8_t> referenceStation;
  std::vector<std::uint16_t> subdetectorId;
  std::vector<std::uint16_t> triggerSector;
  std::vector<std::uint16_t> bcTag;
  std::vector<std::uint8_t> tcId;
  std::vector<std::uint16_t> rawEta;
  std::vector<std::uint16_t> rawPhi;
  std::vector<float> eta;
  std::vector<float> phi;
  std::vector<std::uint8_t> ptCode;
  std::vector<float> pt;
  std::vector<std::uint8_t> threshold;
  std::vector<std::int8_t> charge;
  std::vector<std::uint8_t> innerCoincidence;
  std::vector<std::uint8_t> goodMagneticField;
  std::vector<int> truthIndex;
  std::vector<float> truthMatchDeltaR;
};

/** @brief TGC Sector Logic words and their placeholder metadata. */
struct TgcL0ValidationSectorLogicBlock {
  std::vector<std::uint32_t> inputCandidateIndex;
  std::vector<std::uint32_t> candWord;
  std::vector<std::uint32_t> candExtraWord;
  std::vector<std::uint16_t> boardId;
  std::vector<std::uint16_t> fiberId;
  std::vector<int> bcidOffset;
  std::vector<std::uint16_t> veto;
};

/** @brief ROOT-independent validation data for one event. */
struct TgcL0ValidationEvent {
  TgcL0ValidationEventInfo event;
  TgcL0ValidationTruthBlock truth;
  TgcL0ValidationSegmentBlock segments;
  TgcL0ValidationCandidateBlock candidates;
  TgcL0ValidationFinalCandidateBlock finalCandidates;
  TgcL0ValidationSectorLogicBlock sectorLogic;
};

}  // namespace L0Muon

CLASS_DEF(L0Muon::TgcL0ValidationEvent, 1316895011, 1)

#endif
