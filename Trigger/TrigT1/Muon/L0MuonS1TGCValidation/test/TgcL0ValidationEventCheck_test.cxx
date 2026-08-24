/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "L0MuonS1TGCValidation/TgcL0ValidationEventCheck.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

using L0Muon::TgcL0ValidationEvent;
using L0Muon::TgcL0ValidationInvalidValue;
using L0Muon::TgcL0ValidationNoUnmatchedReason;
using L0Muon::TgcL0ValidationProjection;
using L0Muon::TgcL0ValidationUnmatchedReason;

TgcL0ValidationEvent makeValidEvent() {
  TgcL0ValidationEvent event;

  event.truth.pdgId = {13};
  event.truth.barcode = {1};
  event.truth.pt = {10000.F};
  event.truth.eta = {1.5F};
  event.truth.phi = {0.25F};
  event.truth.charge = {-1.F};
  event.truth.extrapolatedStationMask = {0x7U};
  event.truth.m1Eta = {1.51F};
  event.truth.m1Phi = {0.24F};
  event.truth.m2Eta = {1.50F};
  event.truth.m2Phi = {0.25F};
  event.truth.m3Eta = {1.49F};
  event.truth.m3Phi = {0.26F};
  event.truth.matched = {1U};
  event.truth.matchedCandidateIndex = {0};
  event.truth.matchMeanDeltaR = {0.01F};
  event.truth.unmatchedReason = {TgcL0ValidationNoUnmatchedReason};
  event.truth.wireSegmentMatched = {1U};
  event.truth.stripSegmentMatched = {1U};
  event.truth.matchedWireSegmentIndex = {0};
  event.truth.matchedStripSegmentIndex = {1};
  event.truth.wireSegmentMatchResidual = {0.01F};
  event.truth.stripSegmentMatchResidual = {0.02F};

  event.segments.subdetectorId = {1U, 1U};
  event.segments.triggerSector = {2U, 2U};
  event.segments.bcTag = {0U, 0U};
  event.segments.projection = {
      static_cast<std::uint8_t>(TgcL0ValidationProjection::Wire),
      static_cast<std::uint8_t>(TgcL0ValidationProjection::Strip)};
  event.segments.stationMask = {0x7U, 0x7U};
  event.segments.summedQuality = {7U, 6U};
  event.segments.nStations = {3U, 3U};
  event.segments.eta = {1.5F, 1.5F};
  event.segments.phi = {0.25F, 0.25F};
  event.segments.residual = {0.001F, 0.002F};
  event.segments.outputResidual = {0.001F, 0.002F};
  event.segments.consistency = {0.0005F, 0.0007F};
  event.segments.pivotChannel = {10U, 20U};
  event.segments.truthIndex = {0, 0};
  event.segments.truthMatchResidual = {0.01F, 0.02F};

  event.candidates.subdetectorId = {1U};
  event.candidates.triggerSector = {2U};
  event.candidates.readoutSector = {3U};
  event.candidates.bcTag = {0U};
  event.candidates.stationMask = {0x7U};
  event.candidates.wireStationMask = {0x7U};
  event.candidates.stripStationMask = {0x7U};
  event.candidates.eta = {1.5F};
  event.candidates.phi = {0.25F};
  event.candidates.deltaTheta = {0.001F};
  event.candidates.deltaPhi = {0.002F};
  event.candidates.pt = {25.F};
  event.candidates.threshold = {6U};
  event.candidates.charge = {-1};
  event.candidates.goodMagneticField = {1U};
  event.candidates.truthIndex = {0};

  event.sectorLogic.inputCandidateIndex = {0U};
  event.sectorLogic.candWord = {0x12345678U};
  event.sectorLogic.candExtraWord = {0x9abcdef0U};
  event.sectorLogic.boardId = {0U};
  event.sectorLogic.fiberId = {0U};
  event.sectorLogic.bcidOffset = {0};
  event.sectorLogic.veto = {0U};

  return event;
}

void appendUnmatchedTruth(TgcL0ValidationEvent& event,
                          const TgcL0ValidationUnmatchedReason reason) {
  event.truth.pdgId.emplace_back(-13);
  event.truth.barcode.emplace_back(2);
  event.truth.pt.emplace_back(12000.F);
  event.truth.eta.emplace_back(1.52F);
  event.truth.phi.emplace_back(0.26F);
  event.truth.charge.emplace_back(1.F);
  event.truth.extrapolatedStationMask.emplace_back(0x7U);
  event.truth.m1Eta.emplace_back(1.53F);
  event.truth.m1Phi.emplace_back(0.25F);
  event.truth.m2Eta.emplace_back(1.52F);
  event.truth.m2Phi.emplace_back(0.26F);
  event.truth.m3Eta.emplace_back(1.51F);
  event.truth.m3Phi.emplace_back(0.27F);
  event.truth.matched.emplace_back(0U);
  event.truth.matchedCandidateIndex.emplace_back(-1);
  event.truth.matchMeanDeltaR.emplace_back(TgcL0ValidationInvalidValue);
  event.truth.unmatchedReason.emplace_back(static_cast<std::uint8_t>(reason));
  event.truth.wireSegmentMatched.emplace_back(0U);
  event.truth.stripSegmentMatched.emplace_back(0U);
  event.truth.matchedWireSegmentIndex.emplace_back(-1);
  event.truth.matchedStripSegmentIndex.emplace_back(-1);
  event.truth.wireSegmentMatchResidual.emplace_back(
      TgcL0ValidationInvalidValue);
  event.truth.stripSegmentMatchResidual.emplace_back(
      TgcL0ValidationInvalidValue);
}

bool expectValid(const TgcL0ValidationEvent& event, const std::string& label) {
  const auto result = L0Muon::checkTgcL0ValidationEvent(event);
  if (!result.valid) {
    std::cerr << label << " unexpectedly failed: " << result.message << '\n';
    return false;
  }
  return true;
}

bool expectInvalid(const TgcL0ValidationEvent& event,
                   const std::string& expectedMessage,
                   const std::string& label) {
  const auto result = L0Muon::checkTgcL0ValidationEvent(event);
  if (result.valid) {
    std::cerr << label << " unexpectedly passed\n";
    return false;
  }
  if (result.message != expectedMessage) {
    std::cerr << label << " returned unexpected message: " << result.message
              << " (expected: " << expectedMessage << ")\n";
    return false;
  }
  return true;
}

}  // namespace

int main() {
  bool success = true;

  success &= expectValid(makeValidEvent(), "valid event");

  {
    auto event = makeValidEvent();
    appendUnmatchedTruth(
        event, TgcL0ValidationUnmatchedReason::CandidateCompetition);
    success &= expectValid(event, "one-to-one competition result");
  }
  {
    auto event = makeValidEvent();
    event.truth.pt.clear();
    success &= expectInvalid(event, "Truth block size mismatch for pt",
                             "truth-size mismatch");
  }
  {
    auto event = makeValidEvent();
    event.segments.phi.pop_back();
    success &= expectInvalid(event, "Segment block size mismatch for phi",
                             "segment-size mismatch");
  }
  {
    auto event = makeValidEvent();
    event.segments.projection[0] = 2U;
    success &= expectInvalid(event,
                             "Segment projection is not wire or strip",
                             "invalid segment projection");
  }
  {
    auto event = makeValidEvent();
    event.segments.nStations[0] = 0U;
    success &= expectInvalid(event,
                             "Segment station count is out of range",
                             "invalid segment station count");
  }
  {
    auto event = makeValidEvent();
    event.candidates.pt.clear();
    success &= expectInvalid(event,
                             "Candidate block size mismatch for pt",
                             "candidate-size mismatch");
  }
  {
    auto event = makeValidEvent();
    event.candidates.goodMagneticField[0] = 2U;
    success &= expectInvalid(event,
                             "Candidate GoodMag flag is not binary",
                             "invalid GoodMag flag");
  }
  {
    auto event = makeValidEvent();
    event.sectorLogic.candExtraWord.clear();
    success &= expectInvalid(
        event, "Sector Logic block size mismatch for candExtraWord",
        "sector-logic-size mismatch");
  }
  {
    auto event = makeValidEvent();
    event.sectorLogic.boardId[0] = 1U;
    success &= expectInvalid(
        event, "Sector Logic placeholder metadata is not zero",
        "sector-logic placeholder metadata");
  }
  {
    auto event = makeValidEvent();
    event.truth.matched[0] = 2U;
    success &= expectInvalid(event, "Truth matched flag is not binary",
                             "non-binary matched flag");
  }
  {
    auto event = makeValidEvent();
    event.truth.matchedCandidateIndex[0] = 1;
    success &= expectInvalid(
        event, "Truth matched-candidate index is out of range",
        "truth-to-candidate range");
  }
  {
    auto event = makeValidEvent();
    event.truth.matchedCandidateIndex[0] = -1;
    success &= expectInvalid(event, "Matched truth has no candidate index",
                             "missing matched candidate");
  }
  {
    auto event = makeValidEvent();
    event.truth.matched[0] = 0U;
    success &= expectInvalid(event, "Unmatched truth has a candidate index",
                             "unmatched truth candidate index");
  }
  {
    auto event = makeValidEvent();
    event.candidates.truthIndex[0] = -1;
    success &= expectInvalid(
        event, "Truth-candidate matching indices are not reciprocal",
        "non-reciprocal truth-candidate indices");
  }
  {
    auto event = makeValidEvent();
    appendUnmatchedTruth(
        event, TgcL0ValidationUnmatchedReason::CandidateCompetition);
    event.truth.matched[1] = 1U;
    event.truth.matchedCandidateIndex[1] = 0;
    event.truth.matchMeanDeltaR[1] = 0.02F;
    event.truth.unmatchedReason[1] = TgcL0ValidationNoUnmatchedReason;
    success &= expectInvalid(
        event, "Candidate is matched to more than one truth muon",
        "two truths competing for one candidate");
  }
  {
    auto event = makeValidEvent();
    event.truth.wireSegmentMatched[0] = 2U;
    success &= expectInvalid(event,
                             "Truth segment-matched flag is not binary",
                             "non-binary segment matched flag");
  }
  {
    auto event = makeValidEvent();
    event.truth.matchedWireSegmentIndex[0] = 2;
    success &= expectInvalid(
        event, "Truth matched-wire-segment index is out of range",
        "truth-to-wire-segment range");
  }
  {
    auto event = makeValidEvent();
    event.truth.matchedWireSegmentIndex[0] = -1;
    success &= expectInvalid(
        event, "Wire-segment-matched truth has no segment index",
        "missing matched wire segment");
  }
  {
    auto event = makeValidEvent();
    event.truth.matchedWireSegmentIndex[0] = 1;
    success &= expectInvalid(
        event, "Truth wire-segment index points to a strip segment",
        "wire index projection");
  }
  {
    auto event = makeValidEvent();
    event.segments.truthIndex[0] = 1;
    success &= expectInvalid(event, "Segment truth index is out of range",
                             "segment-to-truth range");
  }
  {
    auto event = makeValidEvent();
    event.segments.truthIndex[0] = -1;
    event.segments.truthMatchResidual[0] = TgcL0ValidationInvalidValue;
    success &= expectInvalid(
        event, "Truth-segment matching indices are not reciprocal",
        "non-reciprocal truth-segment indices");
  }
  {
    auto event = makeValidEvent();
    event.candidates.truthIndex[0] = 1;
    success &= expectInvalid(event, "Candidate truth index is out of range",
                             "candidate-to-truth range");
  }

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
