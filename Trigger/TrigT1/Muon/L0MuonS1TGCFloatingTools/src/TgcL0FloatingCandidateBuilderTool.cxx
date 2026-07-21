/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0FloatingCandidateBuilderTool.h"

#include "MuonRDO/TgcRawData.h"
#include "MuonRDO/TgcRdo.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <tuple>

namespace {

enum class Station : std::uint8_t { M1, M2M3, Inner, Unknown, NumberOfStations };

struct HitGroupKey {
  using Tuple = std::tuple<std::uint16_t, std::uint16_t, std::uint16_t, Station, bool>;

  std::uint16_t subDetectorId{0};
  std::uint16_t detectorSector{0};
  std::uint16_t bcTag{0};
  Station station{Station::Unknown};
  bool isStrip{false};

  Tuple tie() const {
    return std::make_tuple(subDetectorId, detectorSector, bcTag, station, isStrip);
  }

  bool operator<(const HitGroupKey& other) const { return tie() < other.tie(); }
};

struct DecodedHit {
  std::uint16_t subDetectorId{0};
  std::uint16_t detectorSector{0};
  std::uint16_t bcTag{0};
  std::uint16_t sswId{0};
  std::uint16_t slbId{0};
  std::uint16_t channel{0};
  Station station{Station::Unknown};
  bool isStrip{false};
};

using HitGroups = std::map<HitGroupKey, std::vector<DecodedHit>>;

Station station(const TgcRawData::SlbType type) {
  switch (type) {
    case TgcRawData::SLB_TYPE_TRIPLET_WIRE:
    case TgcRawData::SLB_TYPE_TRIPLET_STRIP:
      return Station::M1;
    case TgcRawData::SLB_TYPE_DOUBLET_WIRE:
    case TgcRawData::SLB_TYPE_DOUBLET_STRIP:
      return Station::M2M3;
    case TgcRawData::SLB_TYPE_INNER_WIRE:
    case TgcRawData::SLB_TYPE_INNER_STRIP:
      return Station::Inner;
    default:
      return Station::Unknown;
  }
}

bool isStrip(const TgcRawData::SlbType type) {
  return type == TgcRawData::SLB_TYPE_DOUBLET_STRIP ||
         type == TgcRawData::SLB_TYPE_TRIPLET_STRIP ||
         type == TgcRawData::SLB_TYPE_INNER_STRIP;
}

constexpr std::size_t stationIndex(const Station value) {
  return static_cast<std::size_t>(value);
}

}  // namespace

namespace L0Muon {

StatusCode TgcL0FloatingCandidateBuilderTool::build(
    const TgcRdoContainer& rdos, TgcL0CandidateContainer& candidates,
    const EventContext& ctx) const {
  (void)ctx;
  candidates.clear();

  HitGroups hitGroups;
  std::size_t nRawData{0};
  std::size_t nHits{0};
  std::size_t nUnknownStation{0};
  std::array<std::size_t, stationIndex(Station::NumberOfStations)> nHitsByStation{};
  std::size_t nWireHits{0};
  std::size_t nStripHits{0};

  // This implementation is based on the Run-3 TGC hit RDO format, including
  // its ROD, SSW and SLB structure and detector-sector identifiers. The Run-4
  // TGC readout will use a substantially different data format and will not
  // contain some of these Run-3 readout concepts. Therefore, this decoding and
  // hit-organization code is expected to require a broad update when the Run-4
  // TGC RDO becomes available, rather than a local change to the sector mapping.
  // The output of this tool remains the common TgcL0CandidateContainer, so the
  // downstream Inner Coincidence and Track Selector code is not expected to
  // require corresponding changes.
  for (const TgcRdo* rdo : rdos) {
    if (!rdo) {
      continue;
    }
    for (const TgcRawData* rawData : *rdo) {
      if (!rawData) {
        continue;
      }
      ++nRawData;
      if (rawData->type() != TgcRawData::TYPE_HIT) {
        continue;
      }

      const Station hitStation = station(rawData->slbType());
      const bool hitIsStrip = isStrip(rawData->slbType());
      DecodedHit hit{
          .subDetectorId = rawData->subDetectorId(),
          .detectorSector = rawData->rodId(),
          .bcTag = rawData->bcTag(),
          .sswId = rawData->sswId(),
          .slbId = rawData->slbId(),
          .channel = rawData->channel(),
          .station = hitStation,
          .isStrip = hitIsStrip,
      };

      const HitGroupKey key{
          .subDetectorId = hit.subDetectorId,
          .detectorSector = hit.detectorSector,
          .bcTag = hit.bcTag,
          .station = hit.station,
          .isStrip = hit.isStrip,
      };
      hitGroups[key].push_back(hit);

      ++nHits;
      ++nHitsByStation.at(stationIndex(hitStation));
      if (hitStation == Station::Unknown) {
        ++nUnknownStation;
      }
      if (hitIsStrip) {
        ++nStripHits;
      } else {
        ++nWireHits;
      }
    }
  }

  ATH_MSG_DEBUG("Decoded " << nHits << " TGC hits from " << nRawData
                            << " raw-data words in " << hitGroups.size()
                            << " groups: wire=" << nWireHits
                            << ", strip=" << nStripHits
                            << ", M1=" << nHitsByStation.at(stationIndex(Station::M1))
                            << ", M2/M3=" << nHitsByStation.at(stationIndex(Station::M2M3))
                            << ", inner=" << nHitsByStation.at(stationIndex(Station::Inner))
                            << ", unknown=" << nUnknownStation);

  return StatusCode::SUCCESS;
}

}  // namespace L0Muon
