/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0MdtCandidateSelector.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "AthContainers/OwnershipPolicy.h"
#include "xAODL0MuonCand/TGCCandData.h"

namespace {

using CandidateGroup = std::pair<std::uint16_t, std::uint16_t>;

CandidateGroup candidateGroup(const xAOD::TGCCandData &candidate) {
  return {candidate.subdetectorId(), candidate.sectorId()};
}

}  // namespace

namespace L1Muon {

std::unique_ptr<xAOD::TGCCandDataContainer> TgcL0MdtCandidateSelector::select(
    xAOD::TGCCandDataContainer &candidates) const {
  auto output = std::make_unique<xAOD::TGCCandDataContainer>(SG::VIEW_ELEMENTS);
  std::vector<xAOD::TGCCandData *> orderedCandidates;
  orderedCandidates.reserve(candidates.size());
  for (xAOD::TGCCandData *candidate : candidates) {
    if (candidate->tcId() == 0U)
      continue;
    orderedCandidates.emplace_back(candidate);
  }

  std::stable_sort(
      orderedCandidates.begin(), orderedCandidates.end(),
      [](const xAOD::TGCCandData *left, const xAOD::TGCCandData *right) {
        const CandidateGroup leftGroup = candidateGroup(*left);
        const CandidateGroup rightGroup = candidateGroup(*right);
        if (leftGroup != rightGroup)
          return leftGroup < rightGroup;
        return left->pt() > right->pt();
      });

  CandidateGroup currentGroup{};
  bool haveCurrentGroup{false};
  std::size_t groupCandidate{0U};
  for (xAOD::TGCCandData *candidate : orderedCandidates) {
    const CandidateGroup group = candidateGroup(*candidate);
    if (!haveCurrentGroup || group != currentGroup) {
      currentGroup = group;
      haveCurrentGroup = true;
      groupCandidate = 0U;
    }
    if (groupCandidate >= s_maxCandidatesPerSector)
      continue;

    ++groupCandidate;
    output->push_back(candidate);
  }
  return output;
}

}  // namespace L1Muon
