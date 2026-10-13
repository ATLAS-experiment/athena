/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONS1TGCINTERFACES_TGCSLSEGMENT_H
#define L1MUONS1TGCINTERFACES_TGCSLSEGMENT_H

#include "AthenaKernel/CLASS_DEF.h"

#include <cstdint>
#include <vector>

namespace L1Muon {

/** @brief Projection represented by a transient TGC segment. */
enum class TgcSLSegmentProjection : std::uint8_t { Wire = 0U, Strip = 1U };

/** @brief Event-local projection segment optionally exposed for validation. */
struct TgcSLSegment {
  std::uint16_t subdetectorId{0};
  std::uint16_t triggerSector{0};
  std::uint16_t bcTag{0};
  TgcSLSegmentProjection projection{TgcSLSegmentProjection::Wire};
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

using TgcSLSegmentContainer = std::vector<TgcSLSegment>;

}  // namespace L1Muon

CLASS_DEF(L1Muon::TgcSLSegmentContainer, 1230729782, 1)

#endif
