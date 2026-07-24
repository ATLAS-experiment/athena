/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0FloatingCandidateBuilderTool.h"

#include "StoreGate/ReadCondHandle.h"
#include "TgcL0FloatingData.h"
#include "TgcL0RdoDecoder.h"
#include "TgcL0StationCoincidence.h"

namespace {

struct CoincidenceQualityCounts {
  std::size_t nThreeOfThree{0};
  std::size_t nTwoOfThree{0};
  std::size_t nOneOfThree{0};
  std::size_t nTwoOfTwo{0};
  std::size_t nOneOfTwo{0};
};

void countCoincidenceQuality(
    const L0Muon::TgcL0Floating::StationCoincidence& coincidence,
    CoincidenceQualityCounts& counts) {
  if (coincidence.nominalLayers == 3U) {
    if (coincidence.observedLayers == 3U) {
      ++counts.nThreeOfThree;
    } else if (coincidence.observedLayers == 2U) {
      ++counts.nTwoOfThree;
    } else if (coincidence.observedLayers == 1U) {
      ++counts.nOneOfThree;
    }
  } else if (coincidence.nominalLayers == 2U) {
    if (coincidence.observedLayers == 2U) {
      ++counts.nTwoOfTwo;
    } else if (coincidence.observedLayers == 1U) {
      ++counts.nOneOfTwo;
    }
  }
}

}  // namespace

namespace L0Muon {

StatusCode TgcL0FloatingCandidateBuilderTool::initialize() {
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_cablingKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode TgcL0FloatingCandidateBuilderTool::build(
    const TgcRdoContainer& rdos, TgcL0CandidateContainer& candidates,
    const EventContext& ctx) const {
  candidates.clear();

  const Muon::TgcCablingMap* cabling{};
  ATH_CHECK(SG::get(cabling, m_cablingKey, ctx));

  TgcL0Floating::HitGroups hitGroups;
  TgcL0Floating::DecodeStatistics statistics;
  const TgcL0Floating::RdoDecoder decoder;
  ATH_CHECK(decoder.decode(rdos, *cabling, *m_idHelperSvc, hitGroups,
                           statistics));

  TgcL0Floating::StationCoincidenceContainer coincidences;
  const TgcL0Floating::StationCoincidenceBuilder coincidenceBuilder;
  ATH_CHECK(coincidenceBuilder.build(hitGroups, coincidences));

  CoincidenceQualityCounts totalCounts;
  CoincidenceQualityCounts m1WireCounts;
  CoincidenceQualityCounts m1StripCounts;
  CoincidenceQualityCounts m2WireCounts;
  CoincidenceQualityCounts m2StripCounts;
  CoincidenceQualityCounts m3WireCounts;
  CoincidenceQualityCounts m3StripCounts;

  for (const TgcL0Floating::StationCoincidence& coincidence : coincidences) {
    countCoincidenceQuality(coincidence, totalCounts);

    CoincidenceQualityCounts* stationCounts{nullptr};
    if (coincidence.key.station == TgcL0Floating::Station::M1) {
      stationCounts = coincidence.key.isStrip ? &m1StripCounts : &m1WireCounts;
    } else if (coincidence.key.station == TgcL0Floating::Station::M2) {
      stationCounts = coincidence.key.isStrip ? &m2StripCounts : &m2WireCounts;
    } else if (coincidence.key.station == TgcL0Floating::Station::M3) {
      stationCounts = coincidence.key.isStrip ? &m3StripCounts : &m3WireCounts;
    }
    if (stationCounts != nullptr) {
      countCoincidenceQuality(coincidence, *stationCounts);
    }
  }

  ATH_MSG_DEBUG("Decoded " << statistics.nHits << " TGC hits from "
                            << statistics.nRawData << " raw-data words in "
                            << hitGroups.size() << " groups: wire="
                            << statistics.nWireHits << ", strip="
                            << statistics.nStripHits << ", M1="
                            << statistics.nM1Hits << ", M2="
                            << statistics.nM2Hits << ", M3="
                            << statistics.nM3Hits << ", inner="
                            << statistics.nInnerHits << ", unknown="
                            << statistics.nUnknownStation
                            << ", mapping failures="
                            << statistics.nMappingFailures
                            << "; station coincidences="
                            << coincidences.size() << " (3/3="
                            << totalCounts.nThreeOfThree << ", 2/3="
                            << totalCounts.nTwoOfThree << ", 1/3="
                            << totalCounts.nOneOfThree << ", 2/2="
                            << totalCounts.nTwoOfTwo << ", 1/2="
                            << totalCounts.nOneOfTwo << ")");

  ATH_MSG_DEBUG("Station coincidence breakdown: M1 wire=(3/3="
                << m1WireCounts.nThreeOfThree << ", 2/3="
                << m1WireCounts.nTwoOfThree << ", 1/3="
                << m1WireCounts.nOneOfThree << "), M1 strip=(2/2="
                << m1StripCounts.nTwoOfTwo << ", 1/2="
                << m1StripCounts.nOneOfTwo << "), M2 wire=(2/2="
                << m2WireCounts.nTwoOfTwo << ", 1/2="
                << m2WireCounts.nOneOfTwo << "), M2 strip=(2/2="
                << m2StripCounts.nTwoOfTwo << ", 1/2="
                << m2StripCounts.nOneOfTwo << "), M3 wire=(2/2="
                << m3WireCounts.nTwoOfTwo << ", 1/2="
                << m3WireCounts.nOneOfTwo << "), M3 strip=(2/2="
                << m3StripCounts.nTwoOfTwo << ", 1/2="
                << m3StripCounts.nOneOfTwo << ")");

  return StatusCode::SUCCESS;
}

}  // namespace L0Muon
