/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FloatingTrackSelector.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "xAODTrigL1Muon/L1TGCCandData.h"

namespace {

using CandidateGroup = std::pair<std::uint16_t, std::uint16_t>;

CandidateGroup candidateGroup(const L1Muon::TgcSLCandidate &candidate) {
  return {candidate.subdetectorId, candidate.sectorId};
}

float selectionPt(const L1Muon::TgcSLCandidate &candidate) {
  return std::isfinite(candidate.pt) && candidate.pt > 0.F ? candidate.pt : 0.F;
}

void fillOutputCandidate(const L1Muon::TgcSLCandidate &input, std::uint8_t tcId,
                         xAOD::L1TGCCandData &output) {
  output.initialize(input.subdetectorId, input.sectorId, input.bcTag);
  output.setL1Eta(input.eta);
  output.setL1Phi(input.phi);
  output.setL1Pt(selectionPt(input));
  output.setL1Threshold(input.threshold);
  output.setL1CandCharge(input.charge > 0 ? 1U : 0U);
  output.setCoinType(0U);
  output.setHasInnerCoincidence(input.hasInnerCoincidence);
  output.setGoodMagneticField(input.goodMagneticField);
  output.setDeltaPhi(input.deltaPhi);
  output.setDeltaTheta(input.deltaTheta);
  output.setL1NswSegment(input.nswSegment);
  output.setTcId(tcId);
}

}  // namespace

namespace L1Muon::L1TGC {

void FloatingTrackSelector::select(const TgcSLCandidateContainer &candidates,
                           xAOD::L1TGCCandDataContainer &output) const {
  std::vector<const TgcSLCandidate *> orderedCandidates;
  orderedCandidates.reserve(candidates.size());
  for (const TgcSLCandidate &candidate : candidates) {
    if (candidate.threshold == 0U)
      continue;
    orderedCandidates.emplace_back(&candidate);
  }

  std::stable_sort(orderedCandidates.begin(), orderedCandidates.end(),
                   [](const TgcSLCandidate *left, const TgcSLCandidate *right) {
                     const CandidateGroup leftGroup = candidateGroup(*left);
                     const CandidateGroup rightGroup = candidateGroup(*right);
                     if (leftGroup != rightGroup)
                       return leftGroup < rightGroup;

                     if (left->threshold != right->threshold) {
                       return left->threshold > right->threshold;
                     }
                     const float leftPt = selectionPt(*left);
                     const float rightPt = selectionPt(*right);
                     if (leftPt != rightPt)
                       return leftPt > rightPt;
                     return left->selectorPriority > right->selectorPriority;
                   });

  CandidateGroup currentGroup{};
  bool haveCurrentGroup{false};
  std::size_t groupCandidate{0U};
  for (const TgcSLCandidate *candidate : orderedCandidates) {
    const CandidateGroup group = candidateGroup(*candidate);
    if (!haveCurrentGroup || group != currentGroup) {
      currentGroup = group;
      haveCurrentGroup = true;
      groupCandidate = 0U;
    }
    if (groupCandidate >= s_maxCandidatesPerSector)
      continue;

    ++groupCandidate;
    xAOD::L1TGCCandData *outputCandidate =
        output.push_back(std::make_unique<xAOD::L1TGCCandData>());
    fillOutputCandidate(*candidate, static_cast<std::uint8_t>(groupCandidate),
                        *outputCandidate);
  }
}

}  // namespace L1Muon::L1TGC
