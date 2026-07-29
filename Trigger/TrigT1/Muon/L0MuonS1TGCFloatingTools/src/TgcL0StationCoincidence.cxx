/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0StationCoincidence.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

namespace {

using Hit = L0Muon::TgcL0Floating::Hit;
using Station = L0Muon::TgcL0Floating::Station;

std::size_t stationIndex(const Station station) {
  if (station == Station::M1) return 0U;
  if (station == Station::M2) return 1U;
  return 2U;
}

bool isBigWheelStation(const Station station) {
  return station == Station::M1 || station == Station::M2 ||
         station == Station::M3;
}

bool betterRepresentativeHit(const Hit* lhs, const Hit* rhs,
                             const std::uint16_t representativeChannel) {
  if (rhs == nullptr) return true;
  if (lhs == nullptr) return false;
  const int lhsDifference = std::abs(static_cast<int>(lhs->channel) -
                                     static_cast<int>(representativeChannel));
  const int rhsDifference = std::abs(static_cast<int>(rhs->channel) -
                                     static_cast<int>(representativeChannel));
  if (lhsDifference != rhsDifference) return lhsDifference < rhsDifference;
  if (lhs->gasGap != rhs->gasGap) return lhs->gasGap < rhs->gasGap;
  return lhs->channel < rhs->channel;
}

}  // namespace

namespace L0Muon {
namespace TgcL0Floating {

std::uint8_t StationCoincidenceBuilder::nominalLayers(
    const Station station, const bool isStrip) {
  if (station == Station::M1 && !isStrip) return 3U;
  if (station == Station::M1 || station == Station::M2 ||
      station == Station::M3) {
    return 2U;
  }
  return 0U;
}

StatusCode StationCoincidenceBuilder::build(
    const HitGroups& hitGroups,
    StationCoincidenceContainer& coincidences) const {
  coincidences.clear();

  for (const auto& [key, groupHits] : hitGroups) {
    std::array<std::array<std::vector<const Hit*>, 2>, 3> grouped{};
    for (const Hit& hit : groupHits) {
      if (!isBigWheelStation(hit.station)) continue;
      grouped[stationIndex(hit.station)][hit.isStrip ? 1U : 0U].emplace_back(&hit);
    }

    for (std::size_t stationPosition = 0U; stationPosition < grouped.size();
         ++stationPosition) {
      const Station hitStation = static_cast<Station>(stationPosition);
      for (std::size_t projection = 0U; projection < 2U; ++projection) {
        const bool isStrip = projection == 1U;
        std::vector<const Hit*>& hits = grouped[stationPosition][projection];
        if (hits.empty()) continue;

        std::sort(hits.begin(), hits.end(),
                  [](const Hit* lhs, const Hit* rhs) {
                    if (lhs->channel != rhs->channel) {
                      return lhs->channel < rhs->channel;
                    }
                    if (lhs->gasGap != rhs->gasGap) {
                      return lhs->gasGap < rhs->gasGap;
                    }
                    if (lhs->stationEta != rhs->stationEta) {
                      return lhs->stationEta < rhs->stationEta;
                    }
                    return lhs->stationPhi < rhs->stationPhi;
                  });

        std::vector<const Hit*> cluster;
        auto flushCluster = [&]() {
          if (cluster.empty()) return;

          std::uint32_t channelSum = 0U;
          std::uint8_t layerMask = 0U;
          float etaSum = 0.F;
          float sinPhiSum = 0.F;
          float cosPhiSum = 0.F;
          float rSum = 0.F;
          float zSum = 0.F;
          for (const Hit* hit : cluster) {
            channelSum += hit->channel;
            if (hit->gasGap > 0U && hit->gasGap <= 3U) {
              layerMask |= static_cast<std::uint8_t>(1U << (hit->gasGap - 1U));
            }
            etaSum += hit->eta;
            sinPhiSum += std::sin(hit->phi);
            cosPhiSum += std::cos(hit->phi);
            rSum += hit->r;
            zSum += hit->z;
          }

          const std::uint8_t observedLayers = static_cast<std::uint8_t>(
              std::popcount(static_cast<unsigned int>(layerMask)));
          if (observedLayers == 0U) {
            cluster.clear();
            return;
          }

          const std::uint16_t representativeChannel =
              static_cast<std::uint16_t>(std::lround(
                  static_cast<double>(channelSum) /
                  static_cast<double>(cluster.size())));
          const Hit* representativeHit = nullptr;
          for (const Hit* hit : cluster) {
            if (betterRepresentativeHit(hit, representativeHit,
                                        representativeChannel)) {
              representativeHit = hit;
            }
          }
          if (representativeHit == nullptr) {
            cluster.clear();
            return;
          }

          const float scale = 1.F / static_cast<float>(cluster.size());
          coincidences.emplace_back(
              key, hitStation, isStrip, representativeHit->detectorSector,
              representativeHit->stationEta, representativeHit->stationPhi,
              representativeChannel, layerMask, observedLayers,
              nominalLayers(hitStation, isStrip), etaSum * scale,
              std::atan2(sinPhiSum, cosPhiSum), rSum * scale, zSum * scale);
          cluster.clear();
        };

        std::uint16_t previousChannel = 0U;
        bool hasPreviousChannel = false;
        for (const Hit* hit : hits) {
          if (!hasPreviousChannel ||
              std::abs(static_cast<int>(hit->channel) -
                       static_cast<int>(previousChannel)) <= 1) {
            cluster.emplace_back(hit);
          } else {
            flushCluster();
            cluster.emplace_back(hit);
          }
          previousChannel = hit->channel;
          hasPreviousChannel = true;
        }
        flushCluster();
      }
    }
  }

  std::sort(coincidences.begin(), coincidences.end(),
            [](const StationCoincidence& lhs,
               const StationCoincidence& rhs) {
              if (lhs.key.tie() != rhs.key.tie()) {
                return lhs.key.tie() < rhs.key.tie();
              }
              if (lhs.station != rhs.station) return lhs.station < rhs.station;
              if (lhs.isStrip != rhs.isStrip) return lhs.isStrip < rhs.isStrip;
              if (lhs.observedLayers != rhs.observedLayers) {
                return lhs.observedLayers > rhs.observedLayers;
              }
              return lhs.channel < rhs.channel;
            });
  return StatusCode::SUCCESS;
}

}  // namespace TgcL0Floating
}  // namespace L0Muon
