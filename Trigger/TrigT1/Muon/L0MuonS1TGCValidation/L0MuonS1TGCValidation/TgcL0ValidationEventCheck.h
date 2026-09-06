/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCVALIDATION_TGCL0VALIDATIONEVENTCHECK_H
#define L0MUONS1TGCVALIDATION_TGCL0VALIDATIONEVENTCHECK_H

#include "L0MuonS1TGCValidation/TgcL0ValidationEvent.h"

#include <cmath>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace L0Muon {

/** @brief Result of a structural validation-data consistency check. */
struct TgcL0ValidationCheckResult {
  bool valid{true};
  std::string message;
};

/** @brief Check vector sizes and reciprocal cross-block indices. */
inline TgcL0ValidationCheckResult checkTgcL0ValidationEvent(
    const TgcL0ValidationEvent& event) {
  const std::size_t nTruth = event.truth.pdgId.size();
  const std::size_t nSegments = event.segments.subdetectorId.size();
  const std::size_t nCandidates = event.candidates.subdetectorId.size();
  const std::size_t nFinalCandidates =
      event.finalCandidates.subdetectorId.size();
  const std::size_t nSectorLogic = event.sectorLogic.candWord.size();

  const auto invalid = [](std::string message) {
    return TgcL0ValidationCheckResult{false, std::move(message)};
  };
  const auto checkTruthSize = [&](const std::size_t size,
                                  const char* field) {
    return size == nTruth
               ? TgcL0ValidationCheckResult{}
               : invalid(std::string{"Truth block size mismatch for "} +
                         field);
  };
  const auto checkSegmentSize = [&](const std::size_t size,
                                    const char* field) {
    return size == nSegments
               ? TgcL0ValidationCheckResult{}
               : invalid(std::string{"Segment block size mismatch for "} +
                         field);
  };
  const auto checkCandidateSize = [&](const std::size_t size,
                                      const char* field) {
    return size == nCandidates
               ? TgcL0ValidationCheckResult{}
               : invalid(std::string{"Candidate block size mismatch for "} +
                         field);
  };
  const auto checkFinalCandidateSize = [&](const std::size_t size,
                                           const char* field) {
    return size == nFinalCandidates
               ? TgcL0ValidationCheckResult{}
               : invalid(
                     std::string{"Final-candidate block size mismatch for "} +
                     field);
  };
  //coverity[AUTO_CAUSES_COPY:FALSE]
  const auto checkSectorLogicSize = [&](const std::size_t size,
                                        const char* field) {
    return size == nSectorLogic
               ? TgcL0ValidationCheckResult{}
               : invalid(std::string{"Sector Logic block size mismatch for "} +
                         field);
  };

#define TGC_CHECK_TRUTH_SIZE(FIELD)                 \
  do {                                              \
    const auto result = checkTruthSize(             \
        event.truth.FIELD.size(), #FIELD);           \
    if (!result.valid) return result;                \
  } while (false)

  TGC_CHECK_TRUTH_SIZE(barcode);
  TGC_CHECK_TRUTH_SIZE(pt);
  TGC_CHECK_TRUTH_SIZE(eta);
  TGC_CHECK_TRUTH_SIZE(phi);
  TGC_CHECK_TRUTH_SIZE(charge);
  TGC_CHECK_TRUTH_SIZE(extrapolatedStationMask);
  TGC_CHECK_TRUTH_SIZE(m1Eta);
  TGC_CHECK_TRUTH_SIZE(m1Phi);
  TGC_CHECK_TRUTH_SIZE(m2Eta);
  TGC_CHECK_TRUTH_SIZE(m2Phi);
  TGC_CHECK_TRUTH_SIZE(m3Eta);
  TGC_CHECK_TRUTH_SIZE(m3Phi);
  TGC_CHECK_TRUTH_SIZE(matched);
  TGC_CHECK_TRUTH_SIZE(matchedCandidateIndex);
  TGC_CHECK_TRUTH_SIZE(matchMeanDeltaR);
  TGC_CHECK_TRUTH_SIZE(unmatchedReason);
  TGC_CHECK_TRUTH_SIZE(finalCandidateMatched);
  TGC_CHECK_TRUTH_SIZE(matchedFinalCandidateIndex);
  TGC_CHECK_TRUTH_SIZE(finalCandidateMatchDeltaR);
  TGC_CHECK_TRUTH_SIZE(finalCandidateUnmatchedReason);
  TGC_CHECK_TRUTH_SIZE(wireSegmentMatched);
  TGC_CHECK_TRUTH_SIZE(stripSegmentMatched);
  TGC_CHECK_TRUTH_SIZE(matchedWireSegmentIndex);
  TGC_CHECK_TRUTH_SIZE(matchedStripSegmentIndex);
  TGC_CHECK_TRUTH_SIZE(wireSegmentMatchResidual);
  TGC_CHECK_TRUTH_SIZE(stripSegmentMatchResidual);
#undef TGC_CHECK_TRUTH_SIZE

#define TGC_CHECK_SEGMENT_SIZE(FIELD)               \
  do {                                              \
    const auto result = checkSegmentSize(           \
        event.segments.FIELD.size(), #FIELD);        \
    if (!result.valid) return result;                \
  } while (false)

  TGC_CHECK_SEGMENT_SIZE(triggerSector);
  TGC_CHECK_SEGMENT_SIZE(bcTag);
  TGC_CHECK_SEGMENT_SIZE(projection);
  TGC_CHECK_SEGMENT_SIZE(stationMask);
  TGC_CHECK_SEGMENT_SIZE(summedQuality);
  TGC_CHECK_SEGMENT_SIZE(nStations);
  TGC_CHECK_SEGMENT_SIZE(eta);
  TGC_CHECK_SEGMENT_SIZE(phi);
  TGC_CHECK_SEGMENT_SIZE(residual);
  TGC_CHECK_SEGMENT_SIZE(outputResidual);
  TGC_CHECK_SEGMENT_SIZE(consistency);
  TGC_CHECK_SEGMENT_SIZE(pivotChannel);
  TGC_CHECK_SEGMENT_SIZE(truthIndex);
  TGC_CHECK_SEGMENT_SIZE(truthMatchResidual);
#undef TGC_CHECK_SEGMENT_SIZE

#define TGC_CHECK_CANDIDATE_SIZE(FIELD)             \
  do {                                              \
    const auto result = checkCandidateSize(         \
        event.candidates.FIELD.size(), #FIELD);      \
    if (!result.valid) return result;                \
  } while (false)

  TGC_CHECK_CANDIDATE_SIZE(triggerSector);
  TGC_CHECK_CANDIDATE_SIZE(readoutSector);
  TGC_CHECK_CANDIDATE_SIZE(bcTag);
  TGC_CHECK_CANDIDATE_SIZE(stationMask);
  TGC_CHECK_CANDIDATE_SIZE(wireStationMask);
  TGC_CHECK_CANDIDATE_SIZE(stripStationMask);
  TGC_CHECK_CANDIDATE_SIZE(eta);
  TGC_CHECK_CANDIDATE_SIZE(phi);
  TGC_CHECK_CANDIDATE_SIZE(deltaTheta);
  TGC_CHECK_CANDIDATE_SIZE(deltaPhi);
  TGC_CHECK_CANDIDATE_SIZE(pt);
  TGC_CHECK_CANDIDATE_SIZE(threshold);
  TGC_CHECK_CANDIDATE_SIZE(charge);
  TGC_CHECK_CANDIDATE_SIZE(goodMagneticField);
  TGC_CHECK_CANDIDATE_SIZE(truthIndex);
#undef TGC_CHECK_CANDIDATE_SIZE

#define TGC_CHECK_FINAL_CANDIDATE_SIZE(FIELD)              \
  do {                                                     \
    const auto result = checkFinalCandidateSize(           \
        event.finalCandidates.FIELD.size(), #FIELD);       \
    if (!result.valid) return result;                      \
  } while (false)

  TGC_CHECK_FINAL_CANDIDATE_SIZE(sourceCandidateIndex);
  TGC_CHECK_FINAL_CANDIDATE_SIZE(referenceStation);
  TGC_CHECK_FINAL_CANDIDATE_SIZE(triggerSector);
  TGC_CHECK_FINAL_CANDIDATE_SIZE(bcTag);
  TGC_CHECK_FINAL_CANDIDATE_SIZE(tcId);
  TGC_CHECK_FINAL_CANDIDATE_SIZE(rawEta);
  TGC_CHECK_FINAL_CANDIDATE_SIZE(rawPhi);
  TGC_CHECK_FINAL_CANDIDATE_SIZE(eta);
  TGC_CHECK_FINAL_CANDIDATE_SIZE(phi);
  TGC_CHECK_FINAL_CANDIDATE_SIZE(ptCode);
  TGC_CHECK_FINAL_CANDIDATE_SIZE(pt);
  TGC_CHECK_FINAL_CANDIDATE_SIZE(threshold);
  TGC_CHECK_FINAL_CANDIDATE_SIZE(charge);
  TGC_CHECK_FINAL_CANDIDATE_SIZE(innerCoincidence);
  TGC_CHECK_FINAL_CANDIDATE_SIZE(goodMagneticField);
  TGC_CHECK_FINAL_CANDIDATE_SIZE(truthIndex);
  TGC_CHECK_FINAL_CANDIDATE_SIZE(truthMatchDeltaR);
#undef TGC_CHECK_FINAL_CANDIDATE_SIZE

#define TGC_CHECK_SECTOR_LOGIC_SIZE(FIELD)              \
  do {                                                  \
    const auto result = checkSectorLogicSize(           \
        event.sectorLogic.FIELD.size(), #FIELD);        \
    if (!result.valid) return result;                   \
  } while (false)

  TGC_CHECK_SECTOR_LOGIC_SIZE(inputCandidateIndex);
  TGC_CHECK_SECTOR_LOGIC_SIZE(candExtraWord);
  TGC_CHECK_SECTOR_LOGIC_SIZE(boardId);
  TGC_CHECK_SECTOR_LOGIC_SIZE(fiberId);
  TGC_CHECK_SECTOR_LOGIC_SIZE(bcidOffset);
  TGC_CHECK_SECTOR_LOGIC_SIZE(veto);
#undef TGC_CHECK_SECTOR_LOGIC_SIZE

  const auto wireProjection =
      static_cast<std::uint8_t>(TgcL0ValidationProjection::Wire);
  const auto stripProjection =
      static_cast<std::uint8_t>(TgcL0ValidationProjection::Strip);
  const auto invalidStation =
      static_cast<std::uint8_t>(TgcL0ValidationStation::Invalid);

  for (std::size_t segment = 0U; segment < nSegments; ++segment) {
    const std::uint8_t projection = event.segments.projection[segment];
    if (projection != wireProjection && projection != stripProjection) {
      return invalid("Segment projection is not wire or strip");
    }
    if (event.segments.nStations[segment] == 0U ||
        event.segments.nStations[segment] > 3U) {
      return invalid("Segment station count is out of range");
    }
    const int truth = event.segments.truthIndex[segment];
    if (truth >= 0 && static_cast<std::size_t>(truth) >= nTruth) {
      return invalid("Segment truth index is out of range");
    }
    const float residual = event.segments.truthMatchResidual[segment];
    if (truth >= 0 &&
        (!std::isfinite(residual) ||
         residual == TgcL0ValidationInvalidValue)) {
      return invalid("Matched segment has an invalid truth residual");
    }
    if (truth < 0 && residual != TgcL0ValidationInvalidValue) {
      return invalid("Unmatched segment has a truth residual");
    }
  }

  for (std::size_t candidate = 0U; candidate < nCandidates; ++candidate) {
    if (event.candidates.goodMagneticField[candidate] > 1U) {
      return invalid("Candidate GoodMag flag is not binary");
    }
    const int truth = event.candidates.truthIndex[candidate];
    if (truth >= 0 && static_cast<std::size_t>(truth) >= nTruth) {
      return invalid("Candidate truth index is out of range");
    }
  }

  std::vector<bool> sourceCandidateLinked(nCandidates, false);
  for (std::size_t candidate = 0U; candidate < nFinalCandidates;
       ++candidate) {
    if (event.finalCandidates.tcId[candidate] > 6U) {
      return invalid("Final-candidate TCID is out of range");
    }
    if (event.finalCandidates.innerCoincidence[candidate] > 1U ||
        event.finalCandidates.goodMagneticField[candidate] > 1U) {
      return invalid("Final-candidate flag is not binary");
    }
    if (std::abs(event.finalCandidates.charge[candidate]) != 1) {
      return invalid("Final-candidate charge is invalid");
    }
    const int source = event.finalCandidates.sourceCandidateIndex[candidate];
    const std::uint8_t station =
        event.finalCandidates.referenceStation[candidate];
    const int truth = event.finalCandidates.truthIndex[candidate];
    const float residual = event.finalCandidates.truthMatchDeltaR[candidate];
    if (event.finalCandidates.tcId[candidate] == 0U) {
      if (source >= 0 || station != invalidStation || truth >= 0 ||
          residual != TgcL0ValidationInvalidValue) {
        return invalid("Empty final candidate has validation links");
      }
      continue;
    }
    if (static_cast<std::size_t>(source) >= nCandidates) {
      return invalid("Final-candidate source index is out of range");
    }
    const std::size_t sourceIndex = static_cast<std::size_t>(source);
    if (station > static_cast<std::uint8_t>(TgcL0ValidationStation::M3)) {
      return invalid("Final-candidate reference station is invalid");
    }
    if (sourceCandidateLinked[sourceIndex]) {
      return invalid(
          "Source candidate is linked to more than one final candidate");
    }
    sourceCandidateLinked[sourceIndex] = true;
    if (truth >= 0 && static_cast<std::size_t>(truth) >= nTruth) {
      return invalid("Final-candidate truth index is out of range");
    }
    if (truth >= 0 &&
        (!std::isfinite(residual) ||
         residual == TgcL0ValidationInvalidValue)) {
      return invalid("Matched final candidate has an invalid truth residual");
    }
    if (truth < 0 && residual != TgcL0ValidationInvalidValue) {
      return invalid("Unmatched final candidate has a truth residual");
    }
  }

  for (std::size_t candidate = 0U; candidate < nSectorLogic; ++candidate) {
    const std::size_t inputIndex =
        event.sectorLogic.inputCandidateIndex[candidate];
    if (inputIndex >= nFinalCandidates) {
      return invalid("Sector Logic input-candidate index is out of range");
    }
    if (event.finalCandidates.tcId[inputIndex] == 0U) {
      return invalid("Sector Logic input index points to an empty candidate");
    }
    if (candidate != 0U &&
        event.sectorLogic.inputCandidateIndex[candidate] <=
            event.sectorLogic.inputCandidateIndex[candidate - 1U]) {
      return invalid("Sector Logic input-candidate indices are not ordered");
    }
    if (event.sectorLogic.boardId[candidate] != 0U ||
        event.sectorLogic.fiberId[candidate] != 0U ||
        event.sectorLogic.bcidOffset[candidate] != 0 ||
        event.sectorLogic.veto[candidate] != 0U) {
      return invalid("Sector Logic placeholder metadata is not zero");
    }
  }

  std::vector<int> candidateOwner(nCandidates, -1);
  std::vector<int> finalCandidateOwner(nFinalCandidates, -1);
  for (std::size_t truth = 0U; truth < nTruth; ++truth) {
    if (event.truth.matched[truth] > 1U) {
      return invalid("Truth matched flag is not binary");
    }
    const bool matched = event.truth.matched[truth] != 0U;
    const int candidate = event.truth.matchedCandidateIndex[truth];
    if (candidate >= 0 && static_cast<std::size_t>(candidate) >= nCandidates) {
      return invalid("Truth matched-candidate index is out of range");
    }
    if (matched && candidate < 0) {
      return invalid("Matched truth has no candidate index");
    }
    if (!matched && candidate >= 0) {
      return invalid("Unmatched truth has a candidate index");
    }
    if (matched &&
        event.truth.unmatchedReason[truth] !=
            TgcL0ValidationNoUnmatchedReason) {
      return invalid("Matched truth has an unmatched-reason code");
    }
    if (!matched &&
        event.truth.unmatchedReason[truth] ==
            TgcL0ValidationNoUnmatchedReason) {
      return invalid("Unmatched truth has no unmatched-reason code");
    }
    if (!matched &&
        event.truth.unmatchedReason[truth] >
            static_cast<std::uint8_t>(
                TgcL0ValidationUnmatchedReason::CandidateCompetition)) {
      return invalid("Truth unmatched-reason code is out of range");
    }
    if (matched &&
        (!std::isfinite(event.truth.matchMeanDeltaR[truth]) ||
         event.truth.matchMeanDeltaR[truth] == TgcL0ValidationInvalidValue)) {
      return invalid("Matched truth has an invalid candidate residual");
    }
    if (!matched &&
        event.truth.matchMeanDeltaR[truth] != TgcL0ValidationInvalidValue) {
      return invalid("Unmatched truth has a candidate residual");
    }
    if (candidate >= 0) {
      const auto candidateIndex = static_cast<std::size_t>(candidate);
      if (candidateOwner[candidateIndex] >= 0) {
        return invalid("Candidate is matched to more than one truth muon");
      }
      candidateOwner[candidateIndex] = static_cast<int>(truth);
      if (event.candidates.truthIndex[candidateIndex] !=
          static_cast<int>(truth)) {
        return invalid("Truth-candidate matching indices are not reciprocal");
      }
    }

    if (event.truth.finalCandidateMatched[truth] > 1U) {
      return invalid("Truth final-candidate-matched flag is not binary");
    }
    const bool finalMatched =
        event.truth.finalCandidateMatched[truth] != 0U;
    const int finalCandidate =
        event.truth.matchedFinalCandidateIndex[truth];
    if (finalCandidate >= 0 &&
        static_cast<std::size_t>(finalCandidate) >= nFinalCandidates) {
      return invalid("Truth matched-final-candidate index is out of range");
    }
    if (finalMatched && finalCandidate < 0) {
      return invalid("Final-candidate-matched truth has no candidate index");
    }
    if (!finalMatched && finalCandidate >= 0) {
      return invalid("Final-candidate-unmatched truth has a candidate index");
    }
    if (finalMatched &&
        event.truth.finalCandidateUnmatchedReason[truth] !=
            TgcL0ValidationNoUnmatchedReason) {
      return invalid("Final-candidate-matched truth has an unmatched-reason code");
    }
    if (!finalMatched &&
        event.truth.finalCandidateUnmatchedReason[truth] ==
            TgcL0ValidationNoUnmatchedReason) {
      return invalid("Final-candidate-unmatched truth has no unmatched-reason code");
    }
    if (!finalMatched &&
        event.truth.finalCandidateUnmatchedReason[truth] >
            static_cast<std::uint8_t>(
                TgcL0ValidationUnmatchedReason::CandidateCompetition)) {
      return invalid("Truth final-candidate unmatched-reason code is out of range");
    }
    if (finalMatched &&
        (!std::isfinite(event.truth.finalCandidateMatchDeltaR[truth]) ||
         event.truth.finalCandidateMatchDeltaR[truth] ==
             TgcL0ValidationInvalidValue)) {
      return invalid("Final-candidate-matched truth has an invalid residual");
    }
    if (!finalMatched &&
        event.truth.finalCandidateMatchDeltaR[truth] !=
            TgcL0ValidationInvalidValue) {
      return invalid("Final-candidate-unmatched truth has a residual");
    }
    if (finalCandidate >= 0) {
      const auto finalCandidateIndex =
          static_cast<std::size_t>(finalCandidate);
      if (finalCandidateOwner[finalCandidateIndex] >= 0) {
        return invalid("Final candidate is matched to more than one truth muon");
      }
      finalCandidateOwner[finalCandidateIndex] = static_cast<int>(truth);
      if (event.finalCandidates.truthIndex[finalCandidateIndex] !=
          static_cast<int>(truth)) {
        return invalid(
            "Truth-final-candidate matching indices are not reciprocal");
      }
    }

    if (event.truth.wireSegmentMatched[truth] > 1U ||
        event.truth.stripSegmentMatched[truth] > 1U) {
      return invalid("Truth segment-matched flag is not binary");
    }
    const bool wireMatched = event.truth.wireSegmentMatched[truth] != 0U;
    const bool stripMatched = event.truth.stripSegmentMatched[truth] != 0U;
    const int wireSegment = event.truth.matchedWireSegmentIndex[truth];
    const int stripSegment = event.truth.matchedStripSegmentIndex[truth];
    if (wireSegment >= 0 &&
        static_cast<std::size_t>(wireSegment) >= nSegments) {
      return invalid("Truth matched-wire-segment index is out of range");
    }
    if (stripSegment >= 0 &&
        static_cast<std::size_t>(stripSegment) >= nSegments) {
      return invalid("Truth matched-strip-segment index is out of range");
    }
    if (wireMatched && wireSegment < 0) {
      return invalid("Wire-segment-matched truth has no segment index");
    }
    if (stripMatched && stripSegment < 0) {
      return invalid("Strip-segment-matched truth has no segment index");
    }
    if (!wireMatched && wireSegment >= 0) {
      return invalid("Wire-segment-unmatched truth has a segment index");
    }
    if (!stripMatched && stripSegment >= 0) {
      return invalid("Strip-segment-unmatched truth has a segment index");
    }
    if (wireMatched &&
        (!std::isfinite(event.truth.wireSegmentMatchResidual[truth]) ||
         event.truth.wireSegmentMatchResidual[truth] ==
             TgcL0ValidationInvalidValue)) {
      return invalid("Wire-segment-matched truth has an invalid residual");
    }
    if (!wireMatched &&
        event.truth.wireSegmentMatchResidual[truth] !=
            TgcL0ValidationInvalidValue) {
      return invalid("Wire-segment-unmatched truth has a residual");
    }
    if (stripMatched &&
        (!std::isfinite(event.truth.stripSegmentMatchResidual[truth]) ||
         event.truth.stripSegmentMatchResidual[truth] ==
             TgcL0ValidationInvalidValue)) {
      return invalid("Strip-segment-matched truth has an invalid residual");
    }
    if (!stripMatched &&
        event.truth.stripSegmentMatchResidual[truth] !=
            TgcL0ValidationInvalidValue) {
      return invalid("Strip-segment-unmatched truth has a residual");
    }
    if (wireSegment >= 0 &&
        event.segments.projection[wireSegment] != wireProjection) {
      return invalid("Truth wire-segment index points to a strip segment");
    }
    if (stripSegment >= 0 &&
        event.segments.projection[stripSegment] != stripProjection) {
      return invalid("Truth strip-segment index points to a wire segment");
    }
    if ((wireSegment >= 0 &&
         event.segments.truthIndex[wireSegment] != static_cast<int>(truth)) ||
        (stripSegment >= 0 &&
         event.segments.truthIndex[stripSegment] != static_cast<int>(truth))) {
      return invalid("Truth-segment matching indices are not reciprocal");
    }
  }

  for (std::size_t candidate = 0U; candidate < nCandidates; ++candidate) {
    const int truth = event.candidates.truthIndex[candidate];
    if (truth < 0) continue;
    if (event.truth.matched[truth] == 0U ||
        event.truth.matchedCandidateIndex[truth] !=
            static_cast<int>(candidate)) {
      return invalid("Truth-candidate matching indices are not reciprocal");
    }
  }

  for (std::size_t candidate = 0U; candidate < nFinalCandidates;
       ++candidate) {
    const int truth = event.finalCandidates.truthIndex[candidate];
    if (truth < 0) continue;
    if (event.truth.finalCandidateMatched[truth] == 0U ||
        event.truth.matchedFinalCandidateIndex[truth] !=
            static_cast<int>(candidate)) {
      return invalid(
          "Truth-final-candidate matching indices are not reciprocal");
    }
  }

  for (std::size_t segment = 0U; segment < nSegments; ++segment) {
    const int truth = event.segments.truthIndex[segment];
    if (truth < 0) continue;
    const bool isWire = event.segments.projection[segment] == wireProjection;
    const int reverseIndex =
        isWire ? event.truth.matchedWireSegmentIndex[truth]
               : event.truth.matchedStripSegmentIndex[truth];
    if (reverseIndex != static_cast<int>(segment)) {
      return invalid("Truth-segment matching indices are not reciprocal");
    }
  }

  return {};
}

}  // namespace L0Muon

#endif
