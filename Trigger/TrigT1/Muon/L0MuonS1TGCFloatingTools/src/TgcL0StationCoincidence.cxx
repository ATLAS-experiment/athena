/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0StationCoincidence.h"

#include <algorithm>
#include <bit>
#include <cstdint>
#include <map>
#include <vector>

namespace L0Muon {
namespace TgcL0Floating {

std::uint8_t StationCoincidenceBuilder::nominalLayers(
    const Station station, const bool isStrip) {
  if (station == Station::M1 && !isStrip) {
    return 3;
  }
  if (station == Station::M1 || station == Station::M2 ||
      station == Station::M3) {
    return 2;
  }
  return 0;
}

StatusCode StationCoincidenceBuilder::build(
    const HitGroups& hitGroups,
    StationCoincidenceContainer& coincidences) const {
  coincidences.clear();

  for (const auto& [key, hits] : hitGroups) {
    const std::uint8_t nNominalLayers = nominalLayers(key.station, key.isStrip);
    if (nNominalLayers == 0 || hits.empty()) {
      continue;
    }

    std::map<std::uint16_t, std::uint8_t> channelLayerMasks;
    for (const Hit& hit : hits) {
      // Keep the physical gas-gap numbering. In M1, strip readout is on
      // gas gaps 1 and 3 although it has two instrumented strip layers.
      if (hit.gasGap == 0 || hit.gasGap > 3) {
        continue;
      }
      channelLayerMasks[hit.channel] |=
          static_cast<std::uint8_t>(1U << (hit.gasGap - 1U));
    }

    for (const auto& [channel, layerMask] : channelLayerMasks) {
      std::uint8_t combinedMask = layerMask;
      const std::map<std::uint16_t, std::uint8_t>::const_iterator previous =
          channelLayerMasks.find(static_cast<std::uint16_t>(channel - 1U));
      if (channel > 0U && previous != channelLayerMasks.end()) {
        combinedMask |= previous->second;
      }
      const std::map<std::uint16_t, std::uint8_t>::const_iterator next =
          channelLayerMasks.find(static_cast<std::uint16_t>(channel + 1U));
      if (next != channelLayerMasks.end()) {
        combinedMask |= next->second;
      }

      const std::uint8_t observedLayers = static_cast<std::uint8_t>(
          std::popcount(static_cast<unsigned int>(combinedMask)));
      coincidences.emplace_back(key, channel, combinedMask, observedLayers,
                                nNominalLayers);
    }
  }
  return StatusCode::SUCCESS;
}

}  // namespace TgcL0Floating
}  // namespace L0Muon
