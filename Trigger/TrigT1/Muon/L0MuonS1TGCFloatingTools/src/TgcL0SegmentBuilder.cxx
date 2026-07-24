/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0SegmentBuilder.h"

#include <algorithm>
#include <limits>

namespace L0Muon {
namespace TgcL0Floating {

StatusCode SegmentBuilder::build(const HitGroups& hitGroups,
                                 SegmentContainer& segments) const {
  segments.clear();
  segments.reserve(hitGroups.size());

  for (const auto& [key, hits] : hitGroups) {
    if (hits.empty()) {
      continue;
    }

    std::uint16_t firstChannel = std::numeric_limits<std::uint16_t>::max();
    std::uint16_t lastChannel = 0;
    for (const Hit& hit : hits) {
      firstChannel = std::min(firstChannel, hit.channel);
      lastChannel = std::max(lastChannel, hit.channel);
    }

    segments.emplace_back(key, firstChannel, lastChannel, hits.size());
  }
  return StatusCode::SUCCESS;
}

}  // namespace TgcL0Floating
}  // namespace L0Muon
