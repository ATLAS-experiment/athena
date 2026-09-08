/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCINTERFACES_TGCL0SEGMENT_H
#define L0MUONS1TGCINTERFACES_TGCL0SEGMENT_H

#include "AthenaKernel/CLASS_DEF.h"

#include <cstdint>
#include <vector>

namespace L0Muon {

/** @brief Projection represented by a transient TGC segment. */
enum class TgcL0SegmentProjection : std::uint8_t { Wire = 0U, Strip = 1U };

/** @brief Event-local projection segment optionally exposed for validation. */
struct TgcL0Segment {
  std::uint16_t subdetectorId{0};
  std::uint16_t triggerSector{0};
  std::uint16_t bcTag{0};
  TgcL0SegmentProjection projection{TgcL0SegmentProjection::Wire};
  std::uint8_t stationMask{0};
  std::uint8_t summedQuality{0};
  std::uint8_t nStations{0};
  float eta{0.F};
  float phi{0.F};
  float residual{0.F};
  float outputResidual{0.F};
  float consistency{0.F};
  std::uint16_t pivotChannel{0};
};

using TgcL0SegmentContainer = std::vector<TgcL0Segment>;

}  // namespace L0Muon

CLASS_DEF(L0Muon::TgcL0SegmentContainer, 1316895012, 1)

#endif
