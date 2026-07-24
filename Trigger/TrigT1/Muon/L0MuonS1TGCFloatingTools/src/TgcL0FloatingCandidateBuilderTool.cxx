/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0FloatingCandidateBuilderTool.h"

#include "TgcL0FloatingData.h"
#include "TgcL0RdoDecoder.h"
#include "TgcL0SegmentBuilder.h"

namespace L0Muon {

StatusCode TgcL0FloatingCandidateBuilderTool::build(
    const TgcRdoContainer& rdos, TgcL0CandidateContainer& candidates,
    const EventContext& ctx) const {
  (void)ctx;
  candidates.clear();

  TgcL0Floating::HitGroups hitGroups;
  TgcL0Floating::DecodeStatistics statistics;
  const TgcL0Floating::RdoDecoder decoder;
  ATH_CHECK(decoder.decode(rdos, hitGroups, statistics));

  TgcL0Floating::SegmentContainer segments;
  const TgcL0Floating::SegmentBuilder segmentBuilder;
  ATH_CHECK(segmentBuilder.build(hitGroups, segments));

  ATH_MSG_DEBUG("Decoded " << statistics.nHits << " TGC hits from "
                            << statistics.nRawData << " raw-data words in "
                            << hitGroups.size() << " groups: wire="
                            << statistics.nWireHits << ", strip="
                            << statistics.nStripHits << ", M1="
                            << statistics.nM1Hits << ", M2/M3="
                            << statistics.nM2M3Hits << ", inner="
                            << statistics.nInnerHits << ", unknown="
                            << statistics.nUnknownStation
                            << "; station channel segments="
                            << segments.size());

  return StatusCode::SUCCESS;
}

}  // namespace L0Muon
