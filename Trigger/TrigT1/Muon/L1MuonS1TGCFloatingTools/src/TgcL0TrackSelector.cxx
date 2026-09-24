/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0TrackSelector.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "xAODL0MuonCand/TGCCandData.h"

namespace {

using CandidateGroup = std::pair<std::uint16_t, std::uint16_t>;

CandidateGroup candidateGroup(const L0Muon::TgcL0Candidate &candidate) {
  return {candidate.subdetectorId, candidate.sectorId};
}

float selectionPt(const L0Muon::TgcL0Candidate &candidate) {
  return std::isfinite(candidate.pt) && candidate.pt > 0.F ? candidate.pt : 0.F;
}

void fillOutputCandidate(const L0Muon::TgcL0Candidate &input, std::uint8_t tcId,
                         xAOD::TGCCandData &output) {
  output.initialize(input.subdetectorId, input.sectorId, input.bcTag);
  output.setEta(input.eta);
  output.setPhi(input.phi);
  output.setPt(selectionPt(input));
  output.setThreshold(input.threshold);
  output.setCandCharge(input.charge > 0 ? 1U : 0U);
  output.setMdtFlag(0U);
  output.setCandQuality(xAOD::ICandData_v1::Quality::Q_UNDEFINED);
  output.setCoinType(0U);
  output.setHasInnerCoincidence(input.hasInnerCoincidence);
  output.setGoodMagneticField(input.goodMagneticField);
  output.setDeltaPhi(input.deltaPhi);
  output.setDeltaTheta(input.deltaTheta);
  output.setNswSegment(input.nswSegment);
  output.setTcId(tcId);
}

}  // namespace

namespace L0Muon {
namespace TgcL0Floating {

void TrackSelector::select(const TgcL0CandidateContainer &candidates,
                           xAOD::TGCCandDataContainer &output) const {
  std::vector<const TgcL0Candidate *> orderedCandidates;
  orderedCandidates.reserve(candidates.size());
  for (const TgcL0Candidate &candidate : candidates) {
    if (candidate.threshold == 0U)
      continue;
    orderedCandidates.emplace_back(&candidate);
  }

  std::stable_sort(orderedCandidates.begin(), orderedCandidates.end(),
                   [](const TgcL0Candidate *left, const TgcL0Candidate *right) {
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
  for (const TgcL0Candidate *candidate : orderedCandidates) {
    const CandidateGroup group = candidateGroup(*candidate);
    if (!haveCurrentGroup || group != currentGroup) {
      currentGroup = group;
      haveCurrentGroup = true;
      groupCandidate = 0U;
    }
    if (groupCandidate >= s_maxCandidatesPerSector)
      continue;

    ++groupCandidate;
    xAOD::TGCCandData *outputCandidate =
        output.push_back(std::make_unique<xAOD::TGCCandData>());
    fillOutputCandidate(*candidate, static_cast<std::uint8_t>(groupCandidate),
                        *outputCandidate);
  }
}

}  // namespace TgcL0Floating
}  // namespace L0Muon
