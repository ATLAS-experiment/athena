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
#include "xAODTrigL1Muon/L1TGCCandData.h"

namespace {

using CandidateGroup = std::pair<std::uint16_t, std::uint16_t>;

CandidateGroup candidateGroup(const xAOD::L1TGCCandData &candidate) {
  return {candidate.l1SubdetectorId(), candidate.l1SectorId()};
}

}  // namespace

namespace L1Muon {

std::unique_ptr<xAOD::L1TGCCandDataContainer> TgcL0MdtCandidateSelector::select(
    xAOD::L1TGCCandDataContainer &candidates) const {
  auto output = std::make_unique<xAOD::L1TGCCandDataContainer>(SG::VIEW_ELEMENTS);
  std::vector<xAOD::L1TGCCandData *> orderedCandidates;
  orderedCandidates.reserve(candidates.size());
  for (xAOD::L1TGCCandData *candidate : candidates) {
    if (candidate->tcId() == 0U)
      continue;
    orderedCandidates.emplace_back(candidate);
  }

  std::stable_sort(
      orderedCandidates.begin(), orderedCandidates.end(),
      [](const xAOD::L1TGCCandData *left, const xAOD::L1TGCCandData *right) {
        const CandidateGroup leftGroup = candidateGroup(*left);
        const CandidateGroup rightGroup = candidateGroup(*right);
        if (leftGroup != rightGroup)
          return leftGroup < rightGroup;
        return left->l1Pt() > right->l1Pt();
      });

  CandidateGroup currentGroup{};
  bool haveCurrentGroup{false};
  std::size_t groupCandidate{0U};
  for (xAOD::L1TGCCandData *candidate : orderedCandidates) {
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
