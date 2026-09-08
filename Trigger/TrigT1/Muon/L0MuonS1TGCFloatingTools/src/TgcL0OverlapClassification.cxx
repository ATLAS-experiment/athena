/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TgcL0OverlapClassification.h"

#include "FourMomUtils/xAODP4Helpers.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <vector>

namespace {

bool sameChamber(const L0Muon::TgcL0Candidate& first,
                 const L0Muon::TgcL0Candidate& second,
                 std::uint8_t stationBit) {
  if (stationBit == 0x1U) {
    return first.m1StationEta == second.m1StationEta &&
           first.m1StationPhi == second.m1StationPhi;
  }
  if (stationBit == 0x2U) {
    return first.m2StationEta == second.m2StationEta &&
           first.m2StationPhi == second.m2StationPhi;
  }
  return first.m3StationEta == second.m3StationEta &&
         first.m3StationPhi == second.m3StationPhi;
}

std::size_t countBits(std::uint8_t value) {
  std::size_t count{0U};
  while (value != 0U) {
    count += value & 0x1U;
    value >>= 1U;
  }
  return count;
}

bool isOverlapPair(
    const L0Muon::TgcL0Candidate& first,
    const L0Muon::TgcL0Candidate& second,
    const L0Muon::TgcL0Floating::OverlapClassificationConfig& config) {
  if (first.bcTag != second.bcTag ||
      first.subdetectorId != second.subdetectorId) {
    return false;
  }
  if (std::abs(first.eta - second.eta) >= config.maxDeltaEta ||
      std::abs(xAOD::P4Helpers::deltaPhi(first.phi, second.phi)) >= config.maxDeltaPhi) {
    return false;
  }

  const std::uint8_t commonMask = static_cast<std::uint8_t>(
      first.positionStationMask & second.positionStationMask);
  if (countBits(commonMask) < config.minCommonStations) return false;

  bool differentChamber{false};
  for (const std::uint8_t stationBit : {0x1U, 0x2U, 0x4U}) {
    if ((commonMask & stationBit) != 0U &&
        !sameChamber(first, second, stationBit)) {
      differentChamber = true;
      break;
    }
  }
  return differentChamber;
}

class DisjointSet {
 public:
  explicit DisjointSet(std::size_t size) : m_parent(size), m_size(size, 1U) {
    std::iota(m_parent.begin(), m_parent.end(), 0U);
  }

  std::size_t find(std::size_t value) {
    if (m_parent[value] != value) m_parent[value] = find(m_parent[value]);
    return m_parent[value];
  }

  void join(std::size_t first, std::size_t second) {
    first = find(first);
    second = find(second);
    if (first == second) return;
    if (m_size[first] < m_size[second]) std::swap(first, second);
    m_parent[second] = first;
    m_size[first] += m_size[second];
  }

 private:
  std::vector<std::size_t> m_parent;
  std::vector<std::size_t> m_size;
};

}  // namespace

namespace L0Muon {
namespace TgcL0Floating {

OverlapClassification::OverlapClassification(OverlapClassificationConfig config)
    : m_config(config) {}

void OverlapClassification::classify(
    TgcL0CandidateContainer& candidates,
    OverlapClassificationStatistics& statistics) const {
  statistics = OverlapClassificationStatistics{};
  for (TgcL0Candidate& candidate : candidates) {
    candidate.overlapGroupId = 0U;
    candidate.overlapMultiplicity = 1U;
    candidate.inChamberOverlap = false;
  }
  if (candidates.size() < 2U) return;

  DisjointSet groups(candidates.size());
  for (std::size_t first = 0U; first < candidates.size(); ++first) {
    for (std::size_t second = first + 1U; second < candidates.size(); ++second) {
      if (isOverlapPair(candidates[first], candidates[second], m_config)) {
        groups.join(first, second);
      }
    }
  }

  std::vector<std::size_t> roots(candidates.size());
  std::vector<std::size_t> multiplicities(candidates.size(), 0U);
  for (std::size_t index = 0U; index < candidates.size(); ++index) {
    roots[index] = groups.find(index);
    ++multiplicities[roots[index]];
  }

  std::vector<std::uint16_t> groupIds(candidates.size(), 0U);
  std::uint16_t nextGroupId{1U};
  for (std::size_t index = 0U; index < candidates.size(); ++index) {
    const std::size_t root = roots[index];
    if (multiplicities[root] < 2U) continue;
    if (groupIds[root] == 0U) {
      groupIds[root] = nextGroupId++;
      ++statistics.nGroups;
      statistics.maxMultiplicity =
          std::max(statistics.maxMultiplicity, multiplicities[root]);
    }
    TgcL0Candidate& candidate = candidates[index];
    candidate.overlapGroupId = groupIds[root];
    candidate.overlapMultiplicity = static_cast<std::uint8_t>(
        std::min<std::size_t>(multiplicities[root], 0xffU));
    candidate.inChamberOverlap = true;
    ++statistics.nCandidates;
  }
}

}  // namespace TgcL0Floating
}  // namespace L0Muon
