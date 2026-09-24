/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0SegmentReconstruction.h"

#include "FourMomUtils/xAODP4Helpers.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <map>
#include <numbers>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

namespace {

using Coincidence = L0Muon::TgcL0Floating::StationCoincidence;
using Config = L0Muon::TgcL0Floating::SegmentReconstructionConfig;
using Statistics = L0Muon::TgcL0Floating::SegmentStatistics;
using Station = L0Muon::TgcL0Floating::Station;
using GroupKey = L0Muon::TgcL0Floating::HitGroupKey;
using OutputSegment = L0Muon::TgcL0Segment;
using OutputSegments = L0Muon::TgcL0SegmentContainer;
using Projection = L0Muon::TgcL0SegmentProjection;

constexpr std::uint8_t stationBit(const Station station) {
  if (station == Station::M1) return 0x1U;
  if (station == Station::M2) return 0x2U;
  if (station == Station::M3) return 0x4U;
  return 0U;
}

constexpr std::size_t stationIndex(const Station station) {
  if (station == Station::M1) return 0U;
  if (station == Station::M2) return 1U;
  return 2U;
}

float measuringCoordinate(const Coincidence& point) {
  return point.isStrip ? point.phi : point.eta;
}

float coordinateDifference(const bool isStrip, const float lhs,
                           const float rhs) {
  if (isStrip) {
    return static_cast<float>(xAOD::P4Helpers::deltaPhi(lhs, rhs));
  }
  return lhs - rhs;
}

float physicalDeltaTheta(const Coincidence& inner,
                         const Coincidence& outer) {
  const float deltaR = outer.r - inner.r;
  const float deltaZ = outer.z - inner.z;
  if (deltaR == 0.F && deltaZ == 0.F) return 0.F;
  const float segmentTheta = std::atan2(deltaR, deltaZ);
  const float pivotTheta = std::atan2(outer.r, outer.z);
  // Use the shortest signed angular separation across the atan2 branch cut.
  return static_cast<float>(
      xAOD::P4Helpers::deltaPhi(segmentTheta, pivotTheta));
}

struct ProjectionSegment {
  std::array<const Coincidence*, 3> points{};
  bool isStrip{false};
  std::uint8_t stationMask{0};
  std::uint8_t summedQuality{0};
  std::uint8_t nStations{0};
  float eta{0.F};
  float phi{0.F};
  /// Validated Floating coordinate residual used for matching/ranking.
  float residual{0.F};
  /// Physical candidate payload residual.
  float outputResidual{0.F};
  float consistency{0.F};
  std::uint16_t pivotChannel{0};
};


void appendValidationSegments(const GroupKey& groupKey,
                              const std::vector<ProjectionSegment>& segments,
                              OutputSegments& output) {
  output.reserve(output.size() + segments.size());
  for (const ProjectionSegment& segment : segments) {
    OutputSegment value;
    value.subdetectorId = groupKey.subDetectorId;
    value.triggerSector = groupKey.triggerSector;
    value.bcTag = groupKey.bcTag;
    value.projection = segment.isStrip ? Projection::Strip : Projection::Wire;
    value.stationMask = segment.stationMask;
    value.summedQuality = segment.summedQuality;
    value.nStations = segment.nStations;
    value.eta = segment.eta;
    value.phi = segment.phi;
    value.residual = segment.residual;
    value.outputResidual = segment.outputResidual;
    value.consistency = segment.consistency;
    value.pivotChannel = segment.pivotChannel;
    output.emplace_back(value);
  }
}

const Coincidence* projectionPivot(const ProjectionSegment& segment) {
  if (segment.points[2] != nullptr) return segment.points[2];
  if (segment.points[1] != nullptr) return segment.points[1];
  return segment.points[0];
}

ProjectionSegment makeSegment(const Coincidence* m1, const Coincidence* m2,
                              const Coincidence* m3, const bool isStrip) {
  ProjectionSegment segment;
  segment.points = {m1, m2, m3};
  segment.isStrip = isStrip;
  for (const Coincidence* point : segment.points) {
    if (point == nullptr) continue;
    segment.stationMask |= stationBit(point->station);
    segment.summedQuality = static_cast<std::uint8_t>(
        segment.summedQuality + point->observedLayers);
    ++segment.nStations;
  }

  const Coincidence* pivot = projectionPivot(segment);
  if (pivot != nullptr) {
    segment.eta = pivot->eta;
    segment.phi = pivot->phi;
    segment.pivotChannel = pivot->channel;
  }

  std::vector<const Coincidence*> points;
  for (const Coincidence* point : segment.points) {
    if (point != nullptr) points.emplace_back(point);
  }
  if (points.size() >= 2U) {
    const float first = measuringCoordinate(*points.front());
    const float last = measuringCoordinate(*points.back());
    segment.residual = coordinateDifference(isStrip, first, last);
    segment.outputResidual = isStrip
                                 ? segment.residual
                                 : physicalDeltaTheta(*points.front(),
                                                      *points.back());
    segment.consistency = std::abs(segment.residual);
  }
  if (m1 != nullptr && m2 != nullptr && m3 != nullptr) {
    const float c1 = measuringCoordinate(*m1);
    const float c2 = measuringCoordinate(*m2);
    const float c3 = measuringCoordinate(*m3);
    const float predictedM1 = isStrip
                                  ? c2 - static_cast<float>(
                                             xAOD::P4Helpers::deltaPhi(c3, c2))
                                  : c2 - (c3 - c2);
    segment.consistency = std::abs(
        coordinateDifference(isStrip, c1, predictedM1));
    segment.residual = coordinateDifference(isStrip, c1, c3);
    segment.outputResidual = isStrip
                                 ? segment.residual
                                 : physicalDeltaTheta(*m1, *m3);
  }

  if (!isStrip) {
    float sinPhi = 0.F;
    float cosPhi = 0.F;
    for (const Coincidence* point : points) {
      sinPhi += std::sin(point->phi);
      cosPhi += std::cos(point->phi);
    }
    segment.phi = std::atan2(sinPhi, cosPhi);
  } else if (!points.empty()) {
    float etaSum = 0.F;
    for (const Coincidence* point : points) etaSum += point->eta;
    segment.eta = etaSum / static_cast<float>(points.size());
  }
  return segment;
}

int coordinateKey(const Coincidence* point, const bool isStrip) {
  if (point == nullptr) return 0;
  const float coordinate = isStrip ? point->phi : point->eta;
  return static_cast<int>(std::lround(100000.F * coordinate));
}

using SegmentKey = std::tuple<std::uint8_t, bool, int, int, int>;

SegmentKey segmentKey(const ProjectionSegment& segment) {
  return {segment.stationMask, segment.isStrip,
          coordinateKey(segment.points[0], segment.isStrip),
          coordinateKey(segment.points[1], segment.isStrip),
          coordinateKey(segment.points[2], segment.isStrip)};
}

bool betterSegment(const ProjectionSegment& lhs,
                   const ProjectionSegment& rhs) {
  if (lhs.nStations != rhs.nStations) return lhs.nStations > rhs.nStations;
  if (lhs.summedQuality != rhs.summedQuality) {
    return lhs.summedQuality > rhs.summedQuality;
  }
  const bool lhsHasM1 = (lhs.stationMask & 0x1U) != 0U;
  const bool rhsHasM1 = (rhs.stationMask & 0x1U) != 0U;
  if (lhsHasM1 != rhsHasM1) return lhsHasM1;
  const bool lhsHasM3 = (lhs.stationMask & 0x4U) != 0U;
  const bool rhsHasM3 = (rhs.stationMask & 0x4U) != 0U;
  if (lhsHasM3 != rhsHasM3) return lhsHasM3;
  if (lhs.consistency != rhs.consistency) {
    return lhs.consistency < rhs.consistency;
  }
  return segmentKey(lhs) < segmentKey(rhs);
}

void addSegment(std::vector<ProjectionSegment>& segments,
                std::set<SegmentKey>& keys, const Coincidence* m1,
                const Coincidence* m2, const Coincidence* m3,
                const bool isStrip, Statistics& statistics) {
  const ProjectionSegment segment = makeSegment(m1, m2, m3, isStrip);
  if (!keys.insert(segmentKey(segment)).second) {
    ++statistics.nDuplicateProjectionSegments;
    return;
  }
  segments.emplace_back(segment);
}

void retainProjectionWorkingSet(std::vector<ProjectionSegment>& segments,
                                const std::size_t maximum,
                                Statistics& statistics) {
  std::stable_sort(segments.begin(), segments.end(), betterSegment);
  if (maximum == 0U || segments.size() <= maximum) return;

  std::array<bool, 8> retainedMask{};
  std::vector<ProjectionSegment> retained;
  retained.reserve(maximum);
  for (const ProjectionSegment& segment : segments) {
    if (segment.stationMask >= retainedMask.size() ||
        retainedMask[segment.stationMask]) {
      continue;
    }
    retained.emplace_back(segment);
    retainedMask[segment.stationMask] = true;
    if (retained.size() == maximum) break;
  }
  for (const ProjectionSegment& segment : segments) {
    if (retained.size() == maximum) break;
    const bool alreadyRetained = std::any_of(
        retained.begin(), retained.end(),
        [&segment](const ProjectionSegment& selected) {
          return segmentKey(selected) == segmentKey(segment);
        });
    if (!alreadyRetained) retained.emplace_back(segment);
  }
  statistics.nLimitedProjectionSegments += segments.size() - retained.size();
  segments = std::move(retained);
  std::stable_sort(segments.begin(), segments.end(), betterSegment);
}

std::vector<ProjectionSegment> buildProjectionSegments(
    const std::array<std::vector<const Coincidence*>, 3>& points,
    const bool isStrip, const Config& config, Statistics& statistics) {
  std::vector<ProjectionSegment> segments;
  std::set<SegmentKey> keys;

  for (const Coincidence* m1 : points[0]) {
    for (const Coincidence* m2 : points[1]) {
      for (const Coincidence* m3 : points[2]) {
        addSegment(segments, keys, m1, m2, m3, isStrip, statistics);
      }
    }
  }
  for (const Coincidence* m1 : points[0]) {
    for (const Coincidence* m3 : points[2]) {
      addSegment(segments, keys, m1, nullptr, m3, isStrip, statistics);
    }
    for (const Coincidence* m2 : points[1]) {
      addSegment(segments, keys, m1, m2, nullptr, isStrip, statistics);
    }
  }
  for (const Coincidence* m2 : points[1]) {
    for (const Coincidence* m3 : points[2]) {
      addSegment(segments, keys, nullptr, m2, m3, isStrip, statistics);
    }
  }

  retainProjectionWorkingSet(segments,
                             config.maxSegmentCombinationsPerGroup,
                             statistics);
  return segments;
}

std::uint8_t quality(const ProjectionSegment& segment, const Station station) {
  const Coincidence* point = segment.points[stationIndex(station)];
  return point == nullptr ? 0U : point->observedLayers;
}

struct PositionPair {
  const Coincidence* wire{nullptr};
  const Coincidence* strip{nullptr};
  std::size_t stationIndex{0U};
  float deltaEta{0.F};
  float deltaPhi{0.F};
  float coordinateResidual{0.F};

  explicit operator bool() const { return wire != nullptr && strip != nullptr; }
};

PositionPair selectPositionPair(const ProjectionSegment& wire,
                                const ProjectionSegment& strip,
                                const Config& config) {
  // Validated Floating convention: common pivot M3 -> M2 -> M1, eta measured
  // by wire and phi measured by strip. Channel IDs are provenance only.
  for (std::size_t index = 3U; index > 0U; --index) {
    const std::size_t station = index - 1U;
    const Coincidence* wirePoint = wire.points[station];
    const Coincidence* stripPoint = strip.points[station];
    if (wirePoint == nullptr || stripPoint == nullptr) continue;

    const float deltaEta = std::abs(wirePoint->eta - stripPoint->eta);
    const float deltaPhi = std::abs(static_cast<float>(
        xAOD::P4Helpers::deltaPhi(wirePoint->phi, stripPoint->phi)));
    if (config.maxPivotWireStripDeltaEta >= 0.F &&
        deltaEta > config.maxPivotWireStripDeltaEta) {
      continue;
    }
    if (config.maxPivotWireStripDeltaPhi >= 0.F &&
        deltaPhi > config.maxPivotWireStripDeltaPhi) {
      continue;
    }
    return {wirePoint, stripPoint, station, deltaEta, deltaPhi,
            std::hypot(deltaEta, deltaPhi)};
  }
  return {};
}

struct CandidatePair {
  const ProjectionSegment* wire{nullptr};
  const ProjectionSegment* strip{nullptr};
  PositionPair position{};
  std::uint8_t positionStationMask{0};
  std::uint8_t combinedStationMask{0};
};

int bendScoreMilli(const CandidatePair& pair) {
  const float bend = std::sqrt(
      pair.wire->residual * pair.wire->residual +
      0.25F * pair.strip->residual * pair.strip->residual);
  return static_cast<int>(std::lround(1000.F * bend));
}

bool betterCandidatePair(const CandidatePair& lhs, const CandidatePair& rhs) {
  const std::uint16_t lhsQuality = static_cast<std::uint16_t>(
      lhs.wire->summedQuality + lhs.strip->summedQuality);
  const std::uint16_t rhsQuality = static_cast<std::uint16_t>(
      rhs.wire->summedQuality + rhs.strip->summedQuality);
  if (lhsQuality != rhsQuality) return lhsQuality > rhsQuality;
  const unsigned int lhsStations =
      std::popcount(static_cast<unsigned int>(lhs.combinedStationMask));
  const unsigned int rhsStations =
      std::popcount(static_cast<unsigned int>(rhs.combinedStationMask));
  if (lhsStations != rhsStations) return lhsStations > rhsStations;
  if (lhs.position.stationIndex != rhs.position.stationIndex) {
    return lhs.position.stationIndex > rhs.position.stationIndex;
  }
  const int lhsBend = bendScoreMilli(lhs);
  const int rhsBend = bendScoreMilli(rhs);
  if (lhsBend != rhsBend) return lhsBend < rhsBend;
  if (lhs.position.coordinateResidual != rhs.position.coordinateResidual) {
    return lhs.position.coordinateResidual < rhs.position.coordinateResidual;
  }
  if (lhs.position.wire->eta != rhs.position.wire->eta) {
    return lhs.position.wire->eta < rhs.position.wire->eta;
  }
  return lhs.position.strip->phi < rhs.position.strip->phi;
}

using CandidateKey = std::tuple<
    std::uint16_t, std::uint16_t, std::uint16_t, std::uint16_t,
    std::int16_t, std::uint16_t, std::uint16_t, std::int16_t,
    std::uint16_t, std::uint8_t, std::uint8_t, std::uint8_t, int, int>;

CandidateKey candidateKey(const GroupKey& group, const CandidatePair& pair) {
  return {group.subDetectorId,
          group.triggerSector,
          group.bcTag,
          pair.position.wire->detectorSector,
          pair.position.wire->stationEta,
          pair.position.wire->stationPhi,
          pair.position.strip->detectorSector,
          pair.position.strip->stationEta,
          pair.position.strip->stationPhi,
          pair.wire->stationMask,
          pair.strip->stationMask,
          pair.positionStationMask,
          static_cast<int>(std::lround(100000.F * pair.position.wire->eta)),
          static_cast<int>(std::lround(100000.F * pair.position.strip->phi))};
}

void pruneLocalCandidateBins(std::vector<CandidatePair>& pairs,
                             const Config& config,
                             Statistics& statistics) {
  if (config.maxCandidatesPerLocalBin == 0U ||
      config.localCandidateEtaBinWidth <= 0.F ||
      config.localCandidatePhiBinWidth <= 0.F || pairs.empty()) {
    return;
  }
  std::stable_sort(pairs.begin(), pairs.end(), betterCandidatePair);
  std::map<std::tuple<int, int, std::size_t>, std::size_t> keptPerBin;
  std::vector<CandidatePair> retained;
  retained.reserve(pairs.size());
  for (const CandidatePair& pair : pairs) {
    const float phi = static_cast<float>(
        xAOD::P4Helpers::deltaPhi(pair.position.strip->phi, 0.));
    const int etaBin = static_cast<int>(std::floor(
        pair.position.wire->eta / config.localCandidateEtaBinWidth));
    const int phiBin = static_cast<int>(std::floor(
        (phi + std::numbers::pi_v<float>) /
        config.localCandidatePhiBinWidth));
    const auto key =
        std::make_tuple(etaBin, phiBin, pair.position.stationIndex);
    std::size_t& count = keptPerBin[key];
    if (count >= config.maxCandidatesPerLocalBin) {
      ++statistics.nLimitedCandidates;
      continue;
    }
    retained.emplace_back(pair);
    ++count;
  }
  pairs = std::move(retained);
}

bool sameChamber(const Coincidence& lhs, const Coincidence& rhs) {
  return lhs.station == rhs.station && lhs.stationEta == rhs.stationEta &&
         lhs.stationPhi == rhs.stationPhi;
}

bool sameCandidateChamberPath(const CandidatePair& lhs,
                              const CandidatePair& rhs) {
  unsigned int commonStations = 0U;
  for (std::size_t station = 0U; station < lhs.wire->points.size(); ++station) {
    const Coincidence* lhsPoint = lhs.wire->points[station];
    const Coincidence* rhsPoint = rhs.wire->points[station];
    if (lhsPoint == nullptr || rhsPoint == nullptr) continue;
    if (!sameChamber(*lhsPoint, *rhsPoint)) return false;
    ++commonStations;
  }
  return commonStations >= 2U;
}

bool sameCoarseLocalPosition(const CandidatePair& lhs,
                             const CandidatePair& rhs,
                             const Config& config) {
  if (!sameCandidateChamberPath(lhs, rhs)) return false;
  if (config.localDuplicateEtaWindow <= 0.F ||
      config.localDuplicatePhiWindow <= 0.F) {
    return lhs.position.wire->eta == rhs.position.wire->eta &&
           lhs.position.strip->phi == rhs.position.strip->phi;
  }
  return std::abs(lhs.position.wire->eta - rhs.position.wire->eta) <
             config.localDuplicateEtaWindow &&
         std::abs(xAOD::P4Helpers::deltaPhi(lhs.position.strip->phi,
                                            rhs.position.strip->phi)) <
             config.localDuplicatePhiWindow;
}

void suppressLocalPositionDuplicates(std::vector<CandidatePair>& pairs,
                                     const Config& config,
                                     Statistics& statistics) {
  if (config.maxCandidatesPerLocalPosition == 0U || pairs.empty()) return;
  std::stable_sort(pairs.begin(), pairs.end(), betterCandidatePair);
  std::vector<CandidatePair> retained;
  retained.reserve(pairs.size());
  for (const CandidatePair& pair : pairs) {
    std::size_t count = 0U;
    for (const CandidatePair& selected : retained) {
      if (sameCoarseLocalPosition(pair, selected, config)) {
        ++count;
        if (count >= config.maxCandidatesPerLocalPosition) break;
      }
    }
    if (count >= config.maxCandidatesPerLocalPosition) {
      ++statistics.nLocalDuplicateCandidates;
      continue;
    }
    retained.emplace_back(pair);
  }
  pairs = std::move(retained);
}

void retainCandidateWorkingSet(std::vector<CandidatePair>& pairs,
                               const std::size_t maximum,
                               Statistics& statistics) {
  std::stable_sort(pairs.begin(), pairs.end(), betterCandidatePair);
  if (maximum == 0U || pairs.size() <= maximum) return;

  std::array<bool, 8> retainedMask{};
  std::vector<CandidatePair> retained;
  retained.reserve(maximum);
  for (const CandidatePair& pair : pairs) {
    if (pair.positionStationMask >= retainedMask.size() ||
        retainedMask[pair.positionStationMask]) {
      continue;
    }
    retained.emplace_back(pair);
    retainedMask[pair.positionStationMask] = true;
    if (retained.size() == maximum) break;
  }
  for (const CandidatePair& pair : pairs) {
    if (retained.size() == maximum) break;
    const bool alreadyRetained = std::any_of(
        retained.begin(), retained.end(),
        [&pair](const CandidatePair& selected) {
          return selected.wire == pair.wire && selected.strip == pair.strip &&
                 selected.position.stationIndex == pair.position.stationIndex;
        });
    if (!alreadyRetained) retained.emplace_back(pair);
  }
  statistics.nLimitedCandidates += pairs.size() - retained.size();
  pairs = std::move(retained);
  std::stable_sort(pairs.begin(), pairs.end(), betterCandidatePair);
}

void countPositionMask(const std::uint8_t mask, Statistics& statistics) {
  if (mask == 0x7U) ++statistics.nM1M2M3Candidates;
  else if (mask == 0x3U) ++statistics.nM1M2Candidates;
  else if (mask == 0x5U) ++statistics.nM1M3Candidates;
  else if (mask == 0x6U) ++statistics.nM2M3Candidates;
  else if (mask == 0x1U) ++statistics.nM1OnlyPositionCandidates;
  else if (mask == 0x2U) ++statistics.nM2OnlyPositionCandidates;
  else if (mask == 0x4U) ++statistics.nM3OnlyPositionCandidates;
}

}  // namespace

namespace L0Muon {
namespace TgcL0Floating {

SegmentReconstruction::SegmentReconstruction(SegmentReconstructionConfig config)
    : m_config{std::move(config)} {}

StatusCode SegmentReconstruction::build(
    const StationCoincidenceContainer& coincidences,
    TgcL0CandidateContainer& candidates,
    SegmentStatistics& statistics,
    TgcL0SegmentContainer* validationSegments) const {
  candidates.clear();
  statistics = SegmentStatistics{};
  if (validationSegments != nullptr) validationSegments->clear();

  using Projections =
      std::array<std::array<std::vector<const Coincidence*>, 3>, 2>;
  std::map<GroupKey, Projections> grouped;
  for (const Coincidence& coincidence : coincidences) {
    if (stationBit(coincidence.station) == 0U) continue;
    const std::size_t projection = coincidence.isStrip ? 1U : 0U;
    grouped[coincidence.key][projection][stationIndex(coincidence.station)]
        .emplace_back(&coincidence);
  }

  std::set<CandidateKey> candidateKeys;
  for (const auto& [groupKey, projections] : grouped) {
    const std::vector<ProjectionSegment> wireSegments =
        buildProjectionSegments(projections[0], false, m_config, statistics);
    const std::vector<ProjectionSegment> stripSegments =
        buildProjectionSegments(projections[1], true, m_config, statistics);
    statistics.nWireSegments += wireSegments.size();
    statistics.nStripSegments += stripSegments.size();
    if (validationSegments != nullptr) {
      appendValidationSegments(groupKey, wireSegments, *validationSegments);
      appendValidationSegments(groupKey, stripSegments, *validationSegments);
    }

    std::vector<CandidatePair> pairs;
    pairs.reserve(wireSegments.size() * stripSegments.size());
    for (const ProjectionSegment& wire : wireSegments) {
      if (m_config.maxCandidateDThetaAbs >= 0.F &&
          std::abs(wire.residual) > m_config.maxCandidateDThetaAbs) {
        statistics.nRejectedPairs += stripSegments.size();
        continue;
      }
      for (const ProjectionSegment& strip : stripSegments) {
        if (m_config.maxCandidateDPhiAbs >= 0.F &&
            std::abs(strip.residual) > m_config.maxCandidateDPhiAbs) {
          ++statistics.nRejectedPairs;
          continue;
        }
        const PositionPair position =
            selectPositionPair(wire, strip, m_config);
        if (!position) {
          ++statistics.nRejectedPairs;
          continue;
        }
        const std::uint8_t positionMask = static_cast<std::uint8_t>(
            wire.stationMask & strip.stationMask);
        const std::uint8_t combinedMask = static_cast<std::uint8_t>(
            wire.stationMask | strip.stationMask);
        if (std::popcount(static_cast<unsigned int>(combinedMask)) < 2) {
          ++statistics.nRejectedPairs;
          continue;
        }
        pairs.emplace_back(
            CandidatePair{&wire, &strip, position, positionMask, combinedMask});
      }
    }

    pruneLocalCandidateBins(pairs, m_config, statistics);
    suppressLocalPositionDuplicates(pairs, m_config, statistics);
    retainCandidateWorkingSet(pairs,
                              m_config.maxSegmentCombinationsPerGroup,
                              statistics);

    for (const CandidatePair& pair : pairs) {
      const CandidateKey keyValue = candidateKey(groupKey, pair);
      if (!candidateKeys.insert(keyValue).second) {
        ++statistics.nDuplicateCandidates;
        continue;
      }

      const ProjectionSegment& wire = *pair.wire;
      const ProjectionSegment& strip = *pair.strip;
      TgcL0Candidate candidate;
      candidate.subdetectorId = groupKey.subDetectorId;
      candidate.sectorId = groupKey.triggerSector;
      candidate.readoutSector = pair.position.wire->detectorSector;
      candidate.bcTag = groupKey.bcTag;
      candidate.eta = pair.position.wire->eta;
      candidate.phi = pair.position.strip->phi;
      candidate.deltaTheta = wire.outputResidual;
      candidate.deltaPhi = strip.outputResidual;
      candidate.wireStationMask = wire.stationMask;
      candidate.stripStationMask = strip.stationMask;
      candidate.positionStationMask = pair.positionStationMask;
      candidate.stationMask = candidate.positionStationMask;
      candidate.m1WireQuality = quality(wire, Station::M1);
      candidate.m2WireQuality = quality(wire, Station::M2);
      candidate.m3WireQuality = quality(wire, Station::M3);
      candidate.m1StripQuality = quality(strip, Station::M1);
      candidate.m2StripQuality = quality(strip, Station::M2);
      candidate.m3StripQuality = quality(strip, Station::M3);
      candidate.pivotWireChannel = pair.position.wire->channel;
      candidate.pivotStripChannel = pair.position.strip->channel;

      if ((candidate.positionStationMask & 0x1U) != 0U) {
        candidate.m1Eta = wire.points[0]->eta;
        candidate.m1Phi = strip.points[0]->phi;
        candidate.m1StationEta = wire.points[0]->stationEta;
        candidate.m1StationPhi = wire.points[0]->stationPhi;
      }
      if ((candidate.positionStationMask & 0x2U) != 0U) {
        candidate.m2Eta = wire.points[1]->eta;
        candidate.m2Phi = strip.points[1]->phi;
        candidate.m2StationEta = wire.points[1]->stationEta;
        candidate.m2StationPhi = wire.points[1]->stationPhi;
      }
      if ((candidate.positionStationMask & 0x4U) != 0U) {
        candidate.m3Eta = wire.points[2]->eta;
        candidate.m3Phi = strip.points[2]->phi;
        candidate.m3StationEta = wire.points[2]->stationEta;
        candidate.m3StationPhi = wire.points[2]->stationPhi;
      }
      candidate.wireQuality = wire.summedQuality;
      candidate.selectorPriority = static_cast<std::uint8_t>(
          2U * std::popcount(
                   static_cast<unsigned int>(pair.combinedStationMask)) +
          wire.summedQuality + strip.summedQuality);
      candidates.emplace_back(candidate);
      countPositionMask(candidate.positionStationMask, statistics);
    }
  }
  return StatusCode::SUCCESS;
}

}  // namespace TgcL0Floating
}  // namespace L0Muon
