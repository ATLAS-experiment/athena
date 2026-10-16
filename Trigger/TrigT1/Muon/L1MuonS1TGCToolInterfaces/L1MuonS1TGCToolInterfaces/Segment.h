/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCINTERFACES_SEGMENT_H
#define L1MUONS1TGCINTERFACES_SEGMENT_H

#include "AthenaKernel/CLASS_DEF.h"

#include <cstdint>
#include <vector>

namespace L1Muon::L1TGC {

/** @brief Projection represented by a transient TGC segment. */
enum class SegmentProjection : std::uint8_t { Wire = 0U, Strip = 1U };

/** @brief Event-local projection segment optionally exposed for validation. */
struct Segment {
  std::uint16_t subdetectorId{0};
  std::uint16_t triggerSector{0};
  std::uint16_t bcTag{0};
  SegmentProjection projection{SegmentProjection::Wire};
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

using SegmentContainer = std::vector<Segment>;

}  // namespace L1Muon::L1TGC

CLASS_DEF(L1Muon::L1TGC::SegmentContainer, 1241015246, 1)

#endif
